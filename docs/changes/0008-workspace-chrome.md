# Consolidated chrome and contextual editing — 2026-10-26

## Request
Declutter/rearrange the top two bars into a coherent DAW layout, move secondary
actions to right-click menus and give the playhead a real triangular marker.

## Scope / contract
Native C++20/JUCE, Windows-only. Group project, transport and view controls in one
global bar; keep only playlist-local editing/navigation in the second. Provide
clip/lane/empty-playlist context menus and Project/View command menus. Preserve
shortcuts, source grid, coloured strips, zoom, text-input protection and explicit
unavailable audio/save services. No new module edge or audio/persistence claims.

## Verification
- Implemented centered project / transport / View grouping, compact clock, one
  local playlist bar, grouped bottom view tabs and one quiet audio-off status.
- Removed secondary buttons entirely (not merely hidden); native Project/View
  menus and contextual clip/lane commands share target-validated dispatch.
- Clip/lane/header/mixer-strip popups use lifetime-safe asynchronous callbacks;
  stale/cancel/disabled/invalid commands are rejected. Piano right-click erase is
  deliberately preserved as its documented rapid editing gesture.
- Added downward ruler triangle aligned to the line, with zoom-correct ruler seek.
- Native regressions cover enabled menu IDs, actual popup open/dismiss, shared
  dispatch/undo, right-click non-destruction, stale/cancelled commands, disjoint
  toolbar controls at 1080–1920 widths and both panel width extremes, triangle
  raster taper and zoomed ruler seek/render. Existing source/zoom tests retained.

Local strict C++20 GUI syntax check passed; analysis CTest **8/8 passed** including
62 Python policy/asset tests. Coverage/sanitizer and Windows native/installer
results are recorded in the PR's required checks and final evidence comment.
All seven current-head gates must be green before explicit merge. GUI rendering
is not included in domain coverage; no visual inspection is claimed until a
native screenshot can actually be accessed. Manual DPI, menu focus/keyboard,
pointer feel and visual acceptance remain user tests.

## User test checklist
At minimum and normal window sizes: verify Project on the left, transport centred,
View on the right; one playlist row for tool/snap/time-zoom. Right-click clips,
empty lanes, lane names and mixer-strip backgrounds; cancel should not delete or
move anything. Try duplicate/mute/unique/delete and undo, plus New/Demo/Help and
panel choices in menus. Seek in the ruler before/after zoom and check triangle /
line alignment. Confirm source grid, full mixer colour and name/M-S gap persist.
Audio, import/audition, recording, plugins and saving remain unavailable.

## Native evidence and artifact access
[PR #8](https://github.com/carlaparlor/simplicity/pull/8) records current-head gates.
Windows Debug/Release native tests passed at `8bd1d5f` in
[run 40408827179](https://github.com/carlaparlor/simplicity/actions/runs/40408827179),
as did policy, coverage and sanitizers. The final follow-up fixes a stale README
render link to always point to successful main builds and explicitly invalidates
the ruler during layout (including zoom without a scroll notification). Its final
checks must also pass before merge; see the PR evidence comment for that run.
Local ASan/UBSan: **8/8 passed**, with leak detection; local coverage **283/284
lines (99.6%), 73/73 functions, 521/532 branches (97.9%)**, thresholds unchanged.
Native artifacts were generated, but download redirected to storage outside the
sandbox host allowlist and failed with EOF. No visual screenshot inspection is
claimed. Download current screenshots / unsigned development installer and portable
ZIP from the successful run's artifacts; no public release or licensing clearance.
