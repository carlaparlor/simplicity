# Simplicity

A **Windows-only C++20 / JUCE digital audio workstation**, now with a native dark,
minimal **GUI shell**: compact transport, searchable session/template browser,
arrangement and mixer. Grayscale controls, subtle corners and embedded RetroByte Medium keep the
workspace quiet; only tracks and clips carry colour. Reversible sketch editing
keeps the first interactions simple.

This is still a foundation: clips/levels/transport are in-memory **visual sketch
state**, not real audio/MIDI. No sound, recording, plugins or project saving yet.
See [the GUI guide and shortcuts](docs/gui-shell.md).

![Actual Windows JUCE workspace render](docs/images/workspace.png)

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
**simplicity.exe**. It opens the native workspace directly—no console or
PowerShell launcher. The demo is illustrative; audio output is not connected.

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

GUI render evidence is available in Windows test artifacts under screenshots/.
