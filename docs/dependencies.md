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
