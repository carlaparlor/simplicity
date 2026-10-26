# Shell state contract

## Responsibility
JUCE-independent, in-memory workspace sketch state: track/clip selection, rename,
mute/solo/levels, add/delete/duplicate/move, grid snap, tempo, bounded undo/redo
and visual transport. This is not an audio graph or a persistence/session format.
Limits: 24 tracks, 64 clips/track, 16 bars of 4/4, tempo 40–240, level -60 to +6 dB,
48-byte names and 64 undo states. Clips may overlap and reference actual note
sources; no audio buffers, devices or MIDI I/O exist.
Failed edits retain state, do not consume undo. Selection and transport are not
undo commands. Restoring an edit restores its selection; transport stays separate.

## Dependencies
C++ standard library only. No JUCE, engine, device, storage or SDK dependency.

## Threading
Single UI-thread owner. Allocates/copies bounded state on edits; never call from
an audio callback. advance is a visual wall-clock animation, not audio scheduling.
Stalls over 60 seconds are bounded; Stop resets position, loop wraps SONG’s 16 bars or PAT’s active source.

## Tests
`tests/workspace_test.cpp`: valid/invalid/boundary edits, selection, snap, NaN/Inf,
loop/end behavior, pause/stop/seek, capacity, undo/redo and branching history.
`tests/pattern_test.cpp`: source/note validation and capacities, linked versus unique
edits, duration editing, paint atomicity, offset slicing, placements, pan and PAT
looping, with undo/redo. Runs on Windows Debug/Release and under internal ASan/UBSan/coverage.

## Extension points
Future real session/command services should replace sketch state behind explicit
adapters, not make UI components audio-thread owners. No project save or stable
serialized format is promised. Do not interpret sketch levels as engine gain.

## Pattern editing milestone
Pattern sources (max 128) contain up to 512 integer-tick notes (96 ticks/beat,
1–64 beat source length, four stable MIDI channel slots, pitch 0–127, velocity
1–127). Clips reference stable pattern IDs and may repeat the source. Borrowed
pointers/references expire on edits/undo/redo. Source creation, linked edits,
unique copies, in-use deletion rejection, placement/paint, trim/slice and note
editing run on the single UI owner; all mutation is bounded and undoable. No
sound, plugin identity, serialization or audio routing is claimed.
