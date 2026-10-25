# ADR 0003: Native, honest workspace shell with isolated sketch state

- Status: accepted
- Date: 2026-10-25

## Context
The owner wants a blocky, minimal dark GUI, intuitive to begin using and capable
of growing into a competitive DAW. This is the first GUI, not a request to fake
recording, plugin hosting or parity with mature production workstations.

## Decision
Use native JUCE gui_basics, compiled once behind a static UI boundary, without
juceaide/browser/WebView2. Keep shell state in an independent C++ module and render
it through a dedicated GUI module. Existing engine/adapter/SDK boundaries stay
unchanged. Windows builds now launch a real GUI subsystem executable directly;
remove the console pause launcher from installer/portable delivery.

Use charcoal square surfaces, thin dividers, muted track colour identities and
one lime accent. Keep familiar DAW geography: browser left, arrangement center,
inspector right, mixer below, transport always visible. Provide explicit text,
focus outlines, tooltips, reversible sketch edits and keyboard shortcuts. Start
with a labelled illustrative demo; New produces a guided empty workspace.

Transport, levels and clips are visual/in-memory sketch state only. Disable
recording; explain missing save/device functionality. No animated fake meters.
All state edits are message-thread-only, capacity-bounded and undoable. Future
media/session/audio services must be separate modules, not coupled to painting.

## Test and coverage scope
Windows gates exercise actual GUI control callbacks, rendering, resizing, native
launch, installer/portable smoke reports and generated screenshots. Core, engine,
SDK, adapter **and new shell state** retain the 95% line/90% branch GCC gates and
ASan/UBSan tests. Windows-only rendering is not compiled by Linux analysis, so it
is explicitly outside those coverage numbers, not falsely counted as covered.
No existing coverage threshold is lowered; rendering behavior has its own required
Windows tests. Manual drag/DPI/keyboard/accessibility/desktop checks remain needed.

## Consequences
This is a solid visual/interaction foundation, not playable music software yet.
No sound, recording, real MIDI/clip contents, routing, plugin loading, disk saving
or stable project format. Commercial licensing/signing/public releases remain
pending; unsigned development artifacts retain prior caveats. No new platforms.
