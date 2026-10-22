# Original Navigation & Pathfinding — specialist handoff

**Branch:** `parallel/navigation` (sole write target)  
**Base:** `d483436416c14927851853f01c6ad66fe4cfbbf2`  
**Integration target, unchanged:** `native/original-elf-engine-recovery-20261020`  

## Commits and implementation

- Direct-to-VU0 original stage actor pipeline bridge (new navigation commit; see branch head) with two-polygon source-backed fixture and local stubbed C++20 sanitizer smoke check.

- Continuation: actor-bound navigation route lifecycle, typed source action/target integration and group weak-lifetime safety (new commit; inspect branch head).

- `b77a5fd741ab0c1359fa73fc91b5998380dc7e0f`: connect original stage graph owner, type-1 route costs, original path dispatcher, native stable polygon index route snapshots, actor route/follower adapters.
- `cbd03b0af81725a421aebf4612e57870d5896295`: strengthen stage-instance/revision checks, expand synthetic and optional original-PCK tests, initial integration recipe.
- `8040c9fa40d22c7d222cec8213c03ea6d60dafc4`: add keyed navrange-to-live-graph group binding; connect original class-2 destination vectors and in-place actor route advancement.
- `01b6c689eda08b838ec64e4a8bb4fad04603bf28`: fix type-2 and original-follower test expectations based on actual source-route ordering; broaden foreign-filter and early-return checks.

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

## Navrange membership-version route invalidation

A stage graph can remain live while `0x4E4060` populates additional source polygons into an existing navrange key. The first stage bound route adapter tracked graph release/replacement but did not detect this separate list mutation. The native `OriginalNavStageGroupSession` now provides `originalBoundRouteByKey_2207d0`, returning `OriginalNavStageGroupedRoute` with the source route and a separate host-only group revision/session token. Call `session.originalGroupedRouteCurrent(bound)` before using `bound.sourceRoute`. Source list insertions (`0x21F82C`), group clear (`0x4E3FEC`) and rebind invalidate earlier grouped routes; idempotent registration does not. Group revision/token are **native lifetime safeguards**, not claimed recovered PS2 fields.

The focused group test exercises all these cases, including a source navrange extent change that adds polygon 3 to an existing group without releasing the graph; old grouped route is rejected while graph route identity remains current. The tests are committed, **not executed**.

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
auto bound=groups.originalBoundRouteByKey_2207d0(originalGroupKey,originalRouteQuery);
auto& route=bound.sourceRoute;
if(groups.originalGroupedRouteCurrent(bound)) {
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


## Actor route binding / recovered caller integration — current pass

This continuation introduces a host-native, single-actor `OriginalNavActorRouteBinding` in:

- `native/include/ae3/original_nav_actor_binding.hpp`
- `native/src/original_nav_actor_binding.cpp`
- `native/tests/original_nav_actor_binding_test.cpp`

The adapter does **not** implement a substitute navigation search or pretend to be the original EE actor controller. It owns the lifetime of a previously recovered source route snapshot or a navrange-group-bound route, and invokes the existing original-code-derived `0x221104`/`0x21B444`/`0x57BD88` action classification, `0x57BDB8` type-2 target lookup, and `0x57C7CC` route follower, requiring VU0/physics callbacks for original `0x4C1B3C` and `0x57C8EC`. Original actor controller `0x57BCB8` remains unrecovered.

The bridge clears current target (+0x50 semantic pointer) on source route replacement or stage transition. A released/rebound stage or changed `navrange` membership makes the actor route invalid and returns no exposed stale polygon target. Foreign stage routes are rejected. The stage-owner lifetime must extend beyond the actor binding. A **host-only weak lifetime token** in `OriginalNavStageGroupSession` rejects a group session that has already been destroyed, without dereferencing its stale pointer. These protections are native integration safety rules, not invented claims about EE fields.

New focused test exercises original search tie/predecessor ordering, class-0 and class-2 edges, actual three source vectors for `0x57BDB8`, `0x57C7CC` callback ordering and unchanged-target early return, source-group admission, invalidation after membership growth, safe group session destruction, stage unload, and stage reattach. It contains no private game data.

### Additional CMake request (integrator only)

Append to the native assets translation units:

```cmake
src/original_nav_actor_binding.cpp
```

Register the test:

```cmake
add_executable(Getchu3OriginalNavActorBindingTests tests/original_nav_actor_binding_test.cpp)
target_link_libraries(Getchu3OriginalNavActorBindingTests PRIVATE Getchu3Assets)
add_test(NAME OriginalNavActorBindingRouteLifecycle
         COMMAND Getchu3OriginalNavActorBindingTests)
```

Example native caller:

```cpp
ae3::OriginalNavActorRouteBinding actor(owners);
auto bound=groups.originalBoundRouteByKey_2207d0(selectedOriginalKey,originalRouteQuery);
actor.originalUseGroupRoute_2207d0(groups,std::move(bound));
if(actor.originalRouteCurrent()) {
    auto firstEdge=actor.originalCurrentEdge_57bd88(0);
    auto originalType2=actor.originalInitialType2Target_57bdb8();
    auto follow=actor.originalAdvance_57c7cc(actorSourcePolygonIndex,physicsCalls);
}
if(actor.originalDropInvalidRoute()) {
    // Caller must handle source route replacement as required by
    // original 0x575EC0; this adapter does NOT free game allocations.
}
```

**Verification:** GitHub connector commit/write verification only. No local native build, CTest, original private ELF/PCK audit or Windows game boot was run. The isolated container lacks GitHub network resolution, and the private executable/resource files were not accessible. No GitHub Actions were triggered.

### No-route and partial-registration failure semantics

The actor-binding adapter treats an original `0x2207D0` route with an empty `forwardPath` (e.g. route mode selecting explicit original search index `-1`) as **no active actor route**. It clears a previously bound route rather than proceeding through `0x57C7CC`'s no-multiple-polygon branch and accidentally applying actor+0x30 as if a route existed. This is a native caller guard, not a new pathfinding decision.

The stage group binding wraps original recovered `0x4E4060` registration with exception-safe conservative generation invalidation. If a failure occurs after some group nodes were inserted, prior group routes cannot remain valid. A malformed original resource fails and invalidates borrowed route identities even if mutation did not yet start (conservative host safety). The focused actor regression tests both the explicit no-route transition and malformed-resource invalidation. **Not executed in this environment.**

## New direct native stage actor pipeline bridge

**Source files:** `native/include/ae3/original_nav_actor_binding.hpp` (new public member), `native/src/original_nav_actor_stage_bridge.cpp`, and `native/tests/original_nav_actor_stage_bridge_test.cpp`.

The new `OriginalNavActorRouteBinding::originalAdvanceNativeStage_57c7cc` forwards the already recovered original `0x2207D0` route's ordered source polygon IDs and `0x57BA98` signed path-count test (`>=2`) into the **existing** `OriginalStageActorNavigation_57c7cc::advance` owned by VU0/Physics. That class already dispatches original `0x57C7CC` follower, `0x4C1B3C` source ray/collision, `0x57C8EC` target-update and original shared game random. No substitute VU0 movement or authored pathfinding was added, and no VU0/physics code was modified.

Safety: checks active source route/group generation; checks caller-supplied source graph **object identity** against the live stage owner's decoded graph; checks the original actor polygon index, every route polygon index and any current controller+0x50 polygon pointer **before** calling the stage pipeline. The native actor binding reads the real pipeline's updated target pointer afterwards instead of overriding its existing target. The stage/graph pairing **is still a caller precondition**, since the physics-owned stage class does not expose the graph it was originally constructed from. **Request to VU0 team:** expose a const read-only accessor `const OriginalStageSpatialLoaded& originalBoundStageSource() const noexcept` returning its existing private `stage_`; then the navigation bridge can verify the actual pipeline's stage identity directly rather than taking it on faith from the integrator. Only VU0 team should change that file.

New CMake recipe for integrator only:

```cmake
# Append to Getchu3Assets sources:
src/original_nav_actor_stage_bridge.cpp

add_executable(Getchu3OriginalNavActorStageBridgeTests
    tests/original_nav_actor_stage_bridge_test.cpp)
target_link_libraries(Getchu3OriginalNavActorStageBridgeTests PRIVATE Getchu3Assets)
add_test(NAME OriginalNavActorStagePipelineBridge
    COMMAND Getchu3OriginalNavActorStageBridgeTests)
```

Native integration caller:

```cpp
if (actor.originalRouteCurrent()) {
    auto result=actor.originalAdvanceNativeStage_57c7cc(
        existingOriginalStageActorNavigation,
        *owners.originalAt(selectedNavOwnerSlot).decodedGraph,
        actorSourcePolygonIndex);
}
```

A focused synthetic original-format two-polygon test creates a genuine `0x1E`-stride source nav descriptor, decodes it through the preexisting retail stage parser, calls the recovered original source pathfinder to produce a path, constructs the real physics-owned `OriginalStageActorNavigation_57c7cc`, and checks target advancement, projection early return, different-graph rejection, invalid actor index, single-polygon mode and stage release. It is a **source-format synthetic test** rather than a claim of original game-level parity.

**Testing this turn:** Real C++20 `g++ -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined` *isolated* adapter and bridge smoke harnesses compiled and ran successfully using minimal locally authored dependency stubs. These exercise source lifetime guards, no-route state, bridge dispatch, wrong stage identity, foreign target and invalid indices. They do **not** compile/link the full repository source tree, and the new repository integration test is not executed here (container DNS cannot resolve github.com for a clone). Original USA executable/PCK verification and native Windows boot are not performed; no Actions triggered.

## Source-backed actor retarget / original destination changes

A second navigation-only adapter extension adds:
- `OriginalNavActorRouteBinding::originalRetargetStage_2207d0(slot, request, optionalOriginalMembershipOwner)`.
- `OriginalNavActorRouteBinding::originalRetargetGroup_2207d0(groupSession, originalKey, request)`.

Both call the **existing** recovered source `0x2207D0` dispatcher and its existing group/list fallback. The next route is computed in temporary native storage and installed only after successful admission, type-1 cost and destination extraction. Thus a malformed request, missing group or missing source allocation throws without replacing an already current actor path. A valid original explicit destination `-1` remains **no route**, and clears the old actor binding instead of fabricating another destination. On a successful replacement, controller+0x50 native target is cleared so the existing recovered `0x57C7CC` follower can decide its new target on the next tick. No A*, heuristic, physics rewrite or synthetic state machine was added.

The extended `original_nav_actor_binding_test.cpp` covers nearest/farthest target change, failed query retaining the previous route, explicit -1 removing it, successful regeneration and failed group-key retarget preserving the earlier path.

**Execution evidence:** In addition to the earlier C++20 ASan/UBSan boundary smoke harness, a local isolated `g++ -std=c++20 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined` retarget smoke check compiled and ran against dependency stubs; the focused full-repository tests and Windows game were **not** compiled/booted in this environment.

## Fail-closed navrange registration after source errors

The navigation-owned `OriginalNavStageGroupSession` now tracks a **native-only** `registrationFailed_` state for the case where existing recovered `0x4E4060` group registration throws on a malformed source allocation or after a subset of `0x21F82C` source polygon front-insertions has occurred. A failure invalidates existing group route generations and blocks all subsequent grouped route dispatch and registration until `originalClear_4e3fec()` or a successful source-graph `originalRebindSourceGraph()`. Exposed `originalRegistrationNeedsReset()` is only a host integration diagnostic, not a reconstructed retail EE member.

This specifically prevents an actor from accepting a new route through a partially populated original group registry. Existing successful registration, idempotent duplicate filtering, ordered list links, source per-polygon eligibility, and 0x2207D0 DFS/route ordering remain untouched.

The extended `original_nav_stage_groups_test.cpp` and `original_nav_actor_binding_test.cpp` exercise a bad group key allocation, rejected new routes and repeated registrations, explicit reset, subsequent recovery, and actor route invalidation. Tests are committed but **not run against a full native build** here.

## Source-route integrity audit — 0x220994 / 0x221104 native caller boundary

The recovered original `0x221104` source-edge lookup intentionally returns edge slot 0 when a destination does not match any of the three neighbor attempts. That **retail behavior remains unchanged**. But caller-owned `OriginalNavStageRouteSnapshot` is a copyable native data object; malformed paths could otherwise reach `0x57BD88` and incorrectly appear to use source edge slot 0 (including false type-2 classification).

New navigation-owned **host-integrity** (not a newly translated EE routine): `native/include/ae3/original_nav_path_integrity.hpp`. `originalNavValidateNativeSourceRouteIntegrity` validates the exact original `.nav` count and `0x1E` descriptor table, original `0x3800 / 0x38 = 256` admission-workspace cap, unique admitted DFS source IDs, original `0x220994` root and selected-workspace-index correspondence, forward path acyclicity and membership, and each **directed** source neighbor edge before the route is exported or consumed by an actor. Legitimate source-order alternative predecessor paths are retained; no distance calculations, path scoring, edge fallback or A* logic has been changed. Valid original explicit destination `-1` still yields an empty path.

Validation added to `originalNavStageDispatch_2207d0`, public source polygon/edge/follower and type-2 accessors in `original_nav_stage_route.cpp`, and actor route ingestion for both plain and keyed navrange routes in `original_nav_actor_binding.cpp`. A bad candidate is rejected **before** replacing an already valid actor route; the original lowest-score traversal `0x220D5C`, extractor `0x220994`, source `0x221104`, and physics-owned movement are unchanged. This is native validation of host-boundary data, not a claim to have decompiled missing retail movement instructions.

**Tests:** New standalone `native/tests/original_nav_path_integrity_test.cpp` includes genuine descriptor-format fixtures, both allowed paths to the same endpoint, malformed workspace index, cycles, disconnected links, repeated/out-of-range IDs and 20,000 deterministic route mutations. Extended `original_nav_actor_binding_test.cpp` and `original_nav_stage_route_test.cpp` to check that public actor/edge/follower/polygon consumers reject a modified disconnected route. The standalone validator logic was compiled and **executed locally** against its exact new 3,843-byte header in `/mnt/data/nav_integrity_validation` with:

```
g++ -std=c++20 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer -I /mnt/data/nav_integrity_validation /mnt/data/nav_integrity_validation/check.cpp -o /mnt/data/nav_integrity_validation/check
/mnt/data/nav_integrity_validation/check
clang++ -std=c++20 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer -I /mnt/data/nav_integrity_validation /mnt/data/nav_integrity_validation/check.cpp -o /mnt/data/nav_integrity_validation/check_clang
/mnt/data/nav_integrity_validation/check_clang
```

Both reported `PASS 20000 randomized C++20 source-path validator cases; rejected=16004`. Additionally, the new standalone `original_nav_path_integrity_test.cpp` source was reproduced locally as `/mnt/data/nav_integrity_validation/test_repo.cpp` and independently **compiled and passed** with both `g++` and `clang++` using the same `-std=c++20 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer` flags. No full-project CMake/CTest is claimed. This **does not** mean that the full repository integration CTest was built or run: no full checkout is mountable, direct github.com DNS is blocked in the container, and original private `SCUS_975.01` and original stage PCK were not accessible. The isolated sanitizer run covers the header's behavior only. No Actions triggered.

**CMake request (integration owner only):** the header adds no translation unit to Getchu3Assets. Register:

```cmake
add_executable(Getchu3OriginalNavPathIntegrityTests
    tests/original_nav_path_integrity_test.cpp)
target_link_libraries(Getchu3OriginalNavPathIntegrityTests PRIVATE Getchu3Assets)
add_test(NAME OriginalNavSourceRouteIntegrity
    COMMAND Getchu3OriginalNavPathIntegrityTests)
```

The existing handoff's previous instructions for adding `original_nav_actor_binding.cpp`, `original_nav_stage_route.cpp`, `original_nav_stage_groups.cpp`, and stage bridge tests remain required.

## Navigation specialist checkpoint — 2026-10-21

Two commits on `parallel/navigation` since `252f8342469f074a40a00eecd34ad22ba87d0739`:

- `4d1d9714cd766965ff276f9904a0e747f6609b40`: host-only original circular-list generation and weak lifetime tracking integrated with original `0x2207D0` stage routes and original actor binding, preserving retail list order and link decisions.
- `6d3e8853c89ec22081506f59f3fb1d2cad15ea44`: distinguish untracked temporary source membership spans from durable original list owners; reject only the former when installing long-lived native actor routes.

**CMake:** no new translation units were added, because both changes modify already existing navigation files. No global `native/CMakeLists.txt` edit or GitHub Actions execution. Local native C++ isolated list model was tested with GCC and Clang + ASan/UBSan; full original navigation and game binaries **remain uncompiled/unexecuted in this workspace**.

## Direct original membership-owner route lifetime — 0x21F604 / 0x21F82C / 0x21F804

A remaining native ownership defect existed in `originalNavStageDispatch_2207d0(..., OriginalNavListOwner*)`: it preserved the source member-list ordering during the original `0x2207D0` search but returned a route snapshot tracked only by the **graph's** revision. When `0x21F82C` inserted a newly eligible polygon, `0x21F804` erased one, `0x4E4154` cleared the list, or `0x21F64C`/`0x21F690` destroyed the sentinel, a previously selected actor route could remain marked current despite having been generated from obsolete member candidates. The keyed navrange group session tracked group registration, but generic original membership owners did not.

**Implementation:** `original_nav_linked_owner.hpp/.cpp` introduces an independent **host-only**, weakly referenced list-revision state (not a recovered EE pointer or original data structure). Successful original first-position inserts and nonzero erasures bump the revision; no-op dedup, absent erasure and empty-list clear do **not**. Nonempty clear bumps the revision; owner disposal expires the weak state, preventing a newly allocated sentinel at the same host address from reviving stale routes. Intrusive next/prev ordering, genuine `0x4E3DAC` membership, `0x4E3DEC` first-strict-nearest fallback and original `0x21F82C` front insertion are left intact.

`OriginalNavStageRouteSnapshot` now optionally holds `hostMembershipBound`, `hostMembershipGeneration`, and a weak list-revision token. `originalNavStageDispatch_2207d0` captures the token **only** for a real borrowed `OriginalNavListOwner*`. `originalNavStageRouteCurrent` checks graph owner identity/revision first, then list existence and revision when bound. The existing actor binding, `0x57BD88` action decisions, type-2 polygon reads, source-polygon accessors and follower handoff already consult this current-route function. No new A* or route choice was added. Raw `OriginalNavDispatchRequest::orderedOriginalMembershipList` spans without a list owner remain a one-call source API: they do not carry a persistent revision guard. Long-lived actor routes should use an owned original list or keyed navrange session.

**Regression coverage added:** `original_nav_linked_owner_test.cpp` verifies successful and duplicate insert generations, present and absent erasure, nonempty/empty clear, disposal, and recreating a fresh list. `original_nav_stage_route_test.cpp` verifies a direct membership-bound original route is invalidated by new member insertion, erasure and disposal, while the same stage's unrestricted original route remains valid. `original_nav_actor_binding_test.cpp` verifies direct list source route regeneration through the genuine `0x2207D0` dispatcher, changed route length after insertion, actor drop on stale membership, and target invalidation after disposal. These repository tests were committed but **not executed** in the absent native checkout.

**Local C++ test evidence:** A self-contained reconstruction of the modified circular-list algorithm (not a full translation-unit build) was compiled using both `g++` and `clang++` with `-std=c++20 -O1 -g -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer`. Both passed 100,000 deterministic randomized insert/erase/clear/list-order checks plus weak-lifetime destruction/recreation assertions. File: `/mnt/data/ae3_nav_listcheck/check.cpp`. The first attempt failed a strict `-Werror=misleading-indentation` warning in that isolated harness, which was fixed before both successful executions. **Not evidence of full repository CTest, original ELF parity, or a running Windows game.**

The original USA source addresses above remain grounded in the preexisting SHA-pinned verifier scripts; the private `SCUS_975.01` and game PCK were not present to re-disassemble or compare in this turn. No assets, master branch, shared CMake, VU0 code or GitHub Actions were changed.

## Ephemeral original membership-span lifetime guard

The original route helper accepts `OriginalNavDispatchRequest::sourceMembershipListExists` and `orderedOriginalMembershipList` as a caller-supplied `std::span`, even when no real `OriginalNavListOwner` exists. That is a valid immediate native source query, but the supplied view has **no** original `0x21F604` owner, no native circular-list mutation token and no way to observe a subsequent key/source-membership update.

`OriginalNavStageRouteSnapshot::hostEphemeralMembershipFilter` now explicitly labels such temporary-filter results. `originalNavStageDispatch_2207d0` still runs the **same** recovered original search, returns the same route, and permits immediate source edge/polygon diagnostics. `OriginalNavActorRouteBinding::originalUseStageRoute_2207d0` and grouped actor binding reject the ephemeral flag, preserving any earlier actor route instead of storing an untrackable temporary membership selection. Persistent actor routes must use a real `OriginalNavListOwner` (which now carries the weak generation state) or a live keyed `OriginalNavStageGroupSession`. The source `0x2207D0`, `0x4E3DAC` and `0x221104` behavior remains untouched.

Additional assertions in `original_nav_stage_route_test.cpp` and `original_nav_actor_binding_test.cpp` verify that an immediate span-only query stays available, is marked ephemeral, and cannot replace a good actor route. These full native tests were **not run** in this network-isolated workspace; the previous isolated 100,000-case C++20 list fuzz test remains the only executed test for this pass.

## Remaining original blockers

1. Decompile source `0x57BCB8` original controller post-constructor and subsequent real actor movement/state transitions using original executable input.
2. Complete original `0x21EF3C` COP2/MMI precision behavior and validate native navrange membership against genuine stage data.
3. Recover downstream type-2 actor motion beyond vertex reads (`0x57BDFC` onward) and stage-transition actor route replacement.
4. Validate real stage group ownership across nav_monkey/nav_enemy and multi-key resources; avoid assuming the one-slot native adapter is the retail global registry.
5. CMake + CTest and native Windows game boot are **integrator-only**; this branch is not independently verified playable.


## 2026-10-22: Original actor controller 0x57BCB8 source-audit groundwork

**New navigation-owned offline analysis tools (Python, no runtime code replacement):**
- `native/tools/analyze_original_nav_controller_57bcb8.py`
- `native/tools/test_analyze_original_nav_controller_57bcb8.py`

**Original evidence already present in repo:** `native/tools/verify_original_actor_nav_controller.py` hashes the private USA `SCUS_975.01` at `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63` and the original `0x57BCB8..0x57BD3C` post-constructor region (SHA-256 `ca0b2213c64510b3491ec4799aebc5e6a0bb2b375cd43ea0c9f9094e75070f38`). Existing verified caller sites `0x57BAE4` and `0x57BB4C` invoke `0x57BCB8` from both actor controller constructors. Verified machine words show `0x57BCB8` stack allocation `addiu sp,sp,-0x80`; `0x57BCDC` reads controller +0x30 using LBU; `0x57BCE0` takes the zero-variant branch to `0x57BD2C`; and `0x57BD24` has COP1 `BC1T` to `0x57BD30`. Other instructions and exact state-machine effects are **not inferred** without the genuine executable.

**New tooling behavior:** validates entire private ELF SHA and the exact 132-byte controller region SHA; decodes the 33 original MIPS instruction words into register operands and memory offsets; annotates branches, calls, branch delay slots, likely-branch annulment, basic blocks and successor addresses; marks unsupported COP1/COP2/MMI opcodes unresolved rather than inventing semantics. The report is a **private analysis artifact** that contains original 32-bit opcode words; do not commit/publish that report or any executable bytes. Only scripts and synthetic tests were committed.

**Run privately from the repository root:**

```bash
python native/tools/analyze_original_nav_controller_57bcb8.py /private/path/SCUS_975.01 --json-out /private/path/57bcb8-source-report.json
python -m unittest discover -s native/tools -p test_analyze_original_nav_controller_57bcb8.py -v
```

**Local verification, this turn:** the standalone synthetic unit suite was actually executed in the container: 8/8 `unittest` tests passed. It checks original address/branch arithmetic, decoded verified opcode anchors, branch-delay and basic-block metadata, likely-branch delay annulment, unresolved COP2/MMI instructions, and rejection of nonmatching ELF input. The authored test/tool source was SHA-matched to the GitHub blobs using `git hash-object` before committing. **Real `SCUS_975.01` was not mounted here**: the full original `0x57BCB8` report was not produced, its remaining instructions were not recovered, and no actual native gameplay/controller behavior was implemented or tested. This is a source-recovery aid toward completing the blocking actor constructor, not a substitute for it.

**Next mandatory step to recover actual runtime:** run the verifier and new instruction report against the user's SHA-pinned executable, map unknown loads/stores and COP1 comparisons to source actor and route owner fields, implement exact branch behavior in navigation-owned `original_actor_nav_controller.cpp` (and necessary callers), test stage actor runtime together with VU0-owned physics via the integration team. No source files from other teams, global CMake, game bytes, master branch or GitHub Actions were modified in this pass.
