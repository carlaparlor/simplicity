# Simplicity — browser, pattern workflow, clips, mixer and depth research

**26 October 2026 · Research and proposed specification, not an implemented redesign**

## Recommendation

**Ableton-style sound discovery on the left; FL-style composition and editing in the centre; reusable patterns on the right; a proper mixer/editor dock below.** Keep the requested RetroByte, Lucide, charcoal, restrained curves and grayscale chrome. Add tactile depth without turning the interface into a glossy imitation of either product.

The principal mistake in the current application is its information model, not just its styling. A track list plus four track-creation presets is not a sound browser. Repainting independent sketch rectangles does not create a pattern workflow.

## 1. What the references establish

### Ableton: the browser is for discovering musical assets

Live 12 separates its browser into a search bar, sidebar, filters, results/content pane and preview. The sidebar has Collections, Library and Places. Places includes user folders, the current project and Add Folder. Selecting a category/location changes the results; it does not merely select an arrangement track. Search, filters, navigation history and preview support a find–evaluate–use workflow. [1](https://www.ableton.com/en/live-manual/12/working-with-the-browser/)

Live's support documentation distinguishes the newer Library labels from the older Categories terminology. Its examples include Sounds, Drums, Instruments and Plug-ins. This is the useful organizational principle to borrow—not every tag, cloud feature or label. [2](https://help.ableton.com/hc/en-us/articles/12927340213660-The-Live-12-Browser)

**Proposal for Simplicity:** remove the primary **Session / Templates** tabs. Put actual asset discovery here:

- Search across enabled library locations, with optional category/type filters.
- A compact category/location column and a results column, separated by a draggable divider.
- Favorites; supported content categories such as Samples and MIDI; User Library; Current Project; user-added folders; Add Folder.
- Instruments/Effects when real inventory/scanning/hosting exists—not an invented list of working plugins.
- Folder expansion, keyboard navigation, back/forward, sorting, full-path tooltips, explicit missing-folder and no-result states.
- A compact preview area when real decoding/output exists. Until then, no animated waveform or audition button pretending to play audio.
- Drag targets that explain what will actually happen. A folder navigates; a supported file imports through a real service. Unsupported formats fail clearly without changing the project.

Templates may eventually belong in New Project or a secondary library category. They should not be half of the main browser. Track selection already belongs in the playlist headers and mixer.

**Core interaction to support:** search → choose result → preview if supported → drag into a valid destination → return to composing. Live documents Ctrl+F, Down into results, arrow navigation and Escape clearing search/filter state; adapt that usable keyboard flow deliberately. [1](https://www.ableton.com/en/live-manual/12/working-with-the-browser/)

### FL Studio: patterns are sources, playlist clips are placements

FL's Channel Rack is a shared collection of instrument channels. A pattern can contain step sequences, piano-roll notes and event automation across multiple channels; it is not necessarily a single instrument. Selecting a different pattern changes the displayed musical data, not the instrument inventory. [6](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/channelrack.htm)

The Playlist arranges Pattern, Audio and Automation clips. Playlist lanes are not inherently instrument channels or mixer strips. FL offers explicit instrument/audio track linking as a convenience, but that does not erase the distinction. [1](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/playlist.htm)

The Picker Panel manages project clip sources. A source can be dragged into the Playlist, or selected and then placed. Ordinary instances share source data; **Make unique** creates independently editable content. [1](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/playlist.htm)

**Proposal for Simplicity:** put this pattern-management role on the **right**, as requested. Do not describe the right-side placement as a requirement imposed by FL. It is our explicit layout choice.

## 2. The pattern workflow to build

### Distinct objects

| Object | Owns / means |
|---|---|
| Instrument channel | Instrument/sampler identity and routing destination |
| Pattern | Stable ID, name, color, musical length, note/step data keyed by instrument channel |
| Pattern instance | Its own stable ID, reference to a pattern, playlist lane, start, source offset and visible duration |
| Playlist lane | Arrangement organization, not automatic ownership of an instrument or pattern |
| Mixer strip | Gain/pan/routing/effects state, separate from playlist positioning |

Use stable IDs, not visible row numbers as references. Reordering patterns must not silently redirect clips. Prefer documented integer musical-time units for note starts/durations; painting converts musical time to pixels.

### Right-hand Patterns pane

- Persistent, resizable and independently collapsible; never a renamed inspector.
- Rows with pattern number, editable name, optional color marker, musical length and a small data-derived preview.
- New pattern, rename, clone source, delete source, reorder, search and select-used-in-playlist operations.
- Clicking a row chooses the active pattern; dragging places an instance. Draw/Paint tools use that active source.
- Clear distinction between **clone pattern** (new source) and **duplicate placement** (another reference to the same source).
- Delete placement must not delete the pattern. Deleting a source in use needs an explicit, undoable policy.

### Editing and arranging

- A visible **PAT / SONG** mode switch and active-pattern selector in the transport. FL documents PAT as the current pattern and SONG as the Playlist arrangement. [6](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/toolbar_panels.htm)
- A Channel Rack / step editor and Piano Roll, reachable by keyboard and by opening a pattern. Both edit the same underlying notes rather than maintaining two divergent copies.
- Draw places one instance; Paint repeats it across a drag; moving allows horizontal and vertical placement; resizing trims an instance without silently rewriting shared source notes.
- Double-click a pattern clip to edit its content; explicitly choose the target channel when a pattern has several. FL documents piano-roll/channel-rack opening behavior and configurable edit targets. [1](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/playlist.htm)
- Edit the source once and update every linked instance and preview. Make unique forks only the chosen instance's source.
- One paint stroke, Make unique operation or multi-selection move should each be one undo transaction.

A genuine first milestone is **reusable note data and working editing/placement**, not merely a right-hand list labelled Patterns. Audio scheduling, instrument playback and durable saving remain separate necessary milestones before calling this a usable music-production workflow.

## 3. MIDI/pattern clips: replace decorative blocks with information

FL describes pattern clips as note/event data, with names in their title bars and optional combined note/automation display. Its clip color can come from note, channel or pattern settings. [2](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/playlist_patterns.htm)

**Proposed visual treatment:**

- Compact clip with a narrow, distinct name/header strip; a quieter tinted body rather than a large washed-out slab.
- Thin piano-roll-like note bars below the header. Horizontal position means onset; width means duration; vertical position means pitch.
- Drum/step data reads as short rhythmic hits; sustained notes stay visibly sustained. Chords appear as stacked notes.
- All appearances of a given pattern use consistent preview mapping. Trimming crops the preview; looping repeats actual source content. Do not randomly reposition notes to fill a rectangle.
- Faint beat/bar guides, restrained edges, small corners, a clear selected outline and distinguishable muted state.
- At low zoom, simplify or aggregate the real data. Preserve visibility of short hits without fabricating notes. Empty patterns have an empty body.
- Pattern identity controls its color; moving it to another playlist lane must not accidentally turn it into a different pattern.

**Repository finding:** `SketchClip` currently contains only ID, name, beat and length. `Canvas::paint` manufactures marks with a step/modulo formula; it does not read MIDI notes. The row term is a multiple of the same modulo, so it does not even produce the intended row-dependent variation. A proper note model and preview renderer must replace this, not just a new palette.

## 4. Shortcut compatibility means context and gestures, not a few F-keys

Image-Line explicitly states that shortcuts depend on the active window; single-letter shortcuts are affected by Typing Keyboard to Piano. Its published table also differs from Simplicity's current B/M, Ctrl+D and Enter behavior. [4](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_shortcuts.htm)

### Baseline Windows compatibility targets

| Context | Key / gesture | FL-documented behavior to target |
|---|---|---|
| Transport | Space | Start/stop |
| Transport | Ctrl+Space | Start/pause |
| Transport | L | PAT / SONG |
| Pattern selector | F4 | Next empty pattern with naming dialog in the shortcut table |
| Pattern selector | Numpad + / − | Next / previous pattern |
| Selection | F2 | Rename the selected contextual object |
| Views | F5 / F6 / F7 / F9 | Playlist / Channel Rack / Piano Roll / Mixer |
| Views | Alt+F8 | Browser |
| Playlist | Alt+P | Picker; our right-hand Patterns pane |
| Playlist / Piano Roll | P / B / C / E / T / Z | Draw / Paint / Slice / Select / Mute / Zoom |
| Playlist | D | Delete tool |
| Playlist / Piano Roll | Ctrl+B | Duplicate selection to the right |
| Playlist / Piano Roll | Ctrl+D | Deselect—not duplicate |
| Playlist / Piano Roll | Ctrl+A, C, X, V | Select all, copy, cut, paste |
| Playlist | Shift+drag a clip | Clone a placement |
| Editing | Alt during drag | Temporarily bypass snap |
| Playlist / Piano Roll | Right-click in editing context | Delete behavior appropriate to the active tool/context |
| Mixer | S | Solo selected strip; do not globally bind this over Playlist slip-edit |
| Playlist | Enter / Home | Max/min Playlist / move marker to start—not Simplicity's current Enter=stop |

All these rows derive from the official shortcut table; the **scope** is part of each binding. [4](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_shortcuts.htm)

**Important compatibility traps:**

- FL's shortcut reference calls Ctrl+Z last-edit undo/redo and Ctrl+Alt+Z step-back; General Settings provides Alternate undo mode. Record and implement a chosen undo preference rather than claiming a universal FL mapping. [1](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/envsettings_general.htm)
- Ctrl+N is documented as **Save new version**, not New Project. Retaining Simplicity's existing reset/new-project action under an alleged FL map would be dangerous. Ctrl+S must never claim a successful save while saving is unavailable. [4](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_shortcuts.htm)
- F8 is Plugin Picker, not Pattern Picker. Do not bind it to an unrelated feature to fill a gap. [4](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_shortcuts.htm)
- Some live manual pages differ in F4 naming-dialog details; exact parity needs a recorded target FL version/settings and verification, not random third-party cheat sheets.
- Keep commands inactive while typing in names/search/numeric fields. Respect inline editors, dialogs, plugin keyboard ownership and future Typing Keyboard to Piano mode.
- Use a central command/keymap layer, contextual dispatch, visible menu/tooltips and conflict tests—not another expanding global key switch.

**Deliberate hybrid:** Ctrl+F for Ableton-style library search is a proposed Simplicity decision. It is not a claim that the entire application has an unchanged FL keymap.

## 5. Mixer: design channel strips, not a row of generic sliders

FL's mixer separates insert strips and a master, supports docked tracks, different layout densities, resizing, routing and per-strip effects. Its strip controls have a meaningful signal-flow role, not merely a decorative association with a playlist lane. [2](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/mixer.htm)

**Proposed replacement:**

- A cohesive, resizable bottom mixer dock with narrow, consistently aligned channel strips and a master pinned adjacent to them—not isolated across a large empty gap.
- Strip hierarchy: channel number/name and color identity → compact pan control → mute/solo → proper meter well and fader → numeric gain/readout and routing affordance.
- Tall recessed fader travel, visible scale and unity mark, a substantial thumb with highlight/bevel/grip, subtle channel separators.
- A distinct selected strip, linked-but-not-confused selections across editors, fine adjustment and numeric editing. Proposed default reset: unity gain, with its semantics tested.
- At compact widths, hide secondary details rather than crowd the primary controls. Effects can expand within the mixer or use a local popover; do not restore the rejected global inspector or evict the right-hand Patterns pane.
- Real meters only when an actual engine supplies levels. Until then, visibly inactive wells/−inf. No fake bouncing signal or working-looking routes/FX that do nothing.

A narrow-strip range around 64–80 logical pixels and a 260–340-pixel expanded dock are starting hypotheses, not FL measurements or finalized requirements. Test them at actual Windows DPI before choosing final dimensions.

## 6. Three-dimensionality: a controlled 2.5D material system

Microsoft recommends elevation, contour and purposeful shadows to communicate hierarchy, warning that overusing shadows creates visual noise. Atlassian specifically notes that dark-theme elevation also needs different surface luminance because shadows alone can disappear. [1](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/layering) [13](https://atlassian.design/foundations/elevation)

**Proposed Simplicity material rules:**

| Surface | Treatment |
|---|---|
| Playlist canvas, meter/fader wells | Recessed dark surface, restrained inner rim |
| Panel bodies | Slightly lighter charcoal with clear seams |
| Buttons, knobs, fader thumbs | Shallow neutral gradient, top highlight, darker lower edge, short contact shadow |
| Pressed controls | Reduced/reversed highlight; visually depressed, not merely a different fill |
| Menus and drag previews | Higher elevation and a softer, bounded shadow |

Use one consistent implied top-left light source; roughly one logical pixel of edge treatment, not huge bevels. Retain grayscale chrome, existing font/icons and small corners. Color remains an identity cue for patterns/tracks, not a new global accent.

Do **not** add a 3D scene, rotating UI, thick fake-metal frames, low-contrast neumorphism, wallpaper blur or shadows around every note. JUCE's native 2D painting is sufficient; no Windows-11-only material dependency is needed. Cache suitable shadows/static decoration and measure repaint cost on dense projects.

## 7. Proposed workspace

```text
┌───────────────────────────────────────────────────────────────────────┐
│ Transport · PAT / SONG · active pattern · tempo · snap · view controls │
├────────────────┬───────────────────────────────────┬──────────────────┤
│ SOUND BROWSER  │ PLAYLIST                          │ PATTERNS         │
│ Search         │ Lane headers + time grid          │ 01 Drums A       │
│ Categories     │ Reusable pattern placements       │ 02 Bass A        │
│ Places         │ Compact data-derived note views   │ 03 Chords A      │
│ Results        │                                   │ + New pattern    │
│ Preview*       ├───────────────────────────────────┤                  │
│                │ EDITOR DOCK                       │                  │
│                │ Channel Rack / Piano Roll / Mixer │                  │
└────────────────┴───────────────────────────────────┴──────────────────┘
* Only when real preview output is implemented.
```

At 1440 px, an initial hypothesis is approximately 300 px browser, 240 px patterns, 128 px lane headers, leaving 772 px of time canvas. Panel widths must be adjustable; the narrower supported window must retain useful time-canvas width. Browser, pattern pane and editor dock toggle independently. No oversized logo or generic properties inspector.

## 8. Implementation order and acceptance

1. **Pattern/note semantics and commands first:** stable sources/instances/channels, linked edits, unique copies, meaningful note data and undo transactions. Write the contract and register any justified new JUCE-independent sequencing module; do not turn the painting class into the domain model or invent speculative service layers.
2. **Right-hand picker and working editors:** active pattern, step/piano editing, draw/paint/clone/move/trim, PAT/SONG state and contextual commands. Label visual-only transport honestly until audio scheduling exists.
3. **Data-derived clip rendering:** header/body geometry, pitch/time/duration mapping, zoom, repeat/trim behavior and invalidation of all linked previews.
4. **Real browser:** background folder indexing/search with cancellation, keyboard navigation and capability-gated import/preview services. Never perform disk scanning or allocations in an audio callback.
5. **Mixer and shared depth language:** functional controls and honest capabilities, rather than adding attractive inert controls.
6. **Audio/persistence integration:** real instrument playback, sample audition, routing/meter data, save/reopen and recovery, each through explicit engine/service boundaries. Required before production-DAW claims.

### Tests that should gate implementation

- Place four instances of one pattern, edit once: all four update. Make one unique: only that branch changes afterward. Undo restores exact references/content.
- Source deletion, unused patterns, empty patterns, invalid IDs, exhausted capacity, source/instance rename distinction, and ordering without broken references.
- Step edits and Piano Roll share data; note onset/duration/pitch/velocity boundaries, overlap, crop and repeat behavior are deterministic.
- Drag painting does not stack accidental duplicates in one cell and is one undo step; invalid drops leave state unchanged.
- Native Windows preview tests verify meaningful short/long notes and chords, empty patterns, muted/selected states and zoom-dependent rendering—not just 'some pixels exist'.
- Shortcut matrix by focused view, including B/P/S conflicts, Ctrl+B versus Ctrl+D, Space versus Ctrl+Space, editor focus, popups and undo preference.
- Browser search cancellation, stale results, missing/permission-denied folders, symlink/reparse-loop policy, unsupported files and no-result states. Large libraries must not freeze the message thread.
- Mixer numeric/fine adjustment, reset/unity mapping, undo grouping, selection and truthful zero/inactive meters.
- Layout combinations, minimum size, scroll/zoom and 100/150/200% Windows DPI; clear hit targets, keyboard focus, contrast and screen-reader names.
- Retain architecture/coverage/sanitizer/native Debug+Release/installer gates and capture real Windows renders. A mockup or successful callback test is not proof that audio, pointer interaction or persistence works.

## Method and limits

Primary references were Ableton Live 12 documentation and Image-Line's live manuals for Browser/Playlist/Patterns/Channel Rack/Toolbar/Shortcuts/Mixer, with Microsoft and Atlassian design guidance for depth. The reference manuals link their own browser, playlist, channel-rack and mixer figures. Supplemental image-search results included review/retail thumbnails; these are not treated as exact current-version screenshots, measurements or reusable artwork. No Ableton/FL artwork is proposed for shipping in Simplicity.

The repository audit covered `README.md`, workflow/architecture/module contracts, `browser_panel.cpp`, `workspace_view.cpp`, `workspace.h` and the GUI guide at the current checkout. This report distinguishes sourced reference behavior from proposed Simplicity decisions. It is not hands-on testing of licensed Ableton/FL installations and does not promise exhaustive shortcut equivalence.

**Research only:** no application code, tracked project files, tests, dependencies or release artifacts were changed. No implementation PR was opened and no test run is claimed for this research deliverable.
