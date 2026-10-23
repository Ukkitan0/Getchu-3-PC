# Original actor source snapshots across native Unreal manager callbacks

The owner-original Ape Escape 3 USA executable is the source of game behavior.
This note describes host-native **ownership and atomicity**, not a new gameplay
algorithm. No private ISO, original executable, Lua resources, or stage data are
included in this repository.

## Original initializer 0x57BCB8

Entry point: `ae3_original_manager_init_step` and
`ae3_original_manager_init_step_v2` in
`native/src/ae3_original_manager_init_bridge.cpp`.

Both entry points translate the source branch selection and delegate unresolved
effects to the original manager. Before the first external callback, they now
copy all caller-owned route inputs needed by the recovered source logic:

- Original 0x70-byte spatial polygon records and actor/target polygon identities.
- Original 32-bit route polygon IDs, including the `route[1]` selection used by
  the 0x57C7CC follower.
- Original navigation bytes, including 0x1E-byte action descriptors and
  the original route/action-kind query inputs.
- Initial controller state and the source-original callback table.

Only the committed controller fields and source RNG cursor are published on a
successful return. The original 0x21B854 projection callback cannot silently
change later route reads in the same initializer invocation by modifying a
borrowed input array or replacing the registration table.

**Host side-effect boundary:** the older v1/v2 initializer callbacks still
remain owner-defined, nontransactional effects. Native state publication is
guarded, but a manager whose callback directly mutates its live actors must
provide its own rollback. This is NOT a completed Player/Monkey owner.

## Original route step 0x57C7CC, full-width ABI v3

Entry point: `ae3_original_actor_route_step_v3`.

This call holds its native session guard through the whole
Begin → Stage → Commit/Abort sequence. It snapshots the original source
records, polygon descriptor point counts, route indices, ray/control operands,
and callback table before calling the owner. The source 0x57C8EC target
calculator receives this invocation-local data rather than potentially
modified borrowed inputs.

The target's original **128-bit** low/high result and source VU0 R operand
are staged with the real owner; the game RNG advances only after a successful
commit. Any unavailable genuine owner state or rejected callback aborts, with
no result publication. This does not create game actors or complete
0x57C620 Player/Monkey update/render behavior.

Legacy route APIs v1/v2 are NOT interchangeable with v3: v2 cannot represent
original 0x57C894 follow-up projection's HIGH64 and rejects that path. Only
v3 is suited to multi-target source-backed integration.

## Regression coverage

- `native/tests/original_manager_unreal_abi_test.cpp`: original 0x21B854
  callback mutates borrowed original route/nav buffers and rebinds the
  0x57C8EC owner callback before the recovered 0x57C7CC follow-up.
  The in-flight initializer must still complete using its captured data;
  a later invocation with corrupted route bytes must be rejected.
- `native/tests/original_route_native_target_v3_test.cpp`: original owner
  Begin changes source polygon vertices, controller operands, point-count
  descriptors, and its callback table. The route must use the captured input
  and commit target through its captured original owner methods. Additional
  tests cover rejection of reentrant route stepping and session destruction.
- `native/unreal_bridge/CMakeLists.txt` already registers these existing
  native host ABI test targets; no extra CMake dependency was introduced.

The above describes committed source and tests; it is **not evidence of a
successful Windows CTest, Unreal Editor build, original-game start, visual
parity, or playable game**. Run the original host ABI tests in an actual
Windows build and the owner's genuine stage/actor loop before marking
integration complete.

## Still blocking original gameplay

1. The source-derived Player and Monkey factory/owner lifetimes and the
   actual per-frame actor dispatcher must be connected.
2. Full original `0x57C620` actor controller update and associated
   game-side movement/animation ownership remain unfinished.
3. Actual original Lua game manager binding implementations, gadget/capture,
   collision response, sound, presentation, menu and progression loops must
   run together on genuine owner-private ISO resources.

The safe snapshot layer makes these later connections more reliable; it does
not replace or approximate any missing source engine behavior.

## Original actor controller constructor 0x57BAF8: complete SQ field

A second source-backed constructor fix now connects the existing
`OriginalActorRouteControllerPrepared_57baf8` evidence to the active
`OriginalManagerChild70` lifecycle representation:

- Original `0x57BAF8` stores the full source EE `$a3` 16-byte vector at
  controller `+0x20..+0x2F` **before** calling original `0x57BCB8`.
  The host-native variant-one constructor `originalManagerChildCtor_57baf8`
  and allocator `originalManagerChildNew_31759c` now REQUIRE those exact
  bytes as an input; no fabricated zero controller is accepted as a
  substitute for an unavailable original owner vector.
- Original `0x57BA98` variant-zero construction does NOT write `+0x20`.
  A distinct optional source-vector field preserves the difference between
  missing/uninitialized and the real zero vector, and does not overwrite
  an already present source vector on the variant-zero path.
- The native Unreal initializer bridge populates this field from the
  host-provided original controller vector for the variant-one path.
  Its lifecycle and vector ownership are still transient until a source
  Player/Monkey manager allocates a genuinely persistent game actor.
- `native/tests/original_manager_child_test.cpp` checks all 16 copied
  bytes **inside** the pre-initializer callback and afterward, the variant
  zero non-write rule, source flags, and child dispatch/teardown ordering.

This is a recovered source field transfer, **not** a complete Player/Monkey
factory or `0x57C620` update loop. It neither invents the unported original
animation/movement operations nor supplies a synthetic Unreal actor.

### Constructor → initializer invariant

The source-level `originalManagerInitialize_57bcb8` now requires the
variant-one lifecycle's saved `controller20` SQ and the initializer's
`controller20` to be **bit-identical** before executing original actor
math. This is a native host ownership check: the genuine `0x57BAF8`
constructor must already have provided the full 16-byte vector before
source `0x57BCB8` executes. Missing or contradictory variant-one inputs
reject without running the source RNG, projection or route effects.
Variant-zero `0x57BA98` does not assert that its uninitialized +0x20
field is meaningful.

Both independent original initializer tests now supply the actual
variant-one SQ explicitly. The Unreal C++ test also rejects both a
missing source SQ and a one-byte-mismatched vector, preserving the
previous source update method and game RNG cursor on failure.

The existing `Getchu3OriginalManagerInitializerTests` CTest now
links `original_manager_child.cpp` and calls the actual recovered
`originalManagerChildCtor_57baf8`, whose source-order callback invokes
the real translated `originalManagerInitialize_57bcb8`. The regression
verifies the pre-initializer controller vector, flags, method pair,
source near-owner result and unchanged original RNG cursor. This is a
linked source constructor/initializer path, **not** a complete actor
factory or Unreal Player/Monkey game instance.

## Source spatial selector 0x21D898 / 0x21DA2C exported for Unreal

The native bridge now exports both already source-recovered actor polygon
selectors as C ABI functions, using decoded original-stage 0x70 records and
the original 0x1E polygon descriptor point counts:

- `ae3_original_polygon_nearest_hits_21d898`: consumes the **original
  0x4E3C14 grid candidate order** supplied by a future genuine source
  grid owner; applies translated 0x21B788 XZ polygon eligibility,
  0x21B468 centroid, and 0x21B234 VU0 XYZ distance. Stable strict
  comparison retains the first minimum in the source hit order.
- `ae3_original_polygon_nearest_all_21da2c`: scans the original record
  registry in order with the translated centroid/XYZ distance, without
  the polygon eligibility filter, matching the distinct source function.

Both return source polygon **indices**, never pointers into invocation-local
decoded records. An empty 0x21D898 hit list is a successful query with
`UINT32_MAX` (no eligible polygon). Invalid source candidate indices,
missing records/descriptors, descriptor counts outside 1..3, nonfinite
XYZ queries and reentrant use are rejected without publishing output.
The native session stays guarded until each query is finished.

**Original-system ownership:** the underlying 0x4E3C14 native
grid candidate calculation is source-recovered in
`native/src/original_spatial_support.cpp`. The initial `_hits_21d898`
API still accepts a real owner-provided candidate list. The new
`_grid_4e3c14_21d898` API runs that source grid lookup itself from
original grid fields/cell offsets/hit pool, then invokes the native
0x21D898 VU0 selector; it does not fabricate hits.

The caller must provide original stage geometry, grid resources and
the **actual two scalar grid coordinates** after the original
game-specific VU0 operand extraction. That actor-owned operand
extraction, persistent Player/Monkey manager, target motion, and
Unreal gameplay runtime are NOT provided by this ABI.

`native/tests/original_polygon_nearest_unreal_abi_test.cpp` exercises
original hit-list order versus all-registry order, first-wins distance
ties, rejected candidate planes, empty hit lists, malformed source
indices, descriptor validation, nonfinite queries, W-lane independence,
source 0x4E3C14 cell lookup, two distinct original grid cells, invalid
source offsets, missing cells, and failure without result publication. Its CTest target is
`OriginalPolygonNearest21D89821DA2CHostAbi` in
`native/unreal_bridge/CMakeLists.txt`.

These tests and exports are committed as source only; a successful
Windows DLL build, Unreal Editor integration, and actual ISO-backed
stage operation still require verification.

## Original source-header and complete loaded-stage ABI integration

Two levels of source-backed spatial input are now available to the
native Unreal DLL, in addition to the older caller-supplied polygon
count/record APIs.

### Direct original allocated-header resolution

`ae3_original_polygon_nearest_original_data_all_21da2c` and
`ae3_original_polygon_nearest_original_data_grid_21d898` consume the
**original loaded 0x70-byte spatial records** and actual original
source allocation bytes with their true, owner-provided 32-bit runtime
base addresses. These C functions call the already-recovered
`originalSpatialResolveHeaders_21b468`, which reads the source
`record+0x30` pointer and original `header+0x16` unsigned vertex
count. The original `+0x40..+0x6F` VU0 vectors then feed the
same source-derived 0x4E3C14 / 0x21D898 / 0x21DA2C functions.

Unresolved source pointers, overlapping matching allocations,
malformed counts, incomplete allocation buffers and grid errors
reject before publishing any result. Source addresses are NEVER
dereferenced as native pointers or interpreted as an EE emulator
address space.

The independent native ABI target
`OriginalRawSpatialHeaderGridNearestHostAbi` checks exact
source pointer resolution, original header vertex counts, full
registry tie ordering, native grid eligibility, invalid/ambiguous
source allocations and error atomicity.

### Actual stage-source construction in one DLL invocation

The more complete
`ae3_original_stage_nearest_all_21d46c_21da2c` and
`ae3_original_stage_nearest_grid_21d46c_21d898` entry points
begin with **genuine already-loaded stage bytes and the source
runtime base address**, not host-built spatial records.

They invoke the native original
`originalStageBuildSourceSpatial_21d46c` recovery to resolve all
six source-relative pointers, build genuine 0x70-byte records,
read 0x1E-byte polygon descriptors, fetch source vertex indices
from the real stage vertex table and preserve the source record
header identities. They then run original source VU0 centroid,
plane eligibility, grid (when requested), and nearest selection.

The stage-specific C ABI does **not** invent action type 1/2 actor
objects. Those entries remain pending in the decoded stage, and
game-specific actor management/animation/update logic is still
unfinished. The caller must still provide genuine 0x4E3C14 grid
metadata and actual original packed-coordinate scalar operands
on the grid path; original 0x36EDAC grid allocation, retail
asset loading, and persistent Player/Monkey stage lifetime are
not claimed as complete.

CTest `OriginalLoadedStageSpatialNearestHostAbi` covers
original-stage-source records and vertex tables, grid selection,
empty candidate sets, rejected malformed source descriptors,
invalid indices/metadata, unavailable source base VA and output
atomicity. These are fixture-backed regression tests, NOT a
private retail ISO gameplay verification.

All new bridge methods are registered with
`native/unreal_bridge/CMakeLists.txt` and reuse the existing
native source-recovered components. Neither a Windows/Unreal
build nor in-game Play/Monkey behavior has been verified by
these source commits.

### Stage-owned spatial cache for frame queries

The native `Ae3OriginalManagerInitSession` now also has a strictly
host-owned, persistent `OriginalStageSpatialLoaded` value. A real
actor manager can call:

1. `ae3_original_stage_spatial_mount_21d46c` once at stage entry,
   providing owner-loaded genuine original source bytes and runtime VA.
2. `ae3_original_stage_spatial_nearest_all_21da2c` or
   `ae3_original_stage_spatial_nearest_grid_21d898` repeatedly for
   source actor queries without reallocating/redecoding records.
3. `ae3_original_stage_spatial_unmount` during stage unload, before
   the native manager session itself is destroyed.

Each successful mount decodes the entire actual 0x21D46C spatial
stage and owns the resulting source records and original-header
bindings. The owner may release the borrowed source input bytes
after mount. Invalid replacement mounts leave the previous native
stage intact, not partially replaced, and do not reset or consume
the session's original 0x36F3C0 game RNG state. Queries require a
mounted stage and report source polygon INDEX values only.

This is a persistent **spatial owner**, NOT the source gameplay
Player/Monkey actor allocation/update/destruction manager. The
original `0x57C620` controller update and the original game-owned
actor/animation/spawn chain remain unrecovered or unconnected.

Existing `OriginalLoadedStageSpatialNearestHostAbi` regression
now also covers repeated mounted-stage nearest selection, post-mount
mutation of borrowed stage bytes, failed atomic stage replacement,
explicit unmount and successful subsequent remount. Source files are
committed; actual Windows native CTest and original Unreal runtime
validation remain required.

### Exact stage spatial records and pending actor actions

The mounted `OriginalStageSpatialLoaded` already retained both the
original 0x70-byte stage record array and source `0x21D46C` action
type-1/type-2 descriptors. These were previously inaccessible through
the native DLL, leaving a real Player/Monkey owner unable to consume
the exact source records after stage mount.

Four C ABI exports now connect the retained original data to a future
real actor manager:

- `ae3_original_stage_spatial_record_count` returns source record count.
- `ae3_original_stage_spatial_record_copy` copies precisely the
  recovered 0x70 record bytes plus validated original header VA
  (`record+0x30`) and header `+0x16` point count.
- `ae3_original_stage_spatial_pending_action_count` returns the number
  of source actions identified during 0x1E-byte descriptor traversal.
- `ae3_original_stage_spatial_pending_actions` copies these entries
  in original polygon/slot order: source record index, source action
  slot index, exact original type 1/2, and original argument halfword.

All exported data are **copies**, not transient native pointers.
Reads require a successfully mounted source stage; invalid source
indices, unresolved or mismatched original header identity, output
buffers with insufficient capacity, and unmounted stages fail without
publishing a partial record or action list.

The extended `OriginalLoadedStageSpatialNearestHostAbi` tests
now cover two original action descriptors (type 1 / argument 7 and
type 2 / argument 8), truncated output capacity, exact record header
VA and vertex lane copies, bad indices, and unmount invalidation.

**Scope boundary:** these calls provide necessary original stage data
to the real game manager, but do NOT infer how type-1 or type-2
actions spawn Monkey/Player objects. Original `0x21B0B0` /
`0x21B240` child construction, original `0x57C620` controller
update, animations and the owner lifecycle still need source-accurate
gameplay integration. Nothing here proves Windows native compilation,
original retail ISO execution or gameplay parity.
