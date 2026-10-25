# Grayscale workspace redesign — 2026-10-25

## Request / implementation
Removed the inspector/toggle/I shortcut and oversized internal branding bar.
One 62 px session/transport toolbar replaces 130 px of internal header rows.
Track names edit inline; track/mixer M/S and mixer dB inputs preserve editing.
A separate BrowserPanel provides real Session/Templates tabs, scoped search,
selection, scroll, four working in-memory starters and 24-track-cap behavior.
Shared ui_style owns Segoe UI, neutral grays, subtle corners, vector icons and
native focus/tooltip treatment. Track/clip identities alone retain colour.
Only a compact Preview/help affordance explains scope in the workspace.
The generated gray lowercase-s ICO is compiled into the Windows EXE; the runtime
window uses the same art. No new dependency or module; explicit CMake sources.

## Validation
- Local strict GCC C++20 syntax check: GUI sources, app and render test passed
  against pinned JUCE, -Wall -Wextra -Wpedantic -Werror. Not a Windows runtime test.
- Local analysis CTest: 7/7 passed; separate ASan/UBSan/leak CTest: 7/7 passed.
- Python icon/architecture policy suite: 47/47 passed. Generated header is UTF-8/LF
  on every platform, with a matching .gitattributes rule.
- Domain/adapter/shell coverage: 100% lines (150/150), 97.3% branches (215/221),
  100% functions (45/45); Windows-only GUI is outside these totals.
- Added 4 deterministic icon/PNG/ICO tests and expanded native browser, inline
  editing, palette, icon, layout and removed-inspector regressions. Initial icon
  test incorrectly required fully transparent corners at 16 px: corrected to
  require antialias transparency there and fully clear corners at >=48 px.
- Windows Debug/Release, GUI process/control/render and packaged executable-icon /
  installer/portable lifecycle gates: pending CI; no merge until all pass.
- Actual redesigned Windows screenshot will replace prior render after validation.

## Acceptance / limitations
Check compact toolbar and right-edge arrangement, browser tabs/search/selection,
inline track rename, M/S and mixer dB entry, undo/redo and clip gestures; verify
Explorer/window/taskbar s icon. Test minimum size and 100/150/200% DPI, Tab focus,
screen reader, multiple monitors, and a non-admin Windows account manually.
Still an in-memory sketch shell, not an audio engine: no audio, recording, real
media/plugins, saving or recovery. Template edits undo individually. No fabricated
meter activity. Artifacts remain unsigned development builds; commercial rights,
code signing and public distribution remain pending owner confirmation.
