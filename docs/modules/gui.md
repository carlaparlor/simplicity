# GUI contract

## Responsibility
Native JUCE workspace composition: neutral dark theme, compact transport,
scrollable arrangement/clip gestures, inline track controls, searchable session/
template browser, mixer,
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
checks track rename/mute/solo/level bindings, browser filtering/templates/navigation/
limits/undo, vector and app icons, neutral palette, and reclaimed resize layouts at 1080x640, 1440x900 and 1920x1080, and
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

BrowserPanel owns tab/search/scroll components and borrows the composition root
Workspace. Its optional changed callback runs synchronously after a successful
selection/addition; the owner refreshes views. Invalid rows/cap additions return
false without mutation. Row buttons are preallocated to avoid deleting a live
callback during synchronous refresh. Components/look-and-feel are message-thread
only; callers own their lifetime. No inspector remains.

Native components are centralized; shell commands are
separate from painting. Add real media, devices, persistence, routing and plugin
hosting as dedicated services/modules later. Do not add fake working controls or
claim this shell matches established DAWs' production audio capabilities.
