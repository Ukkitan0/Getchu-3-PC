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
