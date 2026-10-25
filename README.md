# Simplicity

A **Windows-only C++20 / JUCE digital audio workstation**, being built from a tested,
modular foundation. This repository currently contains an audio configuration
contract, gain-processing engine, C extension ABI descriptor, JUCE buffer adapter,
and a **headless bootstrap**. There is no editor, audio-device playback, recording,
sequencer, project format, plugin loader or complete DAW yet.

## Windows only — easiest way to test

**Supported target: Windows 10/11 x64.** Linux/macOS and native ARM64 are not
supported product targets. No compiler, Python, JUCE setup or separate Visual C++
runtime is needed to run a prepared Windows build.

1. Open a **successful** [Quality run on main](https://github.com/carlaparlor/simplicity/actions/workflows/quality.yml?query=branch%3Amain).
2. Download **Simplicity-Windows-x64-development** from its Artifacts section
   (GitHub sign-in required; artifacts are retained for 14 days).
3. Extract that download and run **Simplicity-Setup.exe**. It installs for your
   user, without an admin prompt, and adds a Start menu shortcut.

Prefer no installation? Extract **Simplicity-Portable.zip** and double-click
**Launch Simplicity.cmd**. The current preview prints a readiness message and
keeps the console visible; there is deliberately no DAW window or playback yet.

These are **unsigned development test artifacts**, not a public commercial
release. SmartScreen can warn/block them; never disable security protections.
Commercial licensing/distribution terms and code signing await owner confirmation.
No public releases are automatically published. Checksums are included.
See [Windows installation, troubleshooting and removal](docs/windows-setup.md).

## Developing: one command

Download/extract this source repository (or clone it), then double-click
`setup.cmd`, or run it from a terminal:

```bat
setup.cmd
```

It finds/installs missing developer tools using winget, fetches pinned JUCE,
configures Windows x64, builds Release and runs the tests. Tool installation may
require internet, several GB, UAC consent and a restart; running Simplicity does
not. Already have the tools? `setup.cmd -NoInstall`. Debug:
`setup.cmd -Configuration Debug`. Failed commands stop with a nonzero exit code.
The script never changes your system execution policy.

[Windows setup](docs/windows-setup.md) lists prerequisites, manual presets and
packaging commands. [Testing](docs/testing.md) documents all quality gates. Internal
Linux analysis jobs exist solely for ASan/UBSan/coverage, not Linux distribution.

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
