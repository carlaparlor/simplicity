# 0002 — Windows-only support and low-friction setup

Date: 2026-10-25

## Request and decision
Windows only for now; make setup and installation as easy as possible. Owner chose
commercial / decide-later licensing: prepare/test installers, hold public releases.

## Changes
- Windows 10/11 x64/MSVC product contract, checked CMake preset and unsupported-host
  guard. Linux remains analysis-only; no macOS/Linux product build matrix.
- setup.cmd detects/installs missing development tools, fetches JUCE, builds/tests.
  No global execution-policy changes; NoInstall mode and command failures tested.
- Per-user/no-admin installer, start menu/uninstaller, portable ZIP, checksums,
  bundled notices; static MSVC runtime avoids a separate runtime installer.
- Required Windows packaging/lifecycle CI gate and setup/script tests; policy
  mutation tests now guard Windows scope, preset, privilege and runtime choices.
- Windows-first docs with actual remaining barriers; no public release publishing.

## Verification
Local internal-analysis environment (Debian, GCC 12.2, CMake 4.4.4, Python 3.11):
- Default unsupported-host configure rejects with the intended Windows-only message.
- Architecture contract and 38 policy mutation/change-evidence tests: passed.
- JUCE-enabled internal coverage build: 7/7 CTest suites passed.
- JUCE-enabled internal ASan/UBSan/leak detection: 7/7 suites passed, no reports.
- Initial Windows CI found windows-latest ships a newer Visual Studio, not the
  supported VS2022 toolchain. Pinned CI to windows-2022 rather than bypassing setup
  prerequisites; added a regression policy check and actionable failure annotations.
- Script regressions also cover broken Python/Store aliases and executable paths
  with spaces. Lifecycle tests refuse to overwrite a real existing user install.
- Module coverage unchanged: 100% lines (31/31), 98.1% branches (53/54).
Windows setup/installer results are recorded on the PR after actual hosted runs;
Linux results are not represented as Windows installer evidence.
No Windows desktop, SmartScreen, signing, licence approval or standard-user manual
acceptance is claimed. Hosted Windows CI tests real generated artifacts.

## User test checklist after merge
Download the successful main workflow artifact, extract it, install per-user and
launch from Start. Expect a readiness console, not a DAW window. Reinstall and
uninstall via Settings. Try portable launch independently. Confirm standard-user
behavior, installer wizard and security prompts on your actual Windows desktop.
