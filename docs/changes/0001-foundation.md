# 0001 — Native foundation and enforceable engineering workflow

Date: 2026-10-25

## Request
Create the mechanisms that keep Simplicity a rigorously tested/documented,
modular, scalable, extensible and moddable C++/JUCE DAW. Establish the standing
implement → PR → merge → user tests → fixes workflow.

## Implementation
- Explicit module DAG, configure-time/CI architecture enforcement, mutation tests.
- Independent core, DSP engine and C ABI SDK; actual pinned JUCE adapter/bootstrap.
- Test/documentation evidence gate; Debug/Release native OS matrix, sanitizers,
  coverage thresholds, artifact logs and aggregate required status.
- Agent instructions, module contracts, ADR, PR/bug templates and protection payload.

## Verification
Local environment: Debian Linux, GCC 12.2, CMake 4.4.4, Python 3.11.2.

- Architecture gate: passed, including 32 Python mutation/change-evidence tests.
- JUCE-enabled Debug: 7/7 CTest suites passed.
- JUCE-enabled Release: 7/7 CTest suites passed (CHECK remains active).
- JUCE-enabled ASan + UBSan with leak detection: 7/7 suites passed; no reports.
- GCC/gcovr 8.4 coverage: **100% lines (31/31), 98.1% branches (53/54),
  100% functions (8/8)** over all first-party module code.
- Initial configure exposed unnecessary upstream GUI package probes; fixed by
  registering only juce_core and juce_audio_basics using JUCE's module API.
- Branch-protection GET and PUT: both returned 403; no setting application claimed.

The PR runs the same gates plus Linux/macOS/Windows Debug/Release builds. CI
results are recorded on the PR rather than guessed here. Human acceptance is pending.

## Known limitations
This is a headless foundation, not yet a usable DAW. No UI, playback device,
recording, timeline, project storage or dynamic plugin/mod loading. Test passing
cannot prove real-time safety. Branch protection API access is denied (403) for
the current integration; the server-side protection payload needs owner setup.

## User verification
Use the README commands with JUCE enabled; CTest succeeds and `simplicity` prints
its readiness line and exits 0. Report issues or request the next feature for a
new tested PR cycle.
