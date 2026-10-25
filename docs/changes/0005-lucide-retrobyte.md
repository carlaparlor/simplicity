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
- Local architecture/change checks and 61 Python policy/asset/diagnostic tests passed.
- Local analysis CTest 7/7; separate ASan/UBSan/leak CTest 7/7 passed.
- Strict C++20 GCC GUI/app/test syntax against pinned JUCE passed (-Werror).
- Compiled domain/adapter/shell coverage 100% lines (150/150), 97.3% branches
  (215/221), 100% functions (45/45). Windows-only rendering is excluded.
- Windows Debug/Release native suites passed 9/9 at 59baf13 (run 40241047049).
- Actual Windows render refreshed and inspected: `docs/images/workspace.png`,
  1440x900, 42,809 bytes, SHA256
  `ef53bb82077a529617f995369eb6167d7ab9e45e2d2f6c2a897c9d002ee86b13`.
  Derived from Release check 121312920033, source 59baf13; no production GUI
  changes followed this capture. This is a native render, not a mockup.
- Windows installer/portable check 121313707477 passed at 59baf13: supported
  Release setup/build/CTest, SHA-verified Inno Setup compiler, packaging and real
  install/upgrade/uninstall/portable/icon/notices lifecycle tests. Final-head
  required checks must also pass before merge; authoritative results and the
  development artifact are linked in PR #5.
Source review caught JUCE drawWithin fitting ink bounds instead of the SVG
viewBox; explicit 24x24 mapping plus native padding/stroke assertions preserve
Lucide's intended geometry rather than inflating each symbol independently.
The provider caches JUCE's parsed stroked outline paths in SVG coordinates,
avoiding Drawable component layout/origins entirely; tint is applied at paint
rather than mutated. A subsequent new native assertion initially read the target
while its Graphics context was still open (Direct2D batches until EndDraw), giving
an empty readback. The fixture now closes Graphics before sampling, like the
existing glyph loop. Tint tests require nonzero opaque pixels so this cannot
silently become a vacuous pass. Strict center/padding/stroke thresholds remain;
no production rendering assertion has been removed or weakened.
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

### CI diagnostic follow-up — resolved fixture race
The exact original coverage, ASan and Python-policy failures all identify
`test_removed_test_not_evidence` cleanup, not a failed policy assertion or memory
sanitizer finding: Git 2.55 on Linux was still writing inside the throwaway `.git`
(`Directory not empty: .git` / `objects`) as TemporaryDirectory removed it.
The fixture now disables automatic maintenance/GC detachment/fsmonitor before its
first commit. A regression verifies every setting; 180 fresh fixture lifecycles
(30 repetitions of all 6 change-evidence cases) passed locally. Cleanup errors
remain fatal; no retries/ignore-errors, weakened tests or gate bypasses.
Native startup stress (30 repeats per binary) passed locally and in CI, providing
additional evidence that the failure was the Python fixture, not native memory.
Five failure-annotation regressions cover selection, command escaping, partial
crash output, bounds and CLI directory/file/missing-log behavior. Python output retains bash pipefail while being tee'd.
Temporary render/log retrieval, extended Actions permissions and diagnostic stress
steps were removed. Normal required CTest/coverage/sanitizer gates remain intact.
