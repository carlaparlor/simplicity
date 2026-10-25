# Shell state contract

## Responsibility
JUCE-independent, in-memory workspace sketch state: track/clip selection, rename,
mute/solo/levels, add/delete/duplicate/move, grid snap, tempo, bounded undo/redo
and visual transport. This is not an audio graph or a persistence/session format.
Limits: 24 tracks, 64 clips/track, 16 bars of 4/4, tempo 40–240, level -60 to +6 dB,
48-byte names and 64 undo states. Clips may overlap; no audio/MIDI content exists.
Failed edits retain state, do not consume undo. Selection and transport are not
undo commands. Restoring an edit restores its selection; transport stays separate.

## Dependencies
C++ standard library only. No JUCE, engine, device, storage or SDK dependency.

## Threading
Single UI-thread owner. Allocates/copies bounded state on edits; never call from
an audio callback. advance is a visual wall-clock animation, not audio scheduling.
Stalls over 60 seconds are bounded; Stop resets position, loop wraps all 16 bars.

## Tests
`tests/workspace_test.cpp`: valid/invalid/boundary edits, selection, snap, NaN/Inf,
loop/end behavior, pause/stop/seek, capacity, undo/redo and branching history.
Runs on Windows Debug/Release and under internal ASan/UBSan/coverage.

## Extension points
Future real session/command services should replace sketch state behind explicit
adapters, not make UI components audio-thread owners. No project save or stable
serialized format is promised. Do not interpret sketch levels as engine gain.
