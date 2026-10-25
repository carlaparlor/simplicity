# Dependencies and licensing

- C++20 / C11 compiler, CMake >=3.25, Python >=3.11, Git: build/test tools.
- JUCE **8.0.12**, commit `29396c22c93392d6738e021b83196283d6e4d850`:
  fetched by CMake from https://github.com/juce-framework/JUCE . Only
  `juce_audio_basics` and its transitive `juce_core` dependency are used here.
  CMake registers only these two modules with the upstream module API, avoiding
  configuration of unrelated GUI/device modules. Browser and CURL features are disabled; no GUI/device SDK is needed yet.
- gcovr and its pinned dependencies: coverage tooling only.
- GitHub Actions references are pinned to commits; upgrades are deliberate PRs.

JUCE is available under its commercial licence or AGPLv3 terms, subject to the
upstream licence at the pinned commit. Before distributing Simplicity, choose a
compatible project licensing/distribution model and satisfy JUCE obligations.
This repository intentionally does not invent the owner's licence choice or
claim commercial distribution rights. No licence for Simplicity has been selected.
Do not vendor JUCE source, binary artifacts, or credentials into this repository.
