#!/usr/bin/env python3
"""Fail-closed structural checks. No third-party Python dependencies; see docs/architecture.md."""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def module_cmake(name: str, config: dict, sources: list[str]) -> str:
    target = f"simplicity_{name}"
    kind = "STATIC " + " ".join(sources) if sources else "INTERFACE"
    scope = "PUBLIC" if sources else "INTERFACE"
    lines = [f"add_library({target} {kind})",
             f"target_include_directories({target} {scope} include)"]
    deps = [f"simplicity_{dep}" for dep in config["dependencies"]] + config["external"]
    if deps:
        lines.append(f"target_link_libraries({target} {scope} {' '.join(deps)})")
    if name == "juce_adapter":
        lines.append(f"target_compile_definitions({target} PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)")
    if sources:
        lines.append(f"simplicity_strict({target})")
    return "\n".join(lines) + "\n"


def uncomment(text: str) -> str:
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def validate(root: Path) -> list[str]:
    errors: list[str] = []
    try:
        manifest = json.loads((root / "architecture/modules.json").read_text())
        if manifest["schema_version"] != 1:
            return ["Unsupported architecture schema"]
        modules = manifest["modules"]
        if not modules or not isinstance(modules, dict):
            return ["No modules declared"]
        for name, config in modules.items():
            if not re.fullmatch(r"[a-z][a-z_]*", name):
                errors.append(f"Invalid module name: {name}")
            if set(config) != {"dependencies", "external", "tests"}:
                errors.append(f"{name}: invalid manifest keys")
            if any(not isinstance(config[key], list) for key in ("dependencies", "external", "tests")):
                return [f"{name}: expected lists"]
            if not config["tests"]:
                errors.append(f"{name}: no tests registered")
            for dep in config["dependencies"]:
                if dep not in modules or dep == name:
                    errors.append(f"{name}: invalid dependency {dep}")
            allowed_external = {"juce_adapter": ["juce::juce_audio_basics"], "gui": ["simplicity::juce_gui"]}.get(name, [])
            if config["external"] != allowed_external:
                errors.append(f"{name}: unauthorized external dependencies")
        # Core and SDK must never acquire dependencies, even by changing the manifest.
        for name in ("core", "sdk"):
            if name not in modules or modules[name]["dependencies"]:
                errors.append(f"{name}: foundation must remain independent")
        visiting: set[str] = set()
        visited: set[str] = set()

        def visit(name: str) -> None:
            if name in visiting:
                errors.append(f"Dependency cycle at {name}")
                return
            if name in visited or name not in modules:
                return
            visiting.add(name)
            for dep in modules[name]["dependencies"]:
                visit(dep)
            visiting.remove(name)
            visited.add(name)
        for name in modules:
            visit(name)
        actual = {p.name for p in (root / "modules").iterdir() if p.is_dir()}
        if actual != set(modules):
            errors.append("Module directories and manifest differ")
        owners: dict[str, str] = {}
        for name in modules:
            for header in (root / "modules" / name / "include").rglob("*"):
                if header.is_file():
                    include = header.relative_to(root / "modules" / name / "include").as_posix()
                    if include in owners:
                        errors.append(f"Duplicate public header: {include}")
                    owners[include] = name
        for name, config in modules.items():
            directory = root / "modules" / name
            sources = sorted(p.relative_to(directory).as_posix() for p in directory.rglob("*.cpp"))
            expected = module_cmake(name, config, sources)
            if (directory / "CMakeLists.txt").read_text() != expected:
                errors.append(f"{name}: CMake must match the declared graph and explicit sources; expected:\n{expected}")
            doc = root / "docs/modules" / f"{name}.md"
            if not doc.is_file() or any(section not in doc.read_text() for section in
                                       ("## Responsibility", "## Dependencies", "## Threading", "## Tests", "## Extension points")):
                errors.append(f"{name}: module contract missing required sections")
            for test in config["tests"]:
                if not test.startswith("tests/") or ".." in Path(test).parts or not (root / test).is_file():
                    errors.append(f"{name}: missing/invalid test {test}")
                elif Path(test).name not in (root / "tests/CMakeLists.txt").read_text():
                    errors.append(f"{name}: test not wired into CMake: {test}")
        for path in root.rglob("*"):
            relative = path.relative_to(root)
            if any(part.startswith(".") or part.startswith("build") or part in {"__pycache__", "coverage"}
                   for part in relative.parts):
                continue
            if path.suffix not in {".cpp", ".c", ".h", ".hpp", ".cc", ".cxx"} or not path.is_file():
                continue
            parts = relative.parts
            if parts[0] not in {"modules", "app", "tests"}:
                errors.append(f"Unowned source: {relative}")
                continue
            owner = parts[1] if parts[0] == "modules" else parts[0]
            if owner not in {*modules, "app", "tests"}:
                errors.append(f"Unregistered source owner: {relative}")
                continue
            if parts[0] == "modules" and (len(parts) < 4 or parts[2] not in {"src", "include"}
                                          or path.suffix not in {".cpp", ".h"}):
                errors.append(f"Unsupported module source layout: {relative}")
            text = uncomment(path.read_text())
            for line in text.splitlines():
                if re.match(r"\s*#\s*include\b", line):
                    match = re.fullmatch(r'\s*#\s*include\s*[<"]([^>"\n]+)[>"]\s*', line)
                    if not match:
                        errors.append(f"{relative}: computed includes are forbidden")
                        continue
                    include = match[1]
                    if include.startswith("/") or ".." in Path(include).parts:
                        errors.append(f"{relative}: relative/absolute include bypass")
                    elif include in owners:
                        allowed = {owner, *modules.get(owner, {}).get("dependencies", [])}
                        if owner in {"tests", "app"}:
                            allowed = set(modules)
                        if owners[include] not in allowed:
                            errors.append(f"{relative}: undeclared dependency on {owners[include]}")
                    elif include.startswith("juce_"):
                        if owner not in {"juce_adapter", "gui", "app", "tests"}:
                            errors.append(f"{relative}: JUCE may not leak into domain modules")
                    elif "/" in include and include != "sys/types.h":
                        errors.append(f"{relative}: unregistered include {include}")
                    elif include.endswith((".h", ".hpp")) and include not in {
                        "stdint.h", "stddef.h", "stdlib.h", "stdio.h", "math.h", "assert.h", "test_support.h"}:
                        errors.append(f"{relative}: unregistered header {include}")
        workflow = (root / ".github/workflows/quality.yml").read_text()
        required_workflow_fragments = (
            "pull_request:", "contents: read", "fetch-depth: 0",
            'python3 tools/check_architecture.py --base "$BASE_SHA"',
            "python3 -m unittest discover -s tests", "name: Windows x64 (${{ matrix.config }})",
            "runs-on: windows-2022", "setup.cmd -NoInstall -Configuration ${{ matrix.config }}",
            "config: [Debug, Release]", "-DSIMPLICITY_ANALYSIS_ONLY=ON",
            "./tests/windows/installer_test.ps1", "Simplicity-Windows-x64-development",
            "-DSIMPLICITY_SANITIZERS=ON", "-DSIMPLICITY_COVERAGE=ON",
            "--fail-under-line 95 --fail-under-branch 90", "name: Required gates",
            "needs: [policy, build, sanitizers, coverage, package]", "if: always()",
            "all(job['result'] == 'success' for job in results.values())",
        )
        for fragment in required_workflow_fragments:
            if fragment not in workflow:
                errors.append(f"Required CI workflow contract missing: {fragment}")
        if "macos-latest" in workflow or "os: [" in workflow:
            errors.append("Product CI must target Windows only; Linux jobs are internal analysis")
        windows_contract = {
            "CMakeLists.txt": ("if(NOT WIN32 AND NOT SIMPLICITY_ANALYSIS_ONLY)",
                               'set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")'),
            "setup.cmd": ("-NoProfile -ExecutionPolicy Bypass", "exit /b %ERRORLEVEL%"),
            "tools/windows/setup.ps1": ("$NoInstall", "--preset", "Find-VisualStudio", "ctest.exe"),
            "packaging/windows/simplicity.iss": ("PrivilegesRequired=lowest", "DefaultDirName={localappdata}\\Programs\\Simplicity",
                                                 "ArchitecturesAllowed=x64compatible", "MinVersion=10.0"),
        }
        for filename, fragments in windows_contract.items():
            text = (root / filename).read_text()
            for fragment in fragments:
                if fragment not in text:
                    errors.append(f"Windows setup contract missing in {filename}: {fragment}")
        presets = json.loads((root / "CMakePresets.json").read_text())
        windows = next(p for p in presets["configurePresets"] if p["name"] == "windows")
        if (windows["architecture"] != "x64" or windows["generator"] != "Visual Studio 17 2022"
                or windows["cacheVariables"].get("SIMPLICITY_WITH_JUCE") != "ON"
                or windows["cacheVariables"].get("SIMPLICITY_ANALYSIS_ONLY") != "OFF"):
            errors.append("Windows preset must select the supported x64/JUCE product toolchain")
        gui_support = (root / "cmake/JuceGui.cmake").read_text()
        for fragment in ("PRIVATE juce::juce_gui_basics", "JUCE_WEB_BROWSER=0", "JUCE_USE_CURL=0"):
            if fragment not in gui_support:
                errors.append(f"GUI dependency isolation contract missing: {fragment}")
        if "WIN32 main.cpp" not in (root / "app/CMakeLists.txt").read_text():
            errors.append("Windows product must use the native GUI subsystem, not a console launcher")
        if "gui_controls_and_render" not in (root / "tests/CMakeLists.txt").read_text():
            errors.append("Native GUI controls/render gate missing")
        protection = json.loads((root / ".github/branch-protection.json").read_text())
        if (protection["required_status_checks"] != {"strict": True, "contexts": ["Required gates"]}
                or protection["enforce_admins"] is not True
                or protection["allow_force_pushes"] is not False
                or protection["allow_deletions"] is not False):
            errors.append("Branch protection payload weakened (server application still requires owner access)")
        for doc in ("AGENTS.md", "docs/workflow.md", ".github/pull_request_template.md"):
            if not (root / doc).is_file():
                errors.append(f"Workflow contract missing: {doc}")
        cmake = (root / "CMakeLists.txt").read_text()
        for name in modules:
            if f"add_subdirectory(modules/{name})" not in cmake:
                errors.append(f"{name}: module not wired into root CMake")
        if "tools/check_architecture.py" not in cmake or "COMMAND_ERROR_IS_FATAL ANY" not in cmake:
            errors.append("Configure-time architecture gate missing")
        if "29396c22c93392d6738e021b83196283d6e4d850" not in cmake:
            errors.append("Pinned JUCE commit missing; dependency upgrades require policy update")
    except (OSError, ValueError, KeyError, TypeError, StopIteration) as error:
        errors.append(f"Invalid architecture configuration: {error}")
    return errors


def validate_changes(root: Path, base: str) -> list[str]:
    # --diff-filter includes deletions/renames; removed tests/docs cannot satisfy evidence.
    output = subprocess.check_output(["git", "diff", "--name-only", "--diff-filter=ACDMRT", base, "--"],
                                     cwd=root, text=True)
    changed = set(output.splitlines())
    evidence = {name for name in changed if (root / name).is_file()}
    manifest = json.loads((root / "architecture/modules.json").read_text())
    errors = []
    for name, config in manifest["modules"].items():
        if any(path.startswith(f"modules/{name}/") for path in changed):
            if not evidence.intersection(config["tests"]):
                errors.append(f"{name}: production changes require corresponding test changes")
            if not any(path.startswith("docs/") for path in evidence):
                errors.append(f"{name}: production changes require documentation changes")
    production = any(path.startswith(("modules/", "app/", "tools/", "architecture/", "packaging/", "cmake/", ".github/workflows/"))
                     or path in {"CMakeLists.txt", "CMakePresets.json", "setup.cmd"} for path in changed)
    if production:
        if not any(path.startswith("tests/") for path in evidence):
            errors.append("Code/policy changes require test changes")
        if not any(path.startswith("docs/changes/") and path.endswith(".md") for path in evidence):
            errors.append("Code/policy changes require a change record in docs/changes/")
    architecture_policy_changed = any(
        path in {"architecture/modules.json", "tools/check_architecture.py", "CMakeLists.txt",
                 ".github/workflows/quality.yml", ".github/branch-protection.json", "CMakePresets.json"}
        for path in changed)
    if architecture_policy_changed:
        if "tests/test_architecture.py" not in evidence:
            errors.append("Architecture/policy changes require policy regression test changes")
        if not any(path.startswith("docs/adr/") and path.endswith(".md") for path in evidence):
            errors.append("Architecture/policy changes require an ADR change")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", help="Git base SHA/ref for mandatory changed-test/documentation evidence")
    args = parser.parse_args()
    errors = validate(ROOT)
    if args.base and not errors:
        try:
            errors += validate_changes(ROOT, args.base)
        except subprocess.CalledProcessError as error:
            errors.append(f"Cannot establish change evidence: {error}")
    if errors:
        print("Architecture gate FAILED:\n" + "\n".join(errors), file=sys.stderr)
        return 1
    print("Architecture gate passed" + (" (including change evidence)" if args.base else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
