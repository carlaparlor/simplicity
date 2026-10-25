# ADR 0001: Native modular foundation with executable governance

- Status: accepted for the initial foundation
- Date: 2026-10-25

## Context

Simplicity is a C++/JUCE DAW. An empty repository needs a structure which can grow
without coupling its DSP, UI and extension ecosystem. The owner explicitly
requires rigorous testing/documentation and request → implement → PR → merge →
user testing → fixes cycles, not unmerged suggestions.

## Decision

Use C++20/CMake with explicit targets and a versioned module DAG. Keep core types
and DSP independent of JUCE; adapt at the edge. Establish a small C ABI descriptor
for future trusted native mods, not a speculative plugin framework. Start with
actual tested signal processing and a headless JUCE executable, not a fake UI.

Enforce the structural contract at configure time and in CI. Require corresponding
test/documentation change evidence, native multi-platform tests, ASan/UBSan and
95% line / 90% branch coverage. Aggregate required jobs behind a stable gate.
Document the human workflow in AGENTS.md, a PR template and a workflow contract.
Use server-side branch protection to prohibit merging red or bypassed work.

## Alternatives and consequences

A single JUCE application target is simpler initially but makes engine isolation
and future SDK independence difficult. A plugin framework/dynamic loader now would
invent unrequested functionality and security/lifetime obligations. A full C++ AST
linter is stronger than lexical checks but adds substantial tooling; explicit
canonical CMake and includes are adequate for this small initial codebase.

Canonical module CMake is intentionally restrictive. New dependency families or
source forms require a policy change with tests/ADR. Gates do not prove semantic
quality or real-time safety. Branch protection requires repository administration
rights not granted to the current integration; this limitation is recorded rather
than treating a checked-in payload as an applied server setting.
