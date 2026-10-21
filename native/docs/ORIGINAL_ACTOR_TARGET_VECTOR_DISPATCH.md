# Ape Escape 3 USA — original actor target-vector dispatch 0x57C8EC

The authentic owner-private USA `SCUS_975.01` has SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Private authentic `woods_c/navigation.pck` SHA-256 is
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.
Neither retail binary nor game assets are committed to GitHub.

## Original function and branches

The actor's route follower at `0x57C7CC` calls `0x57C8EC` to construct
a target motion vector for a specific original polygon. The complete
`0x57C8EC..0x57CA64` code region contains **95 original instruction
words** and is SHA-pinned in the verifier.

At `0x57C918` the source reads the controller's byte **+0x30**.

**Variant nonzero**: original `0x57C930` calls original `0x21B788`
with FPU `$f12` loaded from retail ELF location **0x708A60**,
whose exact raw bits are `0x3DCCCCCD` (approximately 0.1).
Original `0x57C93C` branches on the predicate.

- Predicate **true**: source `0x57C944` executes `LQC2 VF1,+0x20(controller)`
  and `0x57C948` executes `DMFC2 $v0,VF1`. This return is
  **exactly the low 64 bits of the original 16-byte vector at controller+0x20**,
  interpreted as bits, not a newly computed float value.
- Predicate **false**: source `0x57C97C` calls original `0x57CA68` to
  calculate geometry, then transfers its packed result to VF1 and returns.

**Variant zero**: source `0x57C98C`, `0x57C99C` and `0x57C9B0`
call original vertex accessor `0x21B458` for source polygon vector slots
0, 1, 2. Their original FPU/VU0 transform at `0x57C9BC..0x57CA60`
then generates the packed result.

## Native C++ implementation

`originalActorTargetVector_57c8ec` now reconstructs that three-way
dispatch using source fields and the exact original F12 constant.
The true-predicate branch is **fully implemented as native low64
vector passthrough**. The variant-zero branch retrieves all three
original source 16-byte vertices by calling the existing recovered
`originalSpatialVector_21b458`.

The two nontrivial VU0 transformations remain **explicit, original-
specific callback dependencies**, not fabricated steering formulas.
A callback must return the original packed output; test callbacks
use fixed values only to test branching and are not a native movement
implementation. The geometry eligibility callback must implement
original `0x21B788` to be used in the actual game.

`native/tests/original_actor_target_dispatch_test.cpp` exercises all
three paths on **all 59 actual woods_c polygons**, checking authentic
vertex bytes, original polygon identity and the precise bit pattern
in the fast-path output. Local GCC and Clang Release and ASan/UBSan
tests passed with the original PCK.

`native/tools/verify_original_actor_target_dispatch.py` checks
95 source instructions, 18 opcode anchors (including original
0x708A60), seven original calls and four branch targets.

This continues recovery of original actor motion, **not a game rewrite**.
The remaining transforms `0x57C98C` and `0x57CA68`, plus actor state
updates, must still be reverse engineered for a working Windows game.
No emulator or GitHub workflow dispatch was used.
