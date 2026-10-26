# Pattern workflow, asset browser and dimensional mixer — 2026-10-26

## Request / design
Implement the researched Ableton-style file discovery, FL-inspired pattern clips
and right-hand reusable patterns, core contextual shortcuts/gestures, improved
mixer and tactile depth. Preserve RetroByte/Lucide/grayscale/native Windows; no
inspector. Research: [official-source report](../research/0006-workflow-redesign.md).
Architecture and bounded first-milestone scope: [ADR 0006](../adr/0006-pattern-workflow.md).

## Implementation
- JUCE-free bounded source/note model, 96 ticks/beat, four explicit MIDI editing
  slots; shared source references versus placements, unique copies, protected
  deletion, real demo notes and data-derived/cropped/repeated previews.
- Actual asynchronous filesystem tree/filter/native folder selection, replacing
  Session/Templates. Worker-safe copied filters and joined teardown.
- Right source picker with drag placement, step toggles and octave piano duration
  draw/resize/erase; contextual keys and text-entry protection.
- Playlist draw/paint/select/delete/mute/slice/zoom, Shift linked copies, Alt free
  movement, cross-lane moves and trim; paint commits atomically.
- Resizable docks, coherent channel strips, editable dB/pan state, inactive meters,
  adjacent inactive Master, subtle gradient/edge/cap depth and gray native file rows.
- Updated module contracts, architecture ownership/tests registry, mutation test,
  user guide and explicit unavailable-services boundaries. No new module dependency.

## Local verification (before native CI)
- Strict C++20 GCC syntax check of every GUI translation unit and native test against
  pinned JUCE headers: pass, `-Wall -Wextra -Wpedantic -Werror`.
- Debug analysis build and CTest: **8/8 passed**.
- Python policy/assets/script tests: **62 passed**.
- Coverage: **283/284 lines (99.6%), 73/73 functions, 521/532 branches (97.9%)**;
  95% line / 90% branch thresholds unchanged. GUI rendering is excluded from this
  domain/adapter coverage metric and is checked by Windows native tests instead.

## Native CI / delivery
Pending Windows Debug/Release rendering, sanitizer/coverage/policy gates and real
per-user installer + portable lifecycle verification. Do not merge before all
seven checks pass. Screenshot and exact CI evidence to be added after inspection.

## Explicit limits / manual acceptance
No audio output, audition/import, hosted instruments, routing, recording or saving.
No public release: unsigned development artifacts only, with licensing unresolved.
Core keyboard subset, not exhaustive FL parity. Piano relocation/multiselection/
velocity editing and file MIDI I/O are deferred. Browser filename search is not a
recursive whole-drive index; extension filtering does not decode content.
Manual DPI/focus/accessibility, actual native chooser cancellation and folder edge
cases, precise pointer feel and installer desktop checks remain user acceptance.
