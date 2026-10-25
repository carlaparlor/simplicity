# Architecture and enforceable boundaries

## Direction and ownership

```
app -> juce_adapter -> engine -> core
             |
       juce_audio_basics
sdk (independent, C ABI)
```

The manifest is the source of truth for direct module dependencies. No circular
edges, implicit source discovery in CMake, global include directories, or private
cross-module includes. Each module owns `include/simplicity/*.h`, `src/*.cpp`, an
explicit CMake target, tests and `docs/modules/<name>.md`. Headers have unique
include names. The checker allows only declared *direct* includes, even when
CMake has made a dependency transitively visible. New domains (timeline, media,
session, persistence, UI, devices) should become cohesive modules when needed,
not additions to a catch-all engine. Keep platform/device/UI code at the edges.

## Mechanism

`python3 tools/check_architecture.py` checks the manifest schema, directory parity,
DAG, permanent core/SDK independence, explicit first-party source list, canonical
module CMake, public-header ownership, direct include dependencies, JUCE isolation,
computed/relative include bypasses, test registration, documentation sections,
the immutable JUCE pin, required CI job/threshold contract and protection payload. CMake runs it before configuring. Unknown source
locations and module CMake changes fail closed. Use the expected CMake printed
by a failure when intentionally adding a source; never patch around the check.

CI runs it again with `--base <PR-base-SHA>` to require changed module tests,
changed docs, and a `docs/changes/*.md` record for production/policy changes.
Deleting a test/document does not count as evidence. Policy mutation tests prove
representative forbidden structures fail. Coverage and ASan/UBSan are separate
gates. All build/test jobs feed the stable `Required gates` status.

Changes to the manifest, checker, root build, CI workflow or protection payload
require changed policy tests and an ADR through the change-evidence gate.
Changes to architectural policy require an ADR, regression tests and an explicit
explanation in the PR. The manifest is a contract, not an unchangeable design.
New external dependency families require an intentional checker/pin-policy change.

## Real-time and concurrency contract

The audio thread owns the prepared processing graph. Configuration, memory
allocation, resource destruction and graph changes occur while it is stopped.
Future control-to-audio communication must use bounded messages/snapshot swaps
with explicit ownership; never share mutable UI state with audio callbacks.

Current processing functions are allocation-free loops, `noexcept`, and do not
lock, log, access devices/files or invoke a framework event loop. Gain is finite
and limited to [0, 4]. Non-finite input *samples* are not sanitized; future DSP
must state its own denormal/clipping/NaN policy. Gain changes are not smoothed.
Do not call setters concurrently with processing. Hard real-time performance,
thread-safety, scheduling, clipping and device behavior are not established by
unit tests or sanitizer success.

## Extensibility and moddability

The SDK provides a version/size-checked, C-compatible descriptor. Native extension
callbacks operate on caller-owned planar samples. C++ classes, standard library
objects, JUCE types and exceptions never cross this ABI. V1 is append-only in size;
incompatible semantics need a new ABI version. Calling convention, word size and
platform must match. Missing callbacks, undersized descriptors and incompatible
versions are rejected. A descriptor must point to a readable, sufficiently sized
object; this validation cannot make arbitrary pointers safe.

This is **not yet** dynamic loading, discovery, plugin hosting, session persistence,
a command API, scripting, a UI mod system or a security sandbox. Implement these
as separate versioned modules with compatibility/lifetime tests when requested.
Do not load untrusted native code. JUCE is kept out of the SDK to make future
extensions independent of the GUI and engine implementation.

## Limits and threat model

The checker is deliberately small, stdlib-only and lexical, not a C++ parser or a
CMake sandbox. It enforces normal literal include/target patterns; compiler flags,
preprocessor tricks, generated source and malicious policy edits are not a
security boundary. It cannot prove test quality, scalable design, semantic ABI
compatibility or real-time safety. CI executes first-party code and should never
receive secrets on untrusted PRs. Repository administrators can change policy;
server-side protection, review discipline and test evidence remain necessary.
