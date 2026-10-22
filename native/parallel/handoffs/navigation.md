# Original Navigation & Pathfinding — specialist handoff

**Exclusive branch:** `parallel/navigation`  
**Integration branch (unchanged):** `native/original-elf-engine-recovery-20261020`  
**Base:** `d483436416c14927851853f01c6ad66fe4cfbbf2`  
**First implementation commit:** `b77a5fd741ab0c1359fa73fc91b5998380dc7e0f`  
**Status:** Stage-bound source route integration implemented. Focused test added but **not executed** in this environment; no claim of native Windows gameplay boot.

## Recovered source addresses and scope

Existing original-code audit utilities in this repo pin USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Source addresses below come from those existing audits; the private ELF was **not available** in this working container for fresh independent verification.

- `0x21ED30` / `0x21ED94`: initialize/teardown three graph owners (source owner size `0x28`); `0x21D7D8` releases an individual graph.
- `0x4F0A5C` / `0x21D46C`: import a genuine .nav source into the owner and decode `0x70` polygon records.
- `0x21D70C` -> `0x21B0B0` / `0x21B240`: materialize type-1 child vectors from original source data. Existing `originalNavType1ActionDistance_220f28` computes `0x21B194` VU0 polyline cost. New adapter connects those original children to `0x220F20` (`0x21B4E0`) and `0x220F28` (`0x21B194`) route-search calls.
- `0x2207D0` / `0x220AA4` / `0x220C00` / `0x220D5C` / `0x220994`: dispatch, original graph admission, node creation, lowest-score relaxation, predecessor backtrace. These **existing** functions were reused without inserting an A* replacement.
- `0x21B444`, `0x221104`, `0x57BD88`: preserve type-0, type-1 and type-2 adjacency slot/actor decision. `0x57BA98` / `0x57BAF8` set the controller +0x1C multi-polygon flag; `0x57C7CC` advances actor route followers.
- `0x21F82C` / `0x4E3DEC`: existing linked membership owner insertion and first-strict-nearest fallback are reused, with a native cross-owner identity check.

## Delivered source and tests

1. `native/include/ae3/original_nav_stage_owners.hpp`: a **native-only** unique stage-owner token and per-slot monotonically changing revision for route invalidation on release, reattachment, teardown and reinitialization. Neither field is attributed to the guest `0x28` structure.
2. `native/include/ae3/original_nav_stage_route.hpp` and `native/src/original_nav_stage_route.cpp`: a stage-bound adapter that takes the actual decoded original graph, calculates original type-1 costs through retained source vectors, calls the original recovered dispatcher, converts route pointer identities to source indices, and provides valid source-polygon/edge/follower interfaces for `0x57BD88` and `0x57C7CC`. Rejects cross-stage linked lists and stale route snapshots. Does not introduce a new route search.
3. `native/tests/original_nav_stage_route_test.cpp`: self-contained synthetic source fixture with 4 polygons exercising type-1 child route preference, competing type-0 route, type-2 action recognition, original DFS admission order, explicit `$8` index, nearest-list fallback, actor follower inputs, two simultaneous source-owner slots, foreign-owner rejection, slot release/rebinding, stage teardown/reinitialization, and stage-instance identity.
4. The same executable optionally consumes one or more **owner-private** original `navigation.pck` files and checks every discovered source graph without storing or publishing source asset bytes.

## Tests and provenance

- Repository structure and integration API reviewed through linked GitHub source as of starting commit `d483436`.
- **Not run:** C++ compilation, native CTest and Windows game boot. GitHub connector access did not mount a local native checkout; container had no original private ELF or PCK. No GitHub Actions were triggered. Added synthetic tests are regression specifications until the integration owner compiles and runs them.
- Existing historical test source documents `woods_c/navigation.pck` with 39 + 20 polygon records, 15 type-2 links, and 0 type-1 links. This is historical repository evidence, **not a new private-asset validation**. Other genuine stage files are needed to exercise actual type-1 child paths.

## Windows/runtime integration recipe — integrator owns these edits

- Append `src/original_nav_stage_route.cpp` to the `Getchu3Assets` source list in `native/CMakeLists.txt`; keep existing `original_nav_route_dispatch.cpp`, `original_nav_route_search.cpp`, `original_nav_route_admission.cpp`, `original_nav_linked_owner.cpp`, `original_actor_nav_route.cpp` and `original_actor_route_follower.cpp`.
- Add:
```cmake
add_executable(Getchu3OriginalNavStageRouteTests tests/original_nav_stage_route_test.cpp)
target_link_libraries(Getchu3OriginalNavStageRouteTests PRIVATE Getchu3Assets)
add_test(NAME OriginalNavStageRouteOwnerType1Type2
         COMMAND Getchu3OriginalNavStageRouteTests)
if(AE3_RETAIL_WOODS_C_NAVIGATION_PCK)
  add_test(NAME OriginalNavStageRouteRetailWoodsC
           COMMAND Getchu3OriginalNavStageRouteTests "${AE3_RETAIL_WOODS_C_NAVIGATION_PCK}")
endif()
```
- After `OriginalNavStageOwners_21ed30::originalAttach_4f0a5c` succeeds, call:
```cpp
auto route = ae3::originalNavStageDispatch_2207d0(
    owners, sourceOwnerSlot, originalDispatchRequest, optionalListOwner);
auto follower = ae3::originalNavStageFollowerState_57c7cc(
    owners, route, actorSourcePolygonIndex, currentControllerTarget);
auto decision = ae3::originalNavStageRouteEdge_57bd88(owners, route, routeStep);
```
- Hold each snapshot only while `originalNavStageRouteCurrent(owners, route)` is true, and replace/clear an actor route on stage transition. Borrowed spans from follower inputs cannot outlive a snapshot or its owner. **Integration owner must wire these into the actual actor pipeline; this patch does not alter another team's files.**

## Important unresolved original behavior

- Genuine retail source audit of `0x57BCB8` actor controller post-constructor, actor action state machine, `0x21EF3C` navrange eligibility, and source class-2 action runtime dispatch remain incomplete and require the genuine private ELF/stage assets.
- Graph population is still conditional on original `0x21EF3C` eligibility; this patch does not manufacture list membership, actor polygons or targets.
- Pending game-owned type-2 action objects cannot be emulated by simply treating the original source table pointer as a host pointer; this patch preserves the recovered decision flag and source slot only.
- The CMake linkage and Windows runtime consumption have not yet been executed or validated.

## Next substantial work

Use owner's SHA-pinned USA ELF and at least two stages with type-1 action links to audit the `0x57BCB8` original controller state-transition logic, `0x21EF3C` membership filtering and route invalidation callers. Then integrate these exact source decisions into the pending actor-route ownership pipeline without touching the VU0 team's implementation.
