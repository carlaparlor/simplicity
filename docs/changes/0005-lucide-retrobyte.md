# Lucide / RetroByte / dark charcoal — 2026-10-25

## Request
User rejected the prior style and requested Lucide icons, the linked RetroByte
font, its lowercase s as the app icon, and dark charcoal rather than medium gray.
After being told the free font is personal-use only, the user explicitly requested
integration now and said they would negotiate licensing later. Rights remain
unresolved; no commercial/public release is authorized or claimed.

## Implementation
- Exact Lucide 1.61.0 SVGs at pinned upstream commit, native JUCE SVG rendering;
  no manually approximated symbols, browser or Node runtime. Notices packaged.
- Exact RetroByte Medium TTF embedded for normal text and native child controls;
  no font install or synthetic bold. Source bytes/hash/provenance are recorded.
- Actual s contours read from the TTF, not the old invented S silhouette; same
  7-size executable ICO, runtime title PNG and native peer icon plumbing.
  Icon background #181818; workspace #121212 / #191919 / #242424 charcoal.
- Cohesive GUI asset provider/cache split from painting, explicit source list;
  no new module edges, no domain/audio changes.
- Source hashes, generated-data parity, TTF polygon/missing-glyph tests, native
  font/whole-icon-mask checks, SVG tint reset and packaged-notice/icon regressions.

## Validation
- Local architecture/change checks and 55 Python policy/asset tests passed.
- Local analysis CTest 7/7; separate ASan/UBSan/leak CTest 7/7 passed.
- Strict C++20 GCC GUI/app/test syntax against pinned JUCE passed (-Werror).
- Compiled domain/adapter/shell coverage 100% lines (150/150), 97.3% branches
  (215/221), 100% functions (45/45). Windows-only rendering is excluded.
- Windows GUI/runtime/installer gates pending; no merge until all pass.
The real Windows render must be refreshed/visually inspected, not a mockup.
Source review caught JUCE drawWithin fitting ink bounds instead of the SVG
viewBox; explicit 24x24 mapping plus native padding/stroke assertions preserve
Lucide's intended geometry rather than inflating each symbol independently.
First Windows Debug/Release native suites passed (including the actual TTF/icon
mask). Render review prompted 14 px text / 28 px clock sizing: RetroByte's 64-unit
pixel grid and 896-unit ascent/descent align at multiples of 14, avoiding tiny
fractional strokes. Existing control bounds remain; the clock's label padding is
removed so its 28 px digits fit without horizontal compression.
FontTools 4.60.1 independently confirmed every one of the 46 TTF s outline points;
the shipped generators/tests use stdlib only, not FontTools.

## Acceptance / limitations
Check the actual RetroByte face in title, browser, labels, menus and text edits;
Lucide symbols in transport/browser/window controls; charcoal s in Explorer,
taskbar/Alt-Tab and title bar. Test minimum size, DPI/readability, text entry and
accessibility manually. Still in-memory sketches, no audio/recording/saving.
Font license, commercial JUCE terms, signing and public release remain pending.
