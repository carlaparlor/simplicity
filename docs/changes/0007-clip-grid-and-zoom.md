# Compact clip grid, full-strip colour and FL-style zoom — 2026-10-26

## Request
Match pattern-picker cards to miniature playlist clips, two per row. Colour full
mixer strips instead of a thin horizontal band. Separate track names from M/S.
Add FL-style zoom controls to the playlist and piano roll.

## Reference and bounded scope
Official FL manuals describe horizontal scrollbar-handle edge zoom, a vertical
height grip, Ctrl-wheel horizontal zoom, middle-button two-axis pan, Page Up/Down
and Shift+1..4 presets:
- https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/playlist.htm
- https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/pianoroll.htm
- https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_interface.htm

Implement those navigation controls with shared JUCE components; not proprietary
source/artwork or exhaustive FL parity. Selection-region zoom is not advertised
without region selection. Preserve Windows-only C++/JUCE, audio/save limitations,
neutral chrome and local track colours. No new module dependency.

## Evidence
- Shared clip renderer with two-column native button grid; selection, double-click,
  tooltips, keyboard activation and source drag remain available.
- Full-height dark track-colour mixer surfaces; no horizontal colour stripe.
- Track-name/M-S gap of at least 8 px; lane height starts at 72 px and zooms 60–180.
- Shared horizontal edge-zoom/centre-pan bars and vertical grips in both views;
  pointer-anchored Ctrl-wheel, middle pan, Page Up/Down, Shift+1..4. Piano pitch
  scroll and note hit tests use the same view transform as drawing. Time zoom
  1–32×; piano vertical zoom 6–48 keys. No music/history changes from navigation.
- Regression tests cover min/max/invalid ranges, handle edges, pan, callback/key
  bindings, two-column empty/odd/min-width layout, shared clip raster, full-strip
  colour, name spacing and editing/undo under piano zoom.

Local strict C++20 GUI syntax check and analysis CTest **8/8 passed** (including
62 Python policy/asset tests). Native GUI runtime tests are Windows-only; local
syntax checking is not described as native render validation. Coverage/sanitizer
and current-head native/installer results are tracked by the PR's required checks.
All seven must be green before explicit merge. Screenshot inspection remains
pending unless the native artifact can actually be downloaded; no mockups.

## Manual user acceptance
Verify pairs of pattern miniatures, source drag/double-click, full coloured mixer
channels and separated name/M-S. In both playlist and piano: drag either horizontal
handle edge, pan its centre, drag the upper-right vertical grip, Ctrl-wheel at a
point, middle-drag, Page Up/Down and Shift+4. Draw/resize/delete notes while zoomed,
then undo; confirm source data changes but navigation is not in undo history.
Check minimum-size layout, DPI and pointer feel. Selection-region zoom and per-lane
height overrides are not advertised. Audio/import/plugins/saving remain unavailable.
