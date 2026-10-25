# Workspace shell — design and interaction contract

## Visual direction
- True neutral grays: background #181818, panels #202020, controls #292929,
  dividers #363636, text #e5e5e5 / #a0a0a0, selection #414141, focus #c8c8c8.
  No global colour accent; four muted track/clip identities remain.
- Segoe UI, mostly regular 13–14 px; subtle 4 px control and 3 px clip corners.
  Coherent, in-house vector icons; native accessible names, tooltips and focus rings.
- One 62 px session/transport toolbar replaces the 64 px branding + 66 px transport
  rows. No oversized internal logo. The normal 26 px window title bar remains.
- No inspector, inspector button, or I shortcut. Arrangement uses the right edge.
  Rename tracks inline; mute/solo on track headers and mixer; level in mixer.
- A single quiet Preview help affordance explains limitations. No scattered
  explanatory paragraphs; empty/no-match labels and necessary control metadata remain.
- Actual executable icon: lowercase s on gray, generated without third-party fonts
  or art. Seven ICO sizes 16–256 plus matching runtime PNG; see tools/assets/.

## What works now
- Native resizable Windows GUI; minimum workspace 1080x640; initial 1440x900.
- Session menu opens New / Demo; double-click session title or a track name to edit.
- Browser **Session** tab navigates actual session tracks; **Templates** adds Empty,
  Pattern (1 sketch), Rhythm (4 sketches) or Texture (2 sketches) tracks. These are
  in-memory visual blocks, **not** media files, samples, instruments or plugins.
  Search is case-insensitive, trimmed and scoped to the selected tab; scrolling
  handles all 24 tracks. Names, selection, deletion, undo and redo stay synchronized.
  New track adds an empty track. Templates disable at the 24-track cap.
- Select sketch clips, drag horizontally with selectable snap, add a four-beat
  sketch at playhead, duplicate/delete, zoom and two-axis arrangement scrolling.
  Empty lane double-click adds a sketch; clicking the lane seeks visual transport.
- Track M/S and mixer M/S stay in sync. Mixer fader numeric dB fields are editable,
  with a -60 to +6 dB range; double-click a fader to return to -6 dB.
- Bounded 64-step edit undo/redo. New and Demo are undoable. Template track/clip
  additions are individual undo steps, not an atomic multi-edit transaction.
- Visual play/pause/stop, tempo 40–240, 16-bar loop; fixed 4/4.
- Mixer meters deliberately show no signal. Browser/mixer can hide; help is on demand.

| Key | Action |
|---|---|
| Space / Enter | Visual play-pause / stop and return |
| Ctrl N | New sketch (undo restores previous layout) |
| Ctrl Z / Ctrl Y | Undo / redo |
| Ctrl D / Delete | Duplicate selected clip / delete clip or selected track |
| B / M | Browser / mixer |
| ? / Escape | Help / close help |
| Ctrl S | Open help explaining saving is unavailable (never claims to save) |

Typing in native text editors does not invoke workspace shortcuts. Changes exist
**only in memory**; closing loses them. No save/recovery/confirmation workflow exists.
No audio device/playback, recording, MIDI instruments, plugin hosting, file import/
export, signal routing or rendered audio is implemented. The disabled record button
and disabled Save menu item do not pretend otherwise.

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
bindings/state/shortcuts and resize bounds, verifies neutral toolbar pixels, vector icons, application icon, reclaimed layout space and
writes `build/windows/screenshots/workspace.png`. `gui_application` launches the
GUI subsystem executable with `--smoke-test --report <file>` and checks its exit.
Installer/portable tests repeat this from installed/extracted paths with spaces.
No persistent user data is written by smoke tests; only the explicit report path.
Smoke mode times out in CI rather than hanging an invisible window indefinitely.

Manual checklist: check Explorer/title/taskbar icons, switch browser tabs and filter,
rename tracks inline, edit mixer dB fields, drag/select clips, use Tab focus, zoom/scroll,
exercise undo, resize to minimum and 1920 wide, test 100/150/200% Windows DPI,
multiple monitors, screen reader/high contrast, standard-user launch and security
prompts. Native test callbacks do not prove all pointer/focus/accessibility paths.
No screenshot is described as evidence before actually rendering it on Windows.
