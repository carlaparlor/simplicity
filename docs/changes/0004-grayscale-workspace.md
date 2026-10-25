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
- Python icon/architecture policy suite: 49/49 passed. Generated header is UTF-8/LF
  on every platform, with a matching .gitattributes rule.
- Domain/adapter/shell coverage: 100% lines (150/150), 97.3% branches (215/221),
  100% functions (45/45); Windows-only GUI is outside these totals.
- Added 5 deterministic icon/PNG/ICO tests and expanded native browser, inline
  editing, palette, icon, layout and removed-inspector regressions. Initial icon
  test incorrectly required fully transparent corners at 16 px: corrected to
  require antialias transparency there and fully clear corners at >=48 px.
- Source review caught a search-overlay override omitting native editor painting;
  restored the base placeholder/outline paint and added a render regression.
- Initial Windows native GUI process/control/render tests passed, but policy tests
  caught cross-platform PNG compression differences. Regeneration now enforces
  exact decoded pixels (plus a fixed pixel digest), local repeat byte identity and
  checked-in runtime/resource byte parity, not zlib compressor identity. A distinct
  compression-variation regression proves this preserves the artwork guarantee.
- Windows run [40234936540](https://github.com/carlaparlor/simplicity/actions/runs/40234936540)
  at 0748194: all 7 jobs passed, including Debug/Release (9/9 CTests each), GUI
  process/control/render, actual packaged-icon extraction and install/reinstall/
  portable/uninstall lifecycle. ASan/UBSan and coverage CI also passed.
- Final update explicitly registers the same image with the native window peer
  as well as DocumentWindow's custom title bar and the executable resource; source
  wiring regression covers both calls. This ensures native taskbar/Alt-Tab binding
  is not confused with custom title-bar painting. Desktop appearance remains a
  manual acceptance check. Final head requires all gates again; its run/head are
  recorded on [PR #4](https://github.com/carlaparlor/simplicity/pull/4).
- Actual Windows render at b840dcf was retrieved and visually reviewed. It exposed
  slider text boxes retaining cached upstream colours/font after dynamic insertion.
  Explicit look-and-feel refresh plus neutral text selection/font regression now
  prevent that drift. The fresh 1440x900 Windows render at 0748194 was retrieved,
  CRC-verified and visually reviewed: docs/images/workspace.png, 50,609 bytes,
  SHA256 f42333d878e41611dc18f65f7a2391e17473eaa8eac6266964c8943edc759228.
  Temporary screenshot diagnostics were removed; normal artifacts remain.

## Acceptance / limitations
Check compact toolbar and right-edge arrangement, browser tabs/search/selection,
inline track rename, M/S and mixer dB entry, undo/redo and clip gestures; verify
Explorer/window/taskbar s icon. Test minimum size and 100/150/200% DPI, Tab focus,
screen reader, multiple monitors, and a non-admin Windows account manually.
Still an in-memory sketch shell, not an audio engine: no audio, recording, real
media/plugins, saving or recovery. Template edits undo individually. No fabricated
meter activity. Artifacts remain unsigned development builds; commercial rights,
code signing and public distribution remain pending owner confirmation.
