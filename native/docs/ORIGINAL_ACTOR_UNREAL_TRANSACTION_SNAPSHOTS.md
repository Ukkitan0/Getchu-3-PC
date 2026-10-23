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

**Original-system dependency:** the underlying 0x4E3C14 grid candidate
generation and owner runtime are not fully recovered; the ABI does not
fabricate hits, stage geometry, player/monkey creation, target motion,
or Unreal navigation. Host owners MUST supply real decoded original
navigation/record bytes. These native exports alone do not create actors.

`native/tests/original_polygon_nearest_unreal_abi_test.cpp` exercises
original grid order versus all-registry order, first-wins distance ties,
rejected candidate planes, empty hit lists, malformed source indices,
descriptor validation, nonfinite queries, W-lane independence, and the
result-publication failure boundary. Its CTest target is
`OriginalPolygonNearest21D89821DA2CHostAbi` in
`native/unreal_bridge/CMakeLists.txt`.

These tests and exports are committed as source only; a successful
Windows DLL build, Unreal Editor integration, and actual ISO-backed
stage operation still require verification.
