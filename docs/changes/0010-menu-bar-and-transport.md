# Traditional menu bar and centred transport — 2026-10-27

## Request / bounded scope
Replace the top-left file/folder-looking icon with File/View/etc menus. Centre
play/stop/record and position related controls alongside them. Build on the reverted
PR #7 workspace, not the rejected PR #8 redesign. Keep existing playlist controls,
marker, gestures, pattern grid, mixer colours and zoom. Native Windows C++20/JUCE;
record/save remain explicitly unavailable.

## Implementation and verification
A JUCE menu bar owns File/Edit/View/Help menus backed by current command state.
Stop/play/record are centred as a 112 px group, with mode/tempo left and loop/clock/
snap right. Project title and undo/redo remain left; panel/help shortcuts right.
Native regressions must cover menu names, availability/dispatch/cancel/invalid
choices, file-button removal, centred geometry and pairwise non-overlap at
1080–1920 widths, while retaining all previous interaction tests.

Local and current-head CI results will be recorded in the PR evidence comment.
All seven required gates must pass before explicit merge. Screenshot/DPI/keyboard
menu acceptance remains manual unless actual native output is inspected.

Local evidence: strict C++20 GUI syntax check passed against pinned JUCE; analysis
CTest **8/8 passed**, including **62 Python tests**. Coverage **283/284 lines
(99.6%), 73/73 functions, 521/532 branches (97.9%)**, thresholds unchanged; GUI
runtime/rendering is tested by Windows CI, not counted in domain coverage.
Native tests also open/dismiss a real File popup, verify neutral 14 px RetroByte
menu rendering and test Edit duplicate/delete on selected placements. Record is
visible but disabled; File Save cannot dispatch an unrelated action.
