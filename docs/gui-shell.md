# Workspace shell — design and interaction contract

## Visual direction
- Charcoal, not pure black; square blocks, 1 px dividers, no decorative gradients.
- Quiet off-white text, subdued metadata, lime reserved for focus/active transport.
- Four muted track identities carry from lane to clips to mixer.
- Native controls, readable labels, clear focus rings/tooltips; no icon guessing.
- One workspace, not a dashboard of unrelated cards. Transport stays at the top;
  details stay beside the selection; advanced panels hide when not needed.

## What works now
- Native resizable Windows GUI; minimum workspace 1080x640; initial 1440x900.
- Demo / New empty sketch, editable session title (double-click), searchable track
  starters, up to 24 tracks, add/delete/rename/select, mute/solo and level state.
- Select sketch clips, drag horizontally with selectable snap, add a four-beat
  sketch at playhead, duplicate/delete, arrangement zoom and two-axis scrolling.
  Empty lane double-click adds a sketch; clicking the lane seeks visual transport.
- Bounded 64-step undo/redo for edits. New and Demo can also be undone.
- Visual play/pause/stop, tempo 40–240, 16-bar loop; fixed 4/4.
- Mixer channel faders mirror inspector state. Meters deliberately show no signal.
- Searchable browser, mixer and inspector toggle; overlay with shortcut help.

| Key | Action |
|---|---|
| Space / Enter | Visual play-pause / stop and return |
| Ctrl N | New sketch (undo restores previous layout) |
| Ctrl Z / Ctrl Y | Undo / redo |
| Ctrl D / Delete | Duplicate selected clip / delete clip or selected track |
| B / M / I | Browser / mixer / inspector |
| ? / Escape | Help / close help |
| Ctrl S | Explain that saving is not available (does not claim to save) |

Typing in text editors does not invoke workspace shortcuts. Changes exist **only
in memory**; closing the app loses them. No save/recovery/confirmation workflow
exists yet. The demo is not media: blocks/notes are illustrative placeholders.
No playback device, audible metronome, recording, MIDI instruments, plugin hosting,
file import/export, signal routing or rendered audio is implemented.

## Architecture toward a powerful DAW
`gui -> shell` is separate from `juce_adapter -> engine -> core` and independent
SDK. The GUI currently owns sketch state, not audio buffers or an engine graph.
Future session commands, media/timeline, device/transport scheduling, automation,
plugins and project persistence should enter through explicit service boundaries.
Do not pass a UI-owned vector to an audio callback or make layout code own devices.
This structure allows progressive disclosure without restricting future depth;
it is not a claim of current feature parity with established DAWs.

## Verification / acceptance
`gui_controls_and_render` creates real JUCE components, exercises native callback
bindings/state/shortcuts and resize bounds, verifies rendered palette pixels and
writes `build/windows/screenshots/workspace.png`. `gui_application` launches the
GUI subsystem executable with `--smoke-test --report <file>` and checks its exit.
Installer/portable tests repeat this from installed/extracted paths with spaces.
No persistent user data is written by smoke tests; only the explicit report path.
Smoke mode times out in CI rather than hanging an invisible window indefinitely.

Manual checklist: drag/select clips, use text edits and Tab focus, zoom/scroll,
exercise undo, resize to minimum and 1920 wide, test 100/150/200% Windows DPI,
multiple monitors, screen reader/high contrast, standard-user launch and security
prompts. Native test callbacks do not prove all pointer/focus/accessibility paths.
No screenshot is described as evidence before actually rendering it on Windows.
