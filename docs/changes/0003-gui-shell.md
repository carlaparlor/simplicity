# 0003 — Blocky dark native GUI shell

Date: 2026-10-25

## Request
Design/create an intuitive, minimalist dark/blocky GUI shell that can grow into
a powerful DAW. Windows-only and low-friction installation remain requirements.

## Implementation
New native GUI and independent shell state modules; declared dependencies and
contracts. Arrangement/browser/transport/inspector/mixer, reversible in-memory
sketch editing, clip dragging/snap, zoom/scroll, track states, panel toggles and
keyboard help. No fake playback/recording/meters/saving. Native GUI executable
replaces console pause launcher; installer/portable/shortcuts launch it directly.

## Verification
- Architecture checks and 43 Python policy tests passed locally.
- Internal JUCE/domain/shell coverage build: 7/7 CTest suites passed.
- All instrumented first-party modules: 100% lines (150/150), 97.3% branches
  (215/221); Windows-only rendering is explicitly outside GCC coverage.
- Internal ASan/UBSan/leak detection: 7/7 suites passed, no reports.
- Native Windows Debug/Release controls/render/app tests passed at the initial
  GUI head; final theme/resized-window/packaging checks remain required at final head.
- Own GUI/app/test source passes GCC syntax checks against actual pinned JUCE headers.
- Actual Windows build/render/control/native launch/installer results recorded on PR
  after execution, not inferred from Linux. Screenshots are actual JUCE renders.

## Limits / user acceptance
In-memory GUI shell, not yet playable production DAW. No audible output, real
recording/MIDI/media contents, plugins, routing or project save/load. Manual
pointer/DPI/screen-reader/security checks remain. Commercial distribution/signing
is still on hold. Try the prepared Windows bundle after merge; see docs/gui-shell.md.
