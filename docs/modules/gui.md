# GUI contract

## Responsibility
Native JUCE workspace composition: blocky dark theme, searchable track starters,
transport, scrollable arrangement/clip gestures, track controls, inspector, mixer,
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
checks state changes and resize layouts at 1080x640, 1440x900 and 1920x1080, and
writes a real PNG screenshot. The installed application has a timed --smoke-test
mode writing a JSON report; installer/portable lifecycle tests exercise it.
Internal GCC coverage includes domain/adapter/shell, not Windows-only rendering.
Manual DPI, screen-reader, dragging, wizard and desktop acceptance remain necessary.

## Extension points
Theme metrics/tokens and native components are centralized; shell commands are
separate from painting. Add real media, devices, persistence, routing and plugin
hosting as dedicated services/modules later. Do not add fake working controls or
claim this shell matches established DAWs' production audio capabilities.
