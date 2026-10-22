# Original stage ownership and Lua integration pass

Spec: owner assignment `D:/Downloads/CODEX_TASK.md` and its private handoff; native contributor and integration protocols apply.

Goal: compile a reviewed native cluster on Windows, connect genuine stage resources to native Lua 5.0, and directly recover the still-missing original 0x57BCB8 initializer using existing translated navigation/math routines.

Architecture: retain the existing native engine and specialist history. A stage script session owns its stage resource scope, directory registry, descriptor world and Lua VM in dependency order. Resource consumers borrow immutable allocations. Original game stage selection and actor scheduling remain unresolved; the caller explicitly selects the stage/directory and no fabricated manager is installed.

Tech stack: C++20, MSVC x64, existing Lua 5.0.3, existing zlib, CMake/CTest.

## Constraints and decisions

- Keep original ISO/ELF/PCK/Lua private and ignored; source-only commits.
- Base and PR target are native/original-elf-engine-recovery-20261020; never main.
- No emulator, speculative original function, fabricated actor or game-boot claim.
- Owner explicitly requested implementation and verification in this pass; proceed inline without an additional design approval round.
- Four selected workstreams are reviewed as a dependency cluster; the other four remain pending, with every commit/path recorded in the ledger.

## Tasks

1. Validate owner sources and inventory all branch commits/paths. Record baseline configure/build/CTest results before integration.
2. Preserve selected runtime, assets, Lua and gameplay history through separate reviewed merges. Reconcile CMake requests for all selected source/test files. Compile and execute actual checkout sources.
3. Write a failing private-resource integration test for stage registry to Lua ownership. Add a native session with explicit execute/invoke operations, and fail before descriptor creation when a genuine actor/parameter manager is unavailable. Test missing resources, byte identity and allocation lifetime, real original Lua, and failure recovery.
4. Register original-resource verifiers and actual private ISO/ELF/PCK tests. Retarget/execute the genuine 168-chunk corpus and report unresolved calls precisely. Full CMake/CTest, followed by actual Win32 --boot-original launch.
5. Diagnose baseline compiler/runtime failures without changing source semantics or hiding failing tests. Record unresolved failures explicitly if repair requires unknown original behavior.
6. Review source-only diff, record evidence tiers, exact blockers and checksums; commit the integration branch. Prepare reviewable source deliverables without packaging an incomplete game.

Owner steering during execution: prioritize actual original-function recovery and do not redo or overwrite existing GitHub work. Inspected baseline/eight specialist refs and history; 0x57BCB8 remains an external callback. Added its 169-word native control-flow translation, source verifier, focused tests and all-15-real-type-2-action test, reusing existing helpers. Full translation notes and remaining projection/actor-owner dependencies are in RECOVERY_PASS_20261022.md. No architectural switch or fabricated actor runtime.

## Review focus

- Stage owner must outlive registry spans, Lua consumers and EXDB views; destruct in reverse dependency order.
- Source suffix pck.sz remains literal registration type; do not invent decompression renaming.
- Missing stage, script, callback and original actor factory must fail explicitly.
- Original Lua values remain unchanged; reject typed transport that cannot preserve them.
- Release tests must execute assertions; optional private inputs must not silently disappear from CTest.

## Verification

Use `cmake -S native -B native/build -G "Visual Studio 17 2022" -A x64` with explicit private cache paths; `cmake --build native/build --config Release --parallel 8`; `ctest --test-dir native/build -C Release --output-on-failure -j 8`. Preserve full local logs and extract exact result counts. Launch the real Win32 executable with verified ISO/ELF and inspect its per-attempt checkpoint journal. A fail-closed startup boundary is not gameplay.
