"""Mutation tests: the policy must reject concrete architectural regressions."""
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("policy", ROOT / "tools/check_architecture.py")
policy = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(policy)


class PolicyFixture(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name in ("modules", "architecture", "docs", "tests", "app", ".github", "packaging", "cmake", "tools"):
            shutil.copytree(ROOT / name, self.root / name, ignore=shutil.ignore_patterns("__pycache__"))
        shutil.copy(ROOT / "CMakeLists.txt", self.root)
        shutil.copy(ROOT / "AGENTS.md", self.root)
        shutil.copy(ROOT / "setup.cmd", self.root)
        shutil.copy(ROOT / "CMakePresets.json", self.root)

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def manifest(self, edit):
        path = self.root / "architecture/modules.json"
        data = json.loads(path.read_text())
        edit(data["modules"])
        path.write_text(json.dumps(data))

    def rejected(self, reason):
        errors = policy.validate(self.root)
        self.assertTrue(any(reason in error for error in errors), errors)


class ArchitectureTests(PolicyFixture):
    def test_pattern_editor_cannot_leak_into_engine(self):
        self.write("modules/engine/src/engine.cpp", "#include <simplicity/pattern_panel.h>\n")
        self.rejected("undeclared dependency")

    def test_valid_repository(self):
        self.assertEqual(policy.validate(self.root), [])

    def test_undeclared_include(self):
        self.write("modules/core/src/core.cpp", "#include <simplicity/engine.h>\n")
        self.rejected("undeclared dependency")

    def test_transitive_include_not_direct(self):
        self.write("modules/juce_adapter/src/juce_adapter.cpp", "#include <simplicity/core.h>\n")
        self.rejected("undeclared dependency")

    def test_juce_leak(self):
        self.write("modules/engine/src/engine.cpp", "#include <juce_audio_basics/juce_audio_basics.h>\n")
        self.rejected("JUCE may not leak")

    def test_relative_include(self):
        self.write("modules/core/src/core.cpp", '#include "../../engine/include/simplicity/engine.h"\n')
        self.rejected("include bypass")

    def test_computed_include(self):
        self.write("modules/core/src/core.cpp", "#define HEADER <simplicity/engine.h>\n#include HEADER\n")
        self.rejected("computed includes")

    def test_cycle(self):
        self.manifest(lambda m: m["core"]["dependencies"].append("engine"))
        self.rejected("Dependency cycle")
        self.rejected("foundation must remain independent")

    def test_unknown_dependency(self):
        self.manifest(lambda m: m["engine"]["dependencies"].append("missing"))
        self.rejected("invalid dependency")

    def test_external_dependency(self):
        self.manifest(lambda m: m["engine"]["external"].append("juce::juce_core"))
        self.rejected("unauthorized external")

    def test_missing_contract(self):
        (self.root / "docs/modules/engine.md").unlink()
        self.rejected("module contract missing")

    def test_incomplete_contract(self):
        self.write("docs/modules/engine.md", "# Empty\n")
        self.rejected("module contract missing")

    def test_unregistered_module(self):
        (self.root / "modules/rogue").mkdir()
        self.rejected("directories and manifest differ")

    def test_unowned_source(self):
        self.write("misc/rogue.cpp", "int rogue;\n")
        self.rejected("Unowned source")

    def test_new_gui_source_requires_explicit_registration(self):
        self.write("modules/gui/src/unregistered_style.cpp", "// Must not be silently globbed.\n")
        self.rejected("CMake must match")

    def test_domain_cannot_include_gui_asset_data(self):
        self.write("modules/engine/src/engine.cpp", "#include <simplicity/ui_asset_data.h>\n")
        self.rejected("undeclared dependency")

    def test_unlisted_source(self):
        self.write("modules/core/src/extra.cpp", "int extra;\n")
        self.rejected("CMake must match")

    def test_cmake_dependency_bypass(self):
        path = self.root / "modules/core/CMakeLists.txt"
        path.write_text(path.read_text() + "target_link_libraries(simplicity_core PUBLIC simplicity_engine)\n")
        self.rejected("CMake must match")

    def test_missing_test(self):
        (self.root / "tests/engine_test.cpp").unlink()
        self.rejected("missing/invalid test")

    def test_test_not_wired(self):
        path = self.root / "tests/CMakeLists.txt"
        path.write_text(path.read_text().replace("engine_test.cpp", "nonexistent.cpp"))
        self.rejected("test not wired")

    def test_duplicate_header(self):
        self.write("modules/engine/include/simplicity/core.h", "#pragma once\n")
        self.rejected("Duplicate public header")

    def test_bad_schema(self):
        self.write("architecture/modules.json", '{"schema_version": 99}')
        self.rejected("Unsupported architecture schema")

    def test_malformed_manifest(self):
        self.write("architecture/modules.json", "{")
        self.rejected("Invalid architecture configuration")

    def test_private_include(self):
        self.write("modules/core/src/core.cpp", '#include "engine_private.h"\n')
        self.rejected("unregistered header")

    def test_gate_removal(self):
        path = self.root / "CMakeLists.txt"
        path.write_text(path.read_text().replace("COMMAND_ERROR_IS_FATAL ANY", ""))
        self.rejected("Configure-time architecture gate missing")

    def test_pin_removal(self):
        path = self.root / "CMakeLists.txt"
        path.write_text(path.read_text().replace("29396c22c93392d6738e021b83196283d6e4d850", "main"))
        self.rejected("Pinned JUCE commit missing")

    def test_required_ci_gate_cannot_be_removed(self):
        path = self.root / ".github/workflows/quality.yml"
        path.write_text(path.read_text().replace("needs: [policy, build, sanitizers, coverage, package]", "needs: [policy]"))
        self.rejected("Required CI workflow contract missing")

    def test_protection_payload_cannot_be_weakened(self):
        path = self.root / ".github/branch-protection.json"
        data = json.loads(path.read_text())
        data["enforce_admins"] = False
        path.write_text(json.dumps(data))
        self.rejected("Branch protection payload weakened")

    def test_workflow_instructions_cannot_be_removed(self):
        (self.root / "AGENTS.md").unlink()
        self.rejected("Workflow contract missing")

    def test_non_windows_product_ci_is_rejected(self):
        path = self.root / ".github/workflows/quality.yml"
        path.write_text(path.read_text() + "\n# macos-latest product job\n")
        self.rejected("Product CI must target Windows only")

    def test_windows_ci_matches_supported_vs2022_toolchain(self):
        path = self.root / ".github/workflows/quality.yml"
        path.write_text(path.read_text().replace("runs-on: windows-2022", "runs-on: windows-latest"))
        self.rejected("Required CI workflow contract missing")

    def test_windows_guard_cannot_be_removed(self):
        path = self.root / "CMakeLists.txt"
        path.write_text(path.read_text().replace("if(NOT WIN32 AND NOT SIMPLICITY_ANALYSIS_ONLY)", "if(FALSE)"))
        self.rejected("Windows setup contract missing")

    def test_installer_must_not_require_admin(self):
        path = self.root / "packaging/windows/simplicity.iss"
        path.write_text(path.read_text().replace("PrivilegesRequired=lowest", "PrivilegesRequired=admin"))
        self.rejected("Windows setup contract missing")

    def test_static_runtime_contract(self):
        path = self.root / "CMakeLists.txt"
        path.write_text(path.read_text().replace("MultiThreaded$<$<CONFIG:Debug>:Debug>", "MultiThreadedDLL"))
        self.rejected("Windows setup contract missing")

    def test_preset_cannot_drop_juce(self):
        path = self.root / "CMakePresets.json"
        data = json.loads(path.read_text())
        data["configurePresets"][0]["cacheVariables"]["SIMPLICITY_WITH_JUCE"] = "OFF"
        path.write_text(json.dumps(data))
        self.rejected("Windows preset must select")

    def test_shell_state_cannot_depend_on_juce(self):
        self.write("modules/shell/src/workspace.cpp", "#include <juce_gui_basics/juce_gui_basics.h>\n")
        self.rejected("JUCE may not leak")

    def test_gui_cannot_smuggle_audio_dependency(self):
        self.write("modules/gui/src/workspace_view.cpp", "#include <juce_audio_basics/juce_audio_basics.h>\n")
        self.rejected("undeclared JUCE family")

    def test_engine_cannot_include_gui(self):
        self.write("modules/engine/src/engine.cpp", "#include <simplicity/workspace_view.h>\n")
        self.rejected("undeclared dependency")

    def test_gui_test_cannot_be_removed(self):
        path = self.root / "tests/CMakeLists.txt"
        path.write_text(path.read_text().replace("gui_controls_and_render", "deleted"))
        self.rejected("Native GUI controls/render gate missing")

    def test_gui_subsystem_required(self):
        path = self.root / "app/CMakeLists.txt"
        path.write_text(path.read_text().replace("WIN32 main.cpp", "main.cpp"))
        self.rejected("native GUI subsystem")

    def test_comments_not_dependencies(self):
        self.write("modules/core/src/core.cpp", "// #include <simplicity/engine.h>\n/* #include UNKNOWN */\n")
        self.assertEqual(policy.validate(self.root), [])


class ChangeEvidenceTests(PolicyFixture):
    def setUp(self):
        super().setUp()
        self.git("init", "-q")
        # Git 2.55 runner background writers can outlive commit and race rmtree.
        # Keep throwaway repos synchronous; never ignore/retry cleanup errors.
        for key, value in (("maintenance.auto", "false"), ("gc.auto", "0"),
                           ("gc.autoDetach", "false"), ("core.fsmonitor", "false")):
            self.git("config", key, value)
        self.git("config", "user.email", "tests@example.invalid")
        self.git("config", "user.name", "Policy tests")
        self.git("add", ".")
        self.git("commit", "-qm", "baseline")

    def git(self, *args):
        return subprocess.check_output(["git", *args], cwd=self.root, text=True, stderr=subprocess.STDOUT)

    def change_engine(self):
        path = self.root / "modules/engine/src/engine.cpp"
        path.write_text(path.read_text() + "\n// behavioral change\n")

    def test_fixture_git_background_tasks_disabled(self):
        for key, value in (("maintenance.auto", "false"), ("gc.auto", "0"),
                           ("gc.autoDetach", "false"), ("core.fsmonitor", "false")):
            self.assertEqual(self.git("config", "--get", key).strip(), value)

    def test_code_without_evidence_fails(self):
        self.change_engine()
        errors = policy.validate_changes(self.root, "HEAD")
        self.assertTrue(any("corresponding test" in error for error in errors))
        self.assertTrue(any("documentation changes" in error for error in errors))
        self.assertTrue(any("change record" in error for error in errors))

    def test_code_with_evidence_passes(self):
        self.change_engine()
        self.write("tests/engine_test.cpp", "// changed regression test\n")
        self.write("docs/changes/test.md", "# Test evidence\n")
        self.git("add", ".")
        self.assertEqual(policy.validate_changes(self.root, "HEAD"), [])

    def test_removed_test_not_evidence(self):
        self.change_engine()
        (self.root / "tests/engine_test.cpp").unlink()
        self.rejected("missing/invalid test")
        self.assertTrue(any("corresponding test" in error for error in policy.validate_changes(self.root, "HEAD")))

    def test_docs_only_no_code_evidence_needed(self):
        self.write("docs/architecture.md", "# Updated documentation\n")
        self.assertEqual(policy.validate_changes(self.root, "HEAD"), [])

    def test_policy_change_needs_tests_and_record(self):
        self.write("architecture/modules.json", (self.root / "architecture/modules.json").read_text() + "\n")
        errors = policy.validate_changes(self.root, "HEAD")
        self.assertTrue(any("require test changes" in error for error in errors))
        self.assertTrue(any("change record" in error for error in errors))
        self.assertTrue(any("require policy regression" in error for error in errors))
        self.assertTrue(any("require an ADR" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
