# Dependencies and licensing

- Windows developers: Visual Studio 2022 C++ tools/Windows SDK, CMake >=3.25,
  Python >=3.11 and Git. setup.cmd discovers/installs missing tools via winget.
- Inno Setup **6.4.3**: packaging tool only, pinned download + SHA-256 in CI.
  It is not shipped to or required by end users. Tool installation on a hosted
  runner is separate from our own non-elevating, per-user installer.
- End users: Windows 10/11 x64; no developer tools or extra MSVC redistributable.
  Windows' built-in system libraries are still required.
- JUCE **8.0.12**, commit `29396c22c93392d6738e021b83196283d6e4d850`:
  fetched by CMake from https://github.com/juce-framework/JUCE . The DSP adapter
  uses `juce_audio_basics`/`juce_core`; Windows UI additionally uses
  `juce_gui_basics`, `juce_graphics`, `juce_events` and `juce_data_structures`.
  CMake registers only required modules with the upstream module API, avoiding
  unrelated device modules and the juceaide build helper. Windows uses the system
  SDK/DirectWrite for native UI; no browser/WebView2 or runtime framework is added. Browser and CURL features are disabled; no GUI/device SDK is needed yet.
- gcovr and its pinned dependencies: coverage tooling only.
- GitHub Actions references are pinned to commits; upgrades are deliberate PRs.

JUCE is available under its commercial licence or AGPLv3 terms, subject to the
upstream licence at the pinned commit. The owner selected the commercial / decide-later route on 2026-10-25.
Installers may be prepared/tested but public releases remain on hold until the
owner confirms suitable JUCE rights and Simplicity distribution terms.
Before distributing Simplicity, choose a
compatible project licensing/distribution model and satisfy JUCE obligations.
This repository intentionally does not invent the owner's licence choice or
claim commercial distribution rights. No licence for Simplicity has been selected.
Do not vendor JUCE source, binary artifacts, or credentials into this repository.

## GUI artwork / font (development revision)

- Lucide **1.61.0**, commit `c29c276f16590cf017126671535ad8c792be64d8`:
  https://github.com/lucide-icons/lucide . Used SVGs are vendored verbatim under
  modules/gui/assets/lucide, with upstream ISC + applicable Feather/MIT notices.
  Generated C++ data preserves the original geometry/stroke; JUCE renders SVGs
  natively. No JavaScript/Node/browser runtime. Packaged LUCIDE-LICENSE.txt retains
  the required notices; manifest.json pins every source SVG SHA256.
- **RetroByte Medium 001.000**, HipFonts / IgorOvsyannykov, 14,212-byte TTF,
  SHA256 `3eda5a11a35f17082e3b5aaf1cbda99b1aa47b3d5680c0cd8a6134db7120e537`.
  Requested page: https://www.1001fonts.com/retrobyte-font.html . Font and exact
  lowercase-s outline are embedded in development builds; no system font install.
  The free download is **personal-use only**. Commercial use, embedding and
  redistribution rights for Simplicity and its icon are **unresolved**. On
  2026-10-25 the owner expressly requested use now and negotiation later. That
  instruction does not grant rights from the author. Keep public/commercial
  release blocked pending appropriate written permission; development labeling
  alone does not resolve the license. Source/provenance/terms are documented in
  modules/gui/assets/retrobyte/NOTICE.txt and shipped as RETROBYTE-NOTICE.txt.
  The font bytes/name/glyph have not been changed. Do not claim it is open source,
  freely redistributable, commercially cleared, or covered by Lucide's license.

Regenerate embedded source assets with `python tools/assets/make_ui_assets.py` and
font-derived icons with `python tools/assets/make_app_icon.py`. Both use only the
standard library; exact source bytes/pixels and notice packaging are tested.
