# Restore the pre-layout-redesign workspace — 2026-10-27

## Request / exact baseline
Undo the changes requested in “improve the layout … de clutter … triangle”.
Revert PR #8's merge `4b24842327af0ea3668c83adf22791b0ba96d678`, using its
first parent `5cb161534e07bc156f03caf6c50686821284711d` (PR #7) as the baseline.
This is a forward revert on the assigned branch, not a reset or rewrite of main.

## Restored behavior
Restore the previous top/playlist bars and their buttons, previous panel toggles,
rectangular ruler marker and prior mouse handling (including playlist right-click
erase). Remove PR #8's new context menus, ruler seek and triangle. Retain PR #7's
two-column miniature pattern clips, full mixer-strip colours, name/M-S spacing,
shared patterns/editors and FL-style zoom controls. No audio/save capability added.

## Verification
All production files under modules/ and app/ must match the baseline exactly.
Restore the baseline native regression suite and add checks for the restored
buttons, 62 px neutral toolbar and right-click erase/undo. Existing pattern-grid,
colour, spacing, zoom, domain, coverage, sanitizer and installer gates remain.
Local and current-head CI evidence will be recorded in this PR's final comment;
merge only after all seven required checks pass. Visual/DPI acceptance remains
manual; no screenshot inspection is claimed without actually accessing the render.

Local evidence: production-tree comparison to the baseline is empty; analysis
CTest **8/8 passed**, Python policy/assets **62 passed**; coverage **283/284 lines
(99.6%), 73/73 functions, 521/532 branches (97.9%)**, thresholds unchanged.
Windows GUI/runtime, sanitizers and real installer/portable results remain
current-head CI gates, not inferred from an earlier build.
