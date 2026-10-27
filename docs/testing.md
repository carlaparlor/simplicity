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
separate MSVC runtime imports, compiled grayscale executable icon, portable launch, actual silent install/reinstall,
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

## Grayscale redesign regressions

`tests/test_app_icon.py` checks all seven ICO directory frames, PNG signatures,
chunk lengths/CRCs, size/format, neutral pixels and antialiased corners, exact
runtime/resource PNG byte parity and exact decoded-pixel regeneration. PNG
compression bytes may differ across zlib/zlib-ng versions; local repeat generation
is byte-identical, and a fixed pixel digest guards the artwork independently. Regenerate
with `python tools/assets/make_app_icon.py`; no image/font libraries are needed.
Windows installer tests extract the actual packaged EXE icon with System.Drawing,
not merely inspect the RC source, and require the gray background/white glyph.
GUI tests render all vector glyphs, neutral normal/active buttons and toolbar;
assert the 62 px toolbar, arrangement right edge/minimum width, removed I shortcut,
inline rename, dB edits, searchable tab navigation/templates, 24-track cap, browser
scrolling, deletion/undo synchronization, help, transport and clip dragging/undo.
Snapshots remain real native JUCE renders, not design mockups.

## Lucide / RetroByte / charcoal regressions

`test_ui_assets.py` verifies pinned upstream SVG hashes, standard viewBox/stroke,
license notices, exact embedded TTF/SVG bytes, deterministic regeneration, the
actual TTF s polygon points and unsupported/missing glyph rejection. The small
font-outline reader is build tooling for this pinned simple-polygon font, not a
runtime/general-purpose untrusted-font parser. A one-off FontTools 4.60.1 audit
independently matched all 46 points; it is not a build/test dependency.
`gui_controls_and_render` verifies the loaded face is RetroByte even without an OS
installation, renders every Lucide symbol and neutral tint reset, tests invalid/
empty icons, and compares the icon's opaque pixels to the actual native font s
path. Existing resize/control/shortcut/installer/sanitizer/coverage gates remain.
Installer tests require both license notices and extract the charcoal icon from
actual portable/installed executables. These tests do not grant font rights.

## Clip-grid and zoom feedback regressions
Native GUI tests verify two columns at min/default/max picker widths, odd/empty
counts, miniature/playlist shared clip raster, full-height mixer colour pixels,
and an 8 px track-name/M-S gap at minimum/default/maximum lane zoom. The shared
time bar tests both handle edges, centre pan, invalid ranges, min/max limits,
callbacks and keyboard navigation; grip tests include drag/reset/keys. Integrated
playlist tests exercise scroll/zoom/pan bindings without changing undo/selection.
Piano tests create and erase notes through zoomed coordinates and verify undo
restores the note rather than a view change. Existing file/installer/coverage gates
are unchanged. Manual pointer feel, DPI and OS accessibility acceptance still apply.

## Menu-bar / centred transport regressions
Native tests require File/Edit/View/Help, absence of the folder button, correct
availability, non-mutating cancel/disabled/wrong-menu commands, File/Edit/View/Help
dispatch and native File popup open/dismiss. Stop-to-record bounds must be centred
within half a pixel at 1080–1920 widths; all transport controls must be pairwise
disjoint and inside their row. Menus use 14 px RetroByte with neutral normal/active
rendering; palette checks cover both menu and transport rows. Record stays disabled.
The prior playlist, rectangular marker, right-click erase/undo, source grid, mixer
and zoom regressions are retained. Menu keyboard/DPI/visual acceptance is manual.
