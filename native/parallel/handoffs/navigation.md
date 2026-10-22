# Original Navigation & Pathfinding — specialist handoff

**Branch:** `parallel/navigation` (sole write target)  
**Base:** `d483436416c14927851853f01c6ad66fe4cfbbf2`  
**Integration target, unchanged:** `native/original-elf-engine-recovery-20261020`  

## Commits and implementation

- `b77a5fd741ab0c1359fa73fc91b5998380dc7e0f`: connect original stage graph owner, type-1 route costs, original path dispatcher, native stable polygon index route snapshots, actor route/follower adapters.
- `cbd03b0af81725a421aebf4612e57870d5896295`: strengthen stage-instance/revision checks, expand synthetic and optional original-PCK tests, initial integration recipe.
- `8040c9fa40d22c7d222cec8213c03ea6d60dafc4`: add keyed navrange-to-live-graph group binding; connect original class-2 destination vectors and in-place actor route advancement.
- This handoff/test correction commit: fix type-2 and original-follower test expectations based on actual source-route ordering; broaden foreign-filter and early-return checks.

### Native files

| File | Change |
|---|---|
| `native/include/ae3/original_nav_stage_owners.hpp` | Host-only stage-instance token and per-slot graph revision; invalidate stale routes after original release and rebind |
| `native/include/ae3/original_nav_stage_route.hpp` | Public source-backed route snapshot, `0x57BD88` original edge decision, optional original `0x57BDB8` type-2 target, `0x57C7CC` original follower state/advance |
| `native/src/original_nav_stage_route.cpp` | Original stage route/type-1-cost/actor decision dispatch, pointer-to-source-index conversion, foreign borrowed list validation |
| `native/include/ae3/original_nav_stage_groups.hpp` | Keyed `navrange` group binding, stage revision validation and explicit register/rebind/invalidation API |
| `native/src/original_nav_stage_groups.cpp` | Connect active owner polygon storage and real source navrange resources to already recovered eligibility and original route selection |
| `native/tests/original_nav_stage_route_test.cpp` | Synthetic original-format source, optional owner-private per-stage PCK verification, actor follower action ordering, alternative paths, type-1/2 edges and graph invalidation |
| `native/tests/original_nav_stage_groups_test.cpp` | Multi-key navrange score/list registration, membership, fallback, source route selection, same-key dedup, type-2 target extraction, slot teardown/rebinding |

All source changes remain inside navigation ownership. No VU0/physics source or global CMake was edited.

## Source addresses and provenance

Address claims below derive from the **preexisting** pinned original USA ELF verifiers in `native/tools/`; no original binary bytes were directly available for re-disassembly this pass. Original owner-private `SCUS_975.01` SHA-256 must equal `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

- `0x21ED30` / `0x21ED94` / `0x21D7D8`: allocate three `0x28` owners, teardown owners and release records. Native revision tokens are safety metadata and **not claimed as original EE fields**.
- `0x4F0A5C` / `0x21D46C`: source `.nav` load and construction of original `0x70` spatial records.
- `0x21D70C` / `0x21B0B0` / `0x21B240` / `0x21B194`: original type-1 vector-array materialization and VU0 polyline path cost. New type-1 adapter provides `0x21B4E0` lookup used at original search `0x220F20`.
- `0x21D6DC`: type-2 original source-table pointer resolution; `0x57BD88` / `0x57BDB8`: original class-2 actor branch, select route[1], fetch its three original vertex vectors. **Subsequent VU0 motion remains untranslated**.
- `0x2207D0` / `0x220AA4` / `0x220C00` / `0x220D5C` / `0x220994`: source graph query, admission DFS, graph nodes, original strict route cost update, predecessor-chain extraction. Existing code reused: **no alternative A* pathfinder**.
- `0x21EF3C`: existing mathematical recovery of original navrange membership score. **Not bit-exact** for all PS2 VU0 precision/NaN behavior, not claimed fully recovered.
- `0x4E4060` / `0x21F6D4` / `0x21F82C` / `0x4E3DEC`: original resource-key registration, polygon eligibility callback, front-of-list insertion and strict-first nearest fallback.
- `0x57BA98` / `0x57BAF8` / `0x57C7CC` / `0x57C8EC`: actor-route count byte and original route advancement/target updates, with missing VU0 geometry kept in explicit callbacks.

## Compilation, verification and evidence

**GitHub connector evidence:** source and rule files read on the exact navigation branch. Branch heads/commits were verified through GitHub API after each write.

**Native C++ tests: not executed.** Direct clone/download from this isolated container failed because public GitHub DNS/network access is disabled. GitHub connector permits source edits and commits but provides no C++ execution. The owner-private USA ELF or original PCK were not present in mounted data/Library search; therefore the retail verifier scripts, PCK regressions, native CTest and actual Windows game boot were **not run**. New tests are committed but require integration-team build to confirm. No GitHub Actions were dispatched. No private executable or game-resource bytes were published.

Historical existing tests cite `woods_c/navigation.pck` with 39+20 polygons and 15 source type-2 links; that is **repository test evidence**, not a new measurement.

## Required integration (Windows/runtime owner only)

Keep all existing native navigation sources. Add the two new translation units to existing `Getchu3Assets` in `native/CMakeLists.txt`:

```cmake
src/original_nav_stage_route.cpp
src/original_nav_stage_groups.cpp
```

Register two new focused native tests:

```cmake
add_executable(Getchu3OriginalNavStageRouteTests tests/original_nav_stage_route_test.cpp)
target_link_libraries(Getchu3OriginalNavStageRouteTests PRIVATE Getchu3Assets)
add_test(NAME OriginalNavStageRouteOwnerType1Type2
         COMMAND Getchu3OriginalNavStageRouteTests)

add_executable(Getchu3OriginalNavStageGroupsTests tests/original_nav_stage_groups_test.cpp)
target_link_libraries(Getchu3OriginalNavStageGroupsTests PRIVATE Getchu3Assets)
add_test(NAME OriginalNavStageRangeAndRouteBindings
         COMMAND Getchu3OriginalNavStageGroupsTests)

if(AE3_RETAIL_WOODS_C_NAVIGATION_PCK)
  add_test(NAME OriginalNavStageRouteRetailWoodsC
      COMMAND Getchu3OriginalNavStageRouteTests "${AE3_RETAIL_WOODS_C_NAVIGATION_PCK}")
endif()
```

Actual API call flow once original stage owners are initialized and resources are attached:

```cpp
ae3::OriginalNavStageGroupSession groups(owners, graphSlot);
groups.originalRegisterRange_4e4060(originalNavrangeSource);
auto route=groups.originalRouteByKey_2207d0(originalGroupKey,originalRouteQuery);
if(ae3::originalNavStageRouteCurrent(owners, route)) {
    auto edge=ae3::originalNavStageRouteEdge_57bd88(owners,route,0);
    auto originalClass2=ae3::originalNavStageType2Target_57bdb8(owners,route);
    auto follower=ae3::originalNavStageFollowerState_57c7cc(
        owners,route,actorPolygon,currentControllerTarget);
    // The physics-owned callbacks implement original 0x4C1B3C
    // projection and 0x57C8EC target update. No substitute motion.
    auto moved=ae3::originalNavStageAdvanceRoute_57c7cc(
        owners,route,actorPolygon,currentControllerTarget,physicsCalls);
}
```

`OriginalNavStageGroupSession` is a native **single-owner-slot** borrowing view and must not outlive `owners`. Call `originalClear_4e3fec()` before dropping the owner or `originalRebindSourceGraph()` after its generation changes, and re-register the original group source. The keyed session deliberately rejects unknown keys instead of inventing default eligibility or graph choice. If the genuine original runtime shares one group key across multiple source-owner slots, the integrator must audit source owner routing before expanding this contract; do not force mixed graphs into one slot.

## Remaining original blockers

1. Decompile source `0x57BCB8` original controller post-constructor and subsequent real actor movement/state transitions using original executable input.
2. Complete original `0x21EF3C` COP2/MMI precision behavior and validate native navrange membership against genuine stage data.
3. Recover downstream type-2 actor motion beyond vertex reads (`0x57BDFC` onward) and stage-transition actor route replacement.
4. Validate real stage group ownership across nav_monkey/nav_enemy and multi-key resources; avoid assuming the one-slot native adapter is the retail global registry.
5. CMake + CTest and native Windows game boot are **integrator-only**; this branch is not independently verified playable.
