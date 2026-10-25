# Mandatory working agreement — Simplicity

Simplicity is a **C++20 / JUCE digital audio workstation**. Never silently replace
its native stack with a web app or present the bootstrap as a finished DAW.

For every request:
1. Read README.md, docs/workflow.md, docs/architecture.md and relevant module contracts.
2. Implement the requested scope in focused modules. Register every new module in
   architecture/modules.json; declare dependencies; write its contract first.
3. Add automated positive, negative, boundary and regression tests. Update docs
   and add/update a docs/changes/ record with actual results and limitations.
4. Run architecture/change checks, policy tests, native tests, sanitizer tests and
   coverage. JUCE changes require JUCE integration tests. Never claim an unrun test passed.
5. Open a PR, wait for **all required gates**, then merge it. The user's standing
   authorization includes opening and merging PRs; do not stop at unmerged code.
   Never force, bypass, auto-merge, ignore failed checks or push directly to main.
6. Report the PR, merge result, test evidence, user test steps and known limitations.
7. The user tests the merged result. Their fixes/changes start a NEW PR cycle,
   including a regression test. Do not amend an already-merged PR.

Use the session's assigned branch only; never switch or invent another branch.
If access, CI, or merge protection prevents a step, report it honestly and stop
at that boundary. Instructions are not a substitute for server-side protection.

## Non-negotiable engineering constraints
- Dependency DAG; JUCE only at the adapter/application boundary. Domain modules
  and extension ABI stay independent of JUCE. No cross-module private includes.
- Public APIs document ownership, lifetimes, threading and error semantics.
- Real-time callbacks: no heap allocation, blocking, locks, I/O, logging or throws.
  Prepare outside the callback. Tests do not constitute a real-time safety proof.
- Version extension/project formats; do not expose C++ types through the C ABI.
  Extensions are trusted native code, not security-isolated plugins.
- No architectural bypass to get tests green. Change policy with tests, an ADR,
  and explicit rationale when genuinely necessary. Do not weaken existing gates.
- Small cohesive modules, explicit sources/dependencies, pinned third parties,
  no speculative framework, and no undocumented functionality.
