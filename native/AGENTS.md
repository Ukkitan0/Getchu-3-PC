# Ape Escape 3 native decompilation — contributor rules

This applies to every ChatGPT/Codex contributor working under `native/`.

## Non-negotiable project scope
- Port the **actual Ape Escape 3 USA game and original engine** to native Windows C/C++. Use original user-owned `SCUS_975.01` and original resources as evidence.
- **Never** substitute PS2/EE/IOP emulation, a replacement gameplay engine, a fabricated playable scene or unverified game behavior.
- Preserve original function addresses and source behavior in names, comments, tests and evidence logs. Label incomplete recovery precisely; do not claim a game boot merely because a unit test works.
- Original USA executable reference SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- The original ISO, ELF, game resources, Lua, original PCK bytes, copyrighted assets and private game data **must not** be committed to this repository.
- Source verification and real-resource tests must use files supplied privately by the owner. Do not check in their bytes.

## Parallel branch policy
Read `native/parallel/README.md`, `native/parallel/WORKSTREAMS.md`, and **your own** `native/parallel/workstreams/<your-team>.md` before changing code.
- Each specialist works only on `parallel/<your-team>`. Never push directly to `main` or `native/original-elf-engine-recovery-20261020`.
- Work on **owned files only**. Cross-subsystem interfaces should be proposed in your unique `native/parallel/handoffs/<your-team>.md` file.
- Do not edit `native/CMakeLists.txt`, `.github/workflows/`, shared orchestration/launch scripts, this `AGENTS.md`, or the shared `native/parallel/` coordination documents unless you are the Windows/runtime integration owner and the integrator approved the change.
- For new subsystem code: create new files prefixed/located within your area; publish a precise CMake integration recipe in your handoff. Do not silently edit another team's headers.
- Run the largest meaningful batch of original-code recovery and local tests per turn. If full build cannot run, report it as unverified.
- Keep commits to your assigned branch and record exact tests, source ranges, changed interfaces and blockers in your team's handoff.
- Do not trigger GitHub Actions. The repository's Windows workflow requires manual `workflow_dispatch`; leave it alone unless explicitly authorized.
- Prefer a PR from your branch to the original native recovery branch **only after** the integration owner coordinates it. PRs are not necessary for every partial checkpoint.
- Never force-push or delete other contributors' branches. If refs moved, inspect and reconcile before pushing.

## Coordination
Only the integration conversation should reconcile shared files, update global CMake/build scripts, run complete builds, and merge/copy specialist work into `native/original-elf-engine-recovery-20261020`. That master branch is the native game's integration baseline; GitHub's default `main` is intentionally untouched.

When finished with a pass, append to the category-specific handoff: retail source addresses/evidence, actual code delivered, tests and runtime proof, dependencies for integration, and remaining work.
