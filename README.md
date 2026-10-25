# Simplicity

A native **C++20 / JUCE digital audio workstation**, being built from a tested,
modular foundation. This repository currently contains an audio configuration
contract, gain-processing engine, C extension ABI descriptor, JUCE buffer adapter,
and a **headless bootstrap**. There is no editor, audio-device playback, recording,
sequencer, project format, plugin loader or complete DAW yet.

## Build and test

Requires CMake 3.25+, a C++20 compiler, Python 3.11+, Git, and internet access on
the first JUCE build. JUCE is fetched at an immutable 8.0.12 commit. Only headless
modules are built, so Linux GUI/audio-device development libraries are not needed.

```sh
python3 tools/check_architecture.py
python3 -m unittest discover -s tests -p 'test_*.py' -v
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
# Linux/macOS; on Windows use build/app/Debug/simplicity.exe
./build/app/simplicity
```

Offline/core-only after installing toolchains:
`cmake -S . -B build-core -DSIMPLICITY_WITH_JUCE=OFF` (not a substitute for JUCE CI).
See [testing](docs/testing.md) for release, sanitizers and coverage commands.

## Engineering contract

- [Architecture and enforcement](docs/architecture.md)
- [Request → implement → PR → merge → user test workflow](docs/workflow.md)
- [Testing and evidence](docs/testing.md)
- [Dependencies and licensing](docs/dependencies.md)
- [Foundation decision](docs/adr/0001-native-modular-foundation.md)
- [Contributor/agent rules](AGENTS.md)
- [Initial implementation and results](docs/changes/0001-foundation.md)

`architecture/modules.json` is the dependency contract. Configuration and CI reject
undeclared dependencies, cycles, noncanonical module CMake, missing contracts,
and unowned sources. CI also demands test/documentation evidence for code changes.
Branch protection must require `Required gates`; see the workflow document for
server setup and the current permission limitation. These checks enforce structural
rules, not semantic correctness or protection against a malicious maintainer.
