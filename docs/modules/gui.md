# GUI contract

## Responsibility
Native JUCE workspace composition: neutral dark theme, compact transport,
scrollable arrangement/clip gestures, inline track controls, asynchronous filesystem discovery, right-hand reusable
patterns, step/piano editor and tactile mixer,
help overlay and keyboard shortcuts. Owns a shell Workspace, never audio state.
The default is an explicitly illustrative demo, not fabricated recording/audio.
Unimplemented recording is disabled; saving explains its absence. Panels toggle;
clip movement snaps. No browser/web UI, network or WebView2 dependency.

## Dependencies
Direct shell state and the pinned simplicity::juce_gui boundary. Upstream JUCE UI
modules compile once in cmake/JuceGui.cmake with private module sources and public
headers/settings. The GUI does not include engine/core/SDK private interfaces.

## Threading
All components, timers, commands and snapshots run on JUCE's message thread.
The 30 Hz timer animates the playhead only; meters never fabricate activity.
Look-and-feel lifetime covers all child components. No audio callback exists here.

## Tests
`tests/gui_test.cpp` renders actual JUCE controls/windows, invokes control callbacks,
checks track rename/mute/solo/level/pan bindings, temporary-folder filtering and
navigation, pattern/unique/step/piano duration bindings, source-derived previews,
limits/undo, vector and app icons, neutral palette, and dock resize layouts at 1080x640, 1440x900 and 1920x1080, and
writes a real PNG screenshot. The installed application has a timed --smoke-test
mode writing a JSON report; installer/portable lifecycle tests exercise it.
Internal GCC coverage includes domain/adapter/shell, not Windows-only rendering.
Manual DPI, screen-reader, dragging, wizard and desktop acceptance remain necessary.

## Extension points
ui_style owns charcoal theme metrics and painting; ui_assets owns the embedded
RetroByte Medium face and official Lucide SVGs. A message-thread shared cache is
held by Look, with a non-owning weak registry. The native SVG parser supplies
immutable stroked paths in 24x24 SVG coordinates; temporary Drawable components
are destroyed immediately, not retained past JUCE shutdown. Native default text uses the embedded face; explicit OS
fallback faces remain available for missing glyphs. Invalid/empty icon IDs draw
nothing. No synthetic bold or installed font is required. Asset/license details
and unresolved font rights are in docs/dependencies.md.

BrowserPanel owns its native tree/search/category/chooser and DirectoryContentsList,
not Workspace. Filter query/category are owned copies under CriticalSection; worker
callbacks never read UI components. The TimeSliceThread is joined before member
teardown; async chooser callbacks use SafePointer. PatternPanel/PatternEditor borrow
Workspace and invoke their synchronous changed/edit callbacks after mutation. Sources
are not instruments; lane level/pan is not an audio graph. All component/look-and-feel
lifetime and painting remains message-thread-only; no inspector.

Native components are centralized; shell commands are
separate from painting. Add real media, devices, persistence, routing and plugin
hosting as dedicated services/modules later. Do not add fake working controls or
claim this shell matches established DAWs' production audio capabilities.

## Pattern-first revision
BrowserPanel now owns a background directory list and native filesystem tree;
category/query filtering operates on files in expanded folders. Explicit native
folder choice, no invented library content or fake audition/import. PatternPanel
owns reusable source selection/actions on the right. PatternEditor renders/edits
actual shell notes in step/piano modes in the bottom dock. All callbacks run on
the message thread, borrow Workspace, and refresh only after operations finish.
Painter geometry is not musical state. Contextual FL-style core commands replace
B/M/Ctrl D workspace-global bindings; text editors retain typing ownership.

## Compact clip grid and navigation revision
Pattern cards use the same clip renderer as the playlist, two columns per row in
an independently scrolling viewport. Each card retains its source identity,
keyboard button semantics, selection, double-click edit and drag placement.
Mixer source colours tint the whole corresponding strip, not a header stripe.
Track-name and M/S rectangles must have a positive gap at every supported lane height.
Shared native horizontal range bars support scroll-handle edge zoom and centre
panning; a vertical drag grip controls playlist lane / piano key height. These
controls, Ctrl-wheel zoom, middle-button panning and zoom presets mutate only
view state; they never add undo entries or alter notes/placements. Editor drawing,
hit tests, scrolling and gestures must share the same zoom transform. All callbacks
are synchronous on the message thread; owners outlive their controls.

## Traditional menus and centred transport
A native JUCE MenuBarComponent exposes File/Edit/View/Help above the existing
transport row, replacing the top-left folder button. Its MenuBarModel lives in
the composition root and outlives the bar; detach the model during teardown.
Menu state reflects current availability; cancel/unknown/wrong-menu and disabled
commands must not mutate state. Saving/recording remain unavailable. Stop/play/
record form a 112 px group centred on the entire workspace at every supported
width; mode/tempo sit immediately left, loop/position/snap immediately right. Preserve
the previously restored playlist row, source grid, mixer and mouse behavior.
