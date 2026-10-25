# ADR 0005 — Exact Lucide assets and embedded RetroByte development face

Accepted 2026-10-25; licensing remains unresolved.

The owner rejected the prior custom symbols/Segoe styling, explicitly selected
Lucide and RetroByte, requested the RetroByte s icon on dark charcoal, and asked
to integrate now while they negotiate licensing later. Use exact, pinned assets,
not another hand-drawn approximation or substituted font. Lucide ISC/Feather MIT
notices are retained. RetroByte's personal-use restriction remains explicitly
unresolved; no commercial/public release permission is inferred or granted.

Keep GUI -> shell boundaries unchanged. ui_assets owns a message-thread shared
native SVG-stroke-path/typeface cache, held for the Look lifetime; a weak registry prevents process-static ownership; native SVG Drawables are
converted to immutable stroked paths then immediately destroyed. Assets compile into C++ data:
no OS font installation, runtime network or Node dependency. No synthetic bold:
the provided typeface is Medium. Font-derived icon generation reads the pinned
TTF s polygons directly and fails on unsupported shapes instead of approximating.
Resource, custom-title and native peer icons continue to share the same PNG art.

Add source/hash/notice tests, native font/icon-mask/tint regressions and an engine
boundary mutation. All prior CI jobs/thresholds remain. Temporary annotations can
retrieve a real Windows render for inspection, then are removed (ADR 0004).
Desktop readability/DPI/accessibility and actual licensing negotiation remain
human acceptance tasks; tests and development artifacts do not clear rights.

CI diagnosis revealed Git 2.55 background writers racing temporary fixture cleanup
(`.git`/`objects` nonempty), not a domain or sanitizer defect. Throwaway policy
repos explicitly disable automatic maintenance, GC detachment and fsmonitor before
the first commit; a regression test verifies those settings. Cleanup errors are
still fatal, never retried/ignored. Real policy assertions remain unchanged.
Read-only Actions permission and temporary native-render/log retrieval steps were
removed after diagnosis. Bounded, escaped local failure annotations remain so
future CI failures can be investigated without external Azure log access.
