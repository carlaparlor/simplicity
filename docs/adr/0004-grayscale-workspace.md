# ADR 0004 — Grayscale GUI refinement without new dependency edges

Status: accepted, 2026-10-25.

The owner requested less blocky grayscale controls, no inspector/branding row,
a better browser/font/icons and a Windows application icon. Keep the existing
GUI -> shell boundary; extract shared style and BrowserPanel within GUI rather
than introduce a theme framework or speculative media/plugin catalogue. Track
colours alone retain colour. Use Windows' Segoe UI and in-house vector glyphs;
a deterministic stdlib generator supplies matching multi-resolution executable
and runtime window icons, with byte-level and actual packaged-EXE tests.

The inspector is removed, not merely hidden. Inline names, track M/S and native
mixer dB fields retain its usable edits. A compact 62 px toolbar and right-edge
arrangement reclaim space. Browser navigation reflects actual sketch state;
templates create visual blocks, not pretend sound sources. No audio/persistence
capability or release-policy change is implied.

Preserve all existing quality thresholds and Windows gates. Expand native
regressions for palette/icons/space/browser/inline bindings, plus a mutation test
for explicit registration of newly extracted GUI sources. Temporary, size-bounded
Windows screenshot annotations may be used to retrieve and visually inspect the
actual render when artifact storage is inaccessible from the coding environment;
remove those diagnostic steps after retrieval. Normal artifacts remain canonical.
No architecture enforcement rule or module dependency is relaxed.
