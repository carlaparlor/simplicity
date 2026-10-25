# Test strategy and reproducible commands

Tests must cover happy paths, boundary values, invalid input, state transitions
and regressions. Random tests use a fixed seed. Assertions use CHECK rather than
`assert`, so release builds still test behavior. Tests avoid timing thresholds.
Do not mark flaky failures as passing or remove tests to unblock a merge.

## Local gates

```sh
python3 tools/check_architecture.py --base origin/main
python3 -m unittest discover -s tests -p 'test_*.py' -v
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel 2
ctest --test-dir build-release --output-on-failure
cmake -S . -B build-sanitizers -DCMAKE_BUILD_TYPE=Debug -DSIMPLICITY_SANITIZERS=ON
cmake --build build-sanitizers --parallel 2
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-sanitizers --output-on-failure
```

On Windows multi-config generators use `--config Debug` and `ctest -C Debug`.
Core-only (`-DSIMPLICITY_WITH_JUCE=OFF`) is a fast offline loop, never evidence that
JUCE integration passed. Linux ASan/UBSan covers JUCE integration in CI. Native
Debug/Release builds cover Linux, macOS and Windows in CI.

## Coverage (GCC/gcov)

Install pinned `tools/requirements-test.txt` in a virtual environment. Do not
commit reports or downloaded libraries.

```sh
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DSIMPLICITY_COVERAGE=ON
cmake --build build-coverage --parallel 2
ctest --test-dir build-coverage --output-on-failure
gcovr --root . --filter 'modules/' --exclude-unreachable-branches --exclude-throw-branches \
  --fail-under-line 95 --fail-under-branch 90 --print-summary \
  --xml-pretty --output build-coverage/coverage.xml build-coverage
```

Thresholds apply to **all first-party modules**, including JUCE adapter and SDK
headers, not upstream JUCE, the bootstrap or test harnesses. They do not prove
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

## User acceptance for this foundation

Build with JUCE enabled, run CTest, then run `simplicity`. Expected: readiness text
and exit code 0; there is intentionally no window or audible playback. Ask for
any desired next DAW feature or report compiler/test failures with OS, command,
logs and reproduction. Human audio, GUI, devices, latency, persistence, extension
loading, race detection and hostile plugin isolation are **not tested/implemented**.
