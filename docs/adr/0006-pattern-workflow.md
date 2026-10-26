# 0006 — Pattern-first editing, asset browser and tactile native UI

Implement the researched direction: a real filesystem sound browser on the left,
shared patterns on the right, note-derived playlist previews, step/piano editing,
contextual FL-style core shortcuts and a recessed/raised charcoal material system.
Retain native C++20/JUCE, RetroByte/Lucide and Windows-only packaging. No inspector.

For this first editing milestone, extend the bounded, JUCE-free shell with pattern
sources and stable clip references. Notes use integer ticks (96/beat), explicit
channel/pitch/velocity/duration. The four named channel slots are MIDI editing
identities, not hosted instruments. Playlist lane levels remain visual state, not
an audio graph; do not imply real routing or audition. Audio/persistence integration
remains a separate milestone. No speculative framework or new module edge.

Shared edits update all placements; Make unique clones and rebinds atomically.
Deleting an in-use pattern fails. A paint gesture is one undo transaction. All
invalid/capacity failures leave state/history intact. Sources/IDs live inside undo
state; selection/visual transport remain message-thread only.

Filesystem enumeration uses JUCE DirectoryContentsList on a TimeSliceThread;
worker lifetime ends before list/filter destruction. Filtering supports current
folder results, not a recursive whole-drive index. User folders are chosen with a
native chooser; no sample audition/import/plugin hosting is fabricated. GUI
callbacks own/borrow according to module contracts and never run in audio callbacks.

Core FL mappings are contextual and documented, not an exhaustive parity claim.
Undo is intentionally sequential Ctrl Z / Ctrl Y in this milestone. Unsupported
record/save/plugin commands never masquerade as successful operations.
