# Native pattern workspace — design and interaction contract

## Appearance and layout
Native C++20/JUCE on Windows 10/11 x64; minimum workspace 1080x640, initial
1440x900. RetroByte Medium, official Lucide icons, grayscale chrome, subtle curves.
Track/source colours are local identities, never a global accent. The executable
icon is the actual font's lowercase s on charcoal. Commercial asset rights remain
unresolved; development use is owner-authorized, not a public release clearance.

One compact transport; no oversized internal brand and no inspector. Files on the
left, reusable **Patterns on the right**, playlist in the middle. Mixer, channel
rack and piano roll share the bottom dock. Drag the three panel boundaries to
resize. Track names and M/S are separate rows with at least 8 px clearance. Browser/pattern widths are bounded and adapt at the minimum window size.
Enter maximizes/restores the playlist. Sunken note grids, edge highlights, restrained
button/fader gradients, fader caps and pan knobs provide depth without fake materials.

## What works
- **Filesystem discovery:** Home/Documents/parent/native folder chooser, asynchronous
  real directory tree, current-folder case-insensitive trimmed filename query,
  audio/MIDI/all-file filters. Folders remain navigable during filtering. No Session/
  Templates, invented sample library or whole-drive recursive search index. Filters
  identify extensions, not valid/decoded media. Double-click folders to navigate.
- **Reusable patterns:** two compact clip cards per row, using the same coloured
  header, body, note preview and selection border as playlist clips. Select a right-hand source then draw into a lane, or drag a
  source into the playlist. New/clone/rename/delete-unused and Make unique. Placements
  share notes; Make unique copies/rebinds only the selected placement. In-use sources
  cannot be deleted. Playlist thumbnails render stored pitch/onset/duration data,
  including source repeats and trimmed/sliced offsets; empty sources have no notes.
- **Editing:** draw/paint/select/delete/mute/slice/zoom tools; move clips across lanes,
  drag the right edge to trim, Shift-drag linked copies, Alt-drag bypasses movement
  snap. Paint places source-length repeats in one undo step. Right-click removes a
  clip. Double-click opens its piano roll. Source edits propagate to linked clips.
- **Channel rack:** four MIDI editing slots (Kick/Snare/Hat/Synth), sixteenth-note
  step toggles. Slots are not hosted instruments and are independent of playlist
  lanes. **Piano:** choose a slot, draw a note with dragged duration; drag an existing
  note to resize, right-click anywhere on its duration to delete. One octave visible by default (6–48 keys with vertical zoom);
  wheel scrolls octaves; a pitch scrollbar reaches the full MIDI range. Horizontal
  scroll/zoom handles and a vertical key-height grip keep editing coordinates
  aligned while zoomed. Note relocation, multiselection, velocity editing and MIDI
  file import/export are not implemented. Duration gestures commit once on release.
- **Mixer:** adjacent inactive Master and full-height track-coloured channel strips (no isolated colour stripe) with name,
  selection, M/S, bipolar pan knob, physical fader cap, dB scale/readout and inactive
  meter well. Lane mute/solo, -60 to +6 dB level and -1 to +1 pan are reversible
  **editing state**, not audio gain/routing. Double-click level/pan resets -6 dB/centre.
  dB text is editable. No fake signal animation or pretending the Master is connected.
- Visual PAT/SONG transport, tempo 40–240, fixed 4/4; PAT loops the active source,
  SONG the 16-bar workspace. Bounded 64-snapshot undo/redo; New/Demo are undoable.
  Limits: 24 lanes, 64 placements/lane, 128 sources, 512 notes/source, 1–64 beat
  sources at 96 ticks/beat. Invalid/capacity/no-op operations consume no undo.

## Contextual keyboard subset
These are deliberate core FL-style mappings, not exhaustive FL Studio parity.

| Key / gesture | Action |
|---|---|
| Space / Ctrl Space / Home | Visual start-stop / start-pause / return to start |
| L | PAT / SONG (stops/returns transport) |
| F4 / F2 | New source + rename / rename source (mixer focus: lane name) |
| F5 / F6 / F7 / F9 | Focus playlist / rack / piano / toggle mixer dock |
| Alt F8 / Alt P / Ctrl F | Browser / right patterns / browser search |
| P / B / E / D / T / C / Z | Playlist draw / paint / select / delete / mute / slice / zoom |
| Ctrl B / Ctrl D / Delete | Duplicate selected placement / deselect / delete selected placement |
| Shift drag / Alt drag | Linked copy / bypass movement snap |
| Horizontal handle edges / centre | Zoom time / scroll time (playlist and piano) |
| Vertical grip | Drag up/down to change lane/key height; double-click resets |
| Ctrl wheel / Alt wheel | Horizontal pointer-anchored zoom / vertical height zoom |
| Middle drag | Two-axis view pan, without editing clips/notes |
| Page Up / Page Down | Horizontal zoom in / out in the focused view |
| Shift 1 / 2 / 3 / 4 | 2× / 4× / 8× time zoom / fit full workspace or source |
| Double-click time scrollbar | Fit full workspace/source horizontally |
| Enter | Maximize / restore playlist |
| S with mixer focus | Solo selected lane (not playlist slip mode) |
| Ctrl Z / Ctrl Y | Sequential undo / redo (explicit project choice) |
| ? / F1 / Escape | Help / help / close help |
| Ctrl S / Ctrl N | Explain unavailable save/version; **never save or reset project** |

Time zoom is bounded to 1–32×; playlist lane heights to 60–180 px, piano to 6–48
visible keys. View navigation adds no undo entries. Selection-region zoom and
FL-specific per-lane height overrides are not implemented.

Native text editing owns its input; piano/mixer focus does not select playlist
tools or duplicate/delete playlist clips. Unsupported commands, including plugin
picker, recording and piano-copy shortcuts, are not mapped to unrelated actions.
An explicit toolbar Remove can remove a selected lane; Delete without a clip does
not silently delete a lane. The Project menu contains New/Demo and disabled Save.

## Honest boundaries
All changes exist **only in memory**. Closing loses them. No project save/recovery,
audio output/recording, hosted instruments/plugins, audition, file import/export,
audio routing or rendered audio. The browser is file discovery, not a working
sampler. File details and on-demand Help state these limits. No scattered marketing
or explanatory paragraphs are used as substitute controls.

`gui -> shell` is separate from `juce_adapter -> engine -> core` and the independent
SDK. The shell owns bounded note/source/placement state; GUI painters own only
geometry. Filesystem work stays on its joined worker; filters use locked copies,
never read text components from a worker. No UI-owned vectors enter an audio callback.
Future media, routing, instruments and persistence require explicit service contracts.

## Verification
`gui_controls_and_render` constructs real JUCE controls, uses temporary filesystem
fixtures, exercises filters, callbacks, source/unique/step/piano duration bindings,
shortcuts/text isolation, slider text/theme, undo, limits, scroll synchronization and
resize bounds. It verifies real note-derived raster previews and writes the native
Windows screenshot. `gui_application` and installer/portable lifecycle tests launch
the actual executable with timed `--smoke-test --report <path>`.

Manual acceptance still required: folder chooser cancellation/network/removable
folders and Unicode names; reverse/rapid drags; focus/popups and keyboard navigation;
100/150/200% DPI, multiple monitors, screen reader/high contrast, installation wizard,
standard-user launch and security prompts. Callback tests do not prove every pointer,
OS shell or accessibility path. No screenshot is evidence until rendered on Windows.
