# Test strategy and reproducible commands

Tests must cover happy paths, boundary values, invalid input, state transitions
and regressions. Random tests use a fixed seed. Assertions use CHECK rather than
`assert`, so release builds still test behavior. Tests avoid timing thresholds.
Do not mark flaky failures as passing or remove tests to unblock a merge.

## Supported Windows gates

```bat
setup.cmd -NoInstall
setup.cmd -NoInstall -Configuration Debug
```

Without `-NoInstall`, setup installs missing developer prerequisites through
winget. Both commands configure the real JUCE-enabled MSVC x64 preset, build, and
run CTest including policy, sketch-state, native GUI control/render and GUI process tests. No developer command prompt is needed. CI uses
the same command, not a separate bypass. Windows PowerShell 5.1 tests parse the
scripts and verify native/cmd failure propagation and NoInstall behavior.

The separate required package job runs `tools/windows/package.ps1`, then
`tests/windows/installer_test.ps1`: hashes, compiled asInvoker manifest, absence of
separate MSVC runtime imports, portable launch, actual silent install/reinstall,
Start menu/HKCU registration, installed launch and full uninstall, using spaced
paths with `&`. The runner is elevated; manual standard-user desktop/security
prompt testing remains essential. See [Windows setup](windows-setup.md).

## Internal analysis only (unsupported host, not product builds)

```sh
python3 tools/check_architecture.py --base origin/main
python3 -m unittest discover -s tests -p 'test_*.py' -v
cmake -S . -B build-sanitizers -DCMAKE_BUILD_TYPE=Debug -DSIMPLICITY_ANALYSIS_ONLY=ON -DSIMPLICITY_SANITIZERS=ON
cmake --build build-sanitizers --parallel 2
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-sanitizers --output-on-failure
```

These Linux jobs are solely sanitizer/coverage tools for portable domain/adapter
code. They do not establish Linux/macOS product support, Windows device safety or
MSVC sanitizer behavior. Default non-Windows CMake configuration is rejected.
Core-only builds are a fast offline loop, not a substitute for real JUCE CI.

## Coverage (GCC/gcov)

Install pinned `tools/requirements-test.txt` in a virtual environment. Do not
commit reports or downloaded libraries.

```sh
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DSIMPLICITY_ANALYSIS_ONLY=ON -DSIMPLICITY_COVERAGE=ON
cmake --build build-coverage --parallel 2
ctest --test-dir build-coverage --output-on-failure
gcovr --root . --filter 'modules/' --exclude-unreachable-branches --exclude-throw-branches \
  --fail-under-line 95 --fail-under-branch 90 --print-summary \
  --xml-pretty --output build-coverage/coverage.xml build-coverage
```

Thresholds apply to **all compiled first-party domain/adapter/shell modules**,
including JUCE adapter and SDK headers, not upstream JUCE or test harnesses. The
Windows-only GUI is not compiled by internal Linux analysis and is explicitly
outside those coverage totals; its real control/render/process tests are required
on Windows Debug/Release (ADR 0003). Existing thresholds have not been lowered. They do not prove
correctness. CI uploads coverage XML and native CTest logs even on test failure.
The Python architecture policy suite includes mutation and change-evidence tests;
it is not included in the C++ coverage percentage.

## Current suites

- core: supported format bounds, zero/overflow-like capacities, NaN/infinite rates.
- engine: prepare/reprepare failure, gain rejection/preservation, empty/oversized
  blocks, silence, amplification, 1,000 deterministic randomized block checks.
- SDK: real C11 consumer + C++ layout/compatibility checks; missing callbacks,
  null descriptors, old/extended sizes, mismatched ABI; actual callback invocation.
- JUCE adapter: planar channel mapping, invalid dimensions with no partial writes,
  zero buffers, gain errors and failed/recovered preparation.
- bootstrap: links/runs actual pinned JUCE modules (not mocks).
- architecture: valid repository plus deliberate forbidden dependency, cycle,
  bypass, missing-doc/test and change-evidence mutations.

## GUI suites / user acceptance

Windows adds `workspace_test`, `gui_controls_and_render` and `gui_application`.
The render test writes a real JUCE PNG; native process smoke mode validates the
same controls and produces a JSON report with a CI timeout. Installer and portable
lifecycle tests run that native process from installed/extracted paths, verify the
shortcut targets the GUI executable directly and still test static runtime/privileges.
Native rendering tests exercise state binding and three sizes, not complete pointer,
DPI or assistive-technology correctness. Read [GUI guide](gui-shell.md) for the
manual acceptance checklist and actual feature limits. No audio, media, saving,
recording, timing/device safety or plugin compatibility is implied by GUI success.
