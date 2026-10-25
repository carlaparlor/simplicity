# ADR 0002: Windows-only support and low-friction delivery

- Status: accepted
- Date: 2026-10-25
- Supersedes ADR 0001's cross-platform product CI requirement (not its module DAG,
  independent domain/SDK, test thresholds or workflow).

## Context

The owner clarified that Simplicity is Windows-only for now and setup/installation
must be as close to zero-friction as possible. They selected **commercial / decide
later** when asked about JUCE licensing; prepare and test installers now, hold
public releases pending confirmation of suitable licensing/distribution terms.

## Decision

Support Windows 10/11 x64 with MSVC 2022; guard default CMake builds on other hosts.
Keep explicit analysis-only Linux jobs for ASan/UBSan/gcov, not product support.
Remove macOS/product OS matrices. Document the platform in agent instructions and
check the supported preset/installer/CI contracts automatically.

Ship per-user Inno Setup installer + portable ZIP test artifacts, with static MSVC
runtime, start-menu launcher, uninstaller, upstream notices and SHA-256 checksums.
Use a stable installer identity and preserve a separate portable copy. The headless
bootstrap launcher pauses so double-click results are visible. Use setup.cmd as a
single developer entry point, discover/install prerequisites through winget, use
CMake presets, and stop on command failures. Do not require Python/compilers/JUCE
setup for an end user. Inno Setup compiler is pinned and hash-verified in CI.

Make real Windows install/reinstall/uninstall/portable/static-runtime/manifest
checks part of Required gates. Keep Debug/Release and previous sanitizer/coverage
thresholds. Public release publication and code signing remain explicitly pending,
not quietly assumed. GitHub CI artifacts are for development evaluation only.

## Consequences and alternatives

No MSI, machine-wide service, auto-updater or additional framework now; these add
privileges/complexity without current need. No CPack/NSIS dependency alongside Inno.
Developer provisioning can require elevation/network/disk; runtime installation
must not. Prepared packages are offline. Unsigned artifact warnings, GitHub login,
standard-user desktop experience and licensing confirmation remain real limits;
"zero friction" is the direction, not a claim that these external barriers vanished.
Do not add a web wrapper or silently broaden supported platforms.
