# USA original actor movement distance gate and type-2 branch

## Retail instruction provenance

Original owner-private USA `SCUS_975.01` SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

Original full `0x57BCDC–0x57BD54` (31 × 32-bit words) SHA-256:
`c93c29dad7891ab3e2af3a5ad39e9cccc2eebd5d89604e762f043e167dc9fcf5`.
`native/tools/verify_original_actor_distance_gate_57bcdc.py` authenticates
these instructions and the original threshold from the **private** executable.

This is direct source translation, not replacement physics.

| Original VA | Recovered source behavior |
|---|---|
| 0x57BCDC / 57BCE0 | LBU controller+0x30; if zero, skip distance math |
| 0x57BCEC | LQC2 VF1, controller+0x20 (all four original float lanes) |
| 0x57BCF0 / 57BCF4 | Follow owner+0x14 and LQC2 VF2, owner+0x40 |
| 0x57BCF8 | VSUB.xyzw VF1,VF1,VF2 with full 0xF lane mask |
| 0x57BCFC / 57BD04 | QMFC2 the source vector as both arguments to 0x4748AC |
| 0x57BD00 | JAL original 0x4748AC: (X²+Y²)+Z²; W omitted |
| 0x57BD10 | SQRT.S original length F0 |
| 0x57BD1C / 57BD20 | Load original global 0x708A20 (float32 bits 0x40000000, 2.0f) and strict C.OLT.S |
| 0x57BD24 / 57BD34 | Source flag select; only a distance **strictly below** 2.0f triggers reset |
| 0x57BD3C–57BD54 | True branch clears controller+0x08 and +0x0C using original zero stores |

**Important:** The source 0x57BCF8 full vector subtraction happens before
original `0x4748AC` XYZ-only dot, so W subtraction takes place but W never
contributes to the distance. The 2.0f comparison is strict, not `<=`.
Non-finite/NAN comparisons cannot trigger the true gate. The source
variant-disabled path never executes the original VU0 math: its native result
exposes `std::nullopt` instead of inventing a distance.

## Connected original source type-2 branch

The VU0-owned `originalActorType2MovementBridge_57bcb8` recovers only
one **fully source-proven** path of parent actor control:
1. Evaluate original 0x57BCDC gate. If true, clear controller+0x08/0x0C
   and return, leaving original target+0x50, controller+0x40 and RNG untouched.
2. When the gate does not fire, original controller+0x1C must indicate a
   populated route (original 0x57BD80).
3. Source type read must be precisely action-kind 2 (original
   0x57BDA4/57BDA8), otherwise **return pending/unrecovered** without
   fabricating actor movement.
4. Existing navigation-owned `originalActorType2Target_57bdb8` chooses
   genuine route[1], reads the actual three 16-byte `0x21B458` vertices.
5. Original `0x57BDC8` writes the record identity to controller+0x50,
   and original `0x57BDCC–0x57BED4` math consumes the two authentic
   game RNG draws and performs full 16-byte `SQC2` into controller+0x40.

The bridge does **not** claim to reproduce the other original actor
branches (`0x57BED8`, `0x57BF54`, other action kinds, parent
`0x57BFB0` etc.). It deliberately returns
`OriginalOtherBranchUnrecovered` and does not invent their side effects.

## Files and integrator-only CMake requirements

New files:
- `native/include/ae3/original_actor_distance_gate.hpp`
- `native/src/original_actor_distance_gate.cpp`
- `native/tests/original_actor_distance_gate_test.cpp`
- `native/include/ae3/original_actor_type2_movement_bridge.hpp`
- `native/src/original_actor_type2_movement_bridge.cpp`
- `native/tests/original_actor_type2_movement_bridge_test.cpp`
- `native/tools/verify_original_actor_distance_gate_57bcdc.py`

The **Windows-runtime integration owner** must add
`original_actor_distance_gate.cpp` and
`original_actor_type2_movement_bridge.cpp` to the existing native game
target and register the two focused test targets, linking the already
recovered original `original_actor_ray_geometry.cpp`,
`original_actor_type2_vu0_target.cpp`,
`original_actor_random_triangle.cpp`, `original_game_random.cpp`,
and **unmodified navigation-owned**
`original_actor_type2_target.cpp` plus original spatial index accessors.
No changes were made to native/CMakeLists.txt or the navigation workstream.

The actual actor engine must supply the live 16-byte source
controller+0x20 vector, the owner(+0x04)->(+0x14)->(+0x40) vector,
the genuine action-kind, route, original global RNG, controller
`+0x08/+0x0C` storage, `+0x40` storage and `+0x50` target pointer.
Do not substitute guessed actor state.

## Executed tests and remaining limits

Owner-private `woods_c/navigation.pck` SHA-256:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

Focused native GCC optimized and Clang AddressSanitizer/UBSan tests
passed against 59 original navigation polygons and all 15 original
kind-two links. The gate test checks all 59 polygons × 4 source
vector combinations, including source W independence, exact
threshold equality, `nextafter(2,0)` and `nextafter(2,3)`,
disabled-variant behavior and NaN. Connected test checks:
distance-gated early exit without RNG draws or target change;
non-type-two pending/no mutation; variant-enabled beyond-2.0
type-two continuation; and variant-disabled type-two dispatch,
including the original full 16-byte target, selected record identity,
and correct evolving MT19937 cursor.

Commands were run with `-std=c++20 -O2 -Wall -Wextra -Werror
-ffp-contract=off -fno-fast-math` for GCC; for Clang
`-O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined
-fno-omit-frame-pointer -ffp-contract=off -fno-fast-math`.

These are **focused native source/module tests**, not a whole-repository
build or Windows gameplay. No original PS2 VU hardware trace, full
Windows application boot, or successful full native integration is
claimed. PS2 denormal/FTZ and unusual floating-point behavior may
differ from ordinary host IEEE float32; original action branches
beyond this connected scope remain unfinished.

No game binary, proprietary assets, ISO, PCK or Lua are committed.
No workflows were dispatched.
