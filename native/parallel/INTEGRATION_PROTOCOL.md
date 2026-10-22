# Native original-engine integration protocol

**Master/integration ref:** `native/original-elf-engine-recovery-20261020`.
**Specialist refs:** all `parallel/*` branches listed in README.
**Default `main`:** leave unchanged. No force pushes. Do not trigger GitHub Actions.

## Workflow

1. The specialist pulls/reads its own branch and `native/parallel/workstreams/<category>.md`, and works exclusively in the files owned by that category.
2. It commits **substantial tested native original-game recovery** to `parallel/<category>`. Each commit should describe recovered original ELF VAs, source evidence, and tests.
3. It updates only `native/parallel/handoffs/<category>.md` with: new commit SHAs, tested/unverified status, interfaces required, CMake file additions/changes, original retail inputs and blocker details.
4. The integration owner reviews one specialist handoff/commit set at a time, checks whether files overlap other merged work, verifies original-game provenance, then cherry-picks or merges **selectively** to the master native branch.
5. **Only the integration owner** updates `native/CMakeLists.txt`, shared build scripts, app entry point, cross-subsystem API wiring, and branch-level coordination docs.
6. The integration owner runs configured CMake, CTest and a real Windows executable boot attempt when possible. Record exact evidence; never call isolated unit tests a playable game.
7. Specialist branches do not automatically update when master advances. Periodically the integrator communicates the new master commit to specialists; each specialist may update its own branch after resolving conflicts, without force-pushing.

## Ownership and conflicts

- A source file has exactly one subsystem owner in WORKSTREAMS. The same data type can be **read** from many categories, but modifying its implementation must be coordinated.
- New files carry the owning category's prefix or live in its subsystem. Do not touch another category's headers or tests without an explicit handoff request and approval.
- No specialist commits to master/default main, global `native/CMakeLists.txt`, shared `.github/workflows/`, or global `native/AGENTS.md`.
- If a required API resides in another team, create an adapter **inside your own subsystem** or describe a precise signature/diff in your handoff for the integrator.
- Use small integration commits when merging multiple large features; preserve specialist authorship/history and avoid blind file replacement.

## Recommended developer checks

```bash
git fetch origin
git switch parallel/<your-category>
git status --short
# Build focused native C++ tests privately using user-owned original data.
# Record compiler, flags, commands, source SHA, and results in the handoff.

# Integrator only (local commands, no GitHub Actions dispatch):
git switch native/original-elf-engine-recovery-20261020
git cherry-pick <reviewed-specialist-commit>
cmake -S native -B native/build
cmake --build native/build --parallel
ctest --test-dir native/build --output-on-failure
```

The CMake project currently FetchContent-downloads Lua 5.0.3 and cgltf, so a network-restricted environment may not complete a full build. Report this rather than claiming a successful build; avoid replacing original gameplay code to make it compile.

## Definition of completion

- **Recovered:** original game instruction/data provenance audited and a matching native C/C++ routine implemented.
- **Locally tested:** native focused tests actually run, original resource checks explicitly identified.
- **Integrated:** compiled into current complete native application and exercised with dependencies.
- **Playable:** the original game itself boots and executes genuine gameplay in a full native Windows executable. Nobody may claim playable based only on offline/probe tests.

A category can be locally recovered without being integrated or playable; preserve the distinction.
