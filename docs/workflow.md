# Standing development workflow

The user has authorized this repeating cycle:

1. **Request** — establish scope and acceptance criteria; only ask when blocked.
2. **Implement** — code, regression/boundary tests, contracts, change record.
3. **Validate** — architecture + change-evidence checks, policy tests, native debug
   and release builds on Windows x64, JUCE integration, installer/portable lifecycle
   tests, internal sanitizers and coverage. Fix failures.
4. **PR** — push the session branch and open a PR against main. Include the request,
   architecture impact, test commands/results, user test steps and limitations.
5. **Merge** — wait for every required CI check to succeed; merge that explicit PR
   using `gh pr merge <number> --merge` (or an explicitly chosen squash/rebase).
   Never use `--auto`, bypass checks, force-push, or directly update main.
6. **User tests** — report merged PR and a reproducible verification checklist.
   Human acceptance is deliberately *after* merge and is not fabricated in CI.
7. **Fixes/changes** — reproduce the feedback, add a failing regression, implement,
   validate, open/merge another PR, then return to user testing.

Follow the session's fixed branch. After a merge, fetch origin and fast-forward
that same branch to origin/main if necessary and possible (`git merge --ff-only
origin/main`); never reset away work or switch branches. With merge commits the
next PR contains only subsequent work. Do not reuse a merged PR for fixes.
No commit/push is necessary just to save work, but a PR necessarily needs a push.

## Required repository settings (server-side enforcement)

On `main`, require a pull request, require the exact check **Required gates** with
strict/up-to-date status, enforce administrators, block force pushes and deletion,
and restrict bypass rights. Zero pre-merge human approvals is intentional: the
standing workflow puts the user's acceptance testing after merge. Do not require
a manual review and then silently bypass it to achieve automatic merge.

The checked-in `.github/branch-protection.json` is the intended REST API payload:

```sh
gh api --method PUT repos/carlaparlor/simplicity/branches/main/protection \
  --input .github/branch-protection.json
```

Repository settings are not controlled by committed files. The current Arena
GitHub integration returns **403 Resource not accessible by integration** for the
branch-protection API. Therefore protection is **not verified/enabled by this
implementation**. An owner must apply the payload through an integration with
branch-administration permission or the GitHub settings UI. Do not request or
paste credentials. CI + configure-time enforcement is implemented, but preventing
direct merges/pushes requires that server-side step. Do not call this fully locked
down until its settings have been verified.

## Failure handling

A red or absent check means no merge. A permission/network failure is reported,
not bypassed. Manual UI/audio testing remains pending until the user reports it.
Coverage is evidence, not permission to delete assertions. An architectural rule
change requires an ADR, policy tests and documentation in the same PR.

## Windows setup and delivery contract

Windows 10/11 x64 is the only supported product target (ADR 0002). No manual JUCE,
compiler or runtime setup for end users. The developer entry point is setup.cmd.
PRs must pass the actual Windows setup command, Debug/Release tests and a packaging
job that installs, reinstalls, launches and uninstalls the generated installer and
checks portable execution. Installer/portable artifacts appear on successful main
runs; publishing a public release is deliberately not automated until the owner's
commercial JUCE licensing/distribution decision is confirmed. Signing is also
pending; do not promise a frictionless SmartScreen experience without it.
