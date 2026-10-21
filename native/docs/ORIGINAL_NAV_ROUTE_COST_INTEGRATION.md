# Original Ape Escape 3 USA — native NavGraph route-cost integration

## Source and intent

Source executable: owner-private US retail `SCUS_975.01` (SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`).
Optional real-stage test: private original `woods_c/navigation.pck`
(SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`).
Neither original file is included in this repository.

**Avoid duplicating already recovered functions.** The repository already
contained real source-derived math in `original_vu0_geometry.cpp`:

- `originalVU0PolylineLength_21b194`: original 0x21B194 consecutive
  16-byte VU0 vector distance reduction.
- `originalVU0DistanceXYZ_21b234`: original squared XYZ differences,
  VADDy/VADDz component sum and scalar SQRT.S.
- `originalVU0Mean_21b468`: original polygon centroid with 1/count.

What was missing was **the original call path** connecting the recently
recovered 0x21D70C type-1 *allocated child* and original navigation
path relaxation at 0x220F18/0x220E8C. This change supplies that bridge
instead of implementing another distance formula, replacing the engine,
or introducing a PS2 interpreter.

## Retail instruction and control-flow recovery

Original 0x21B178 reads the type-1 child's 16-byte vector count, and
0x21B180 indexes its allocated 16-byte vectors. Original 0x21B194
loops over consecutive vectors, returns exactly zero for count 0 or 1,
and for each pair performs:

```text
LQ/SQ first vector, QMTC2 second vector, LQC2 first vector
VSUB.xyzw -> VMUL.xyzw -> VADDy.x -> VADDz.x
QMFC2 x -> MTC1 -> SQRT.S -> ADD.S to accumulated length
```

The W difference is squared by VSUB/VMUL but omitted from the X
reduction: **route length is XYZ-only**. The new typed adapter converts
original raw child-vector bytes to existing `OriginalVU0Vec4` values,
then calls `originalVU0PolylineLength_21b194` without duplicating
the VU0 algorithm.

The retail route traversal at 0x220E74 branches to 0x220F18 for a
type-1 action. It uses `0x21B4E0` to retrieve the child pointer from
original record+0x04+4*actionIndex, then calls `0x21B194` at 0x220F28.
The shared path at 0x220E8C loads the source and destination route
scores at record+0x34, computes `candidate = sourceScore + edgeCost`,
and updates destination+0x34 plus its predecessor ID at +0x30 **only
when candidate is strictly less** than the current score.

For direct-distance traversal, 0x220EC8 calls original 0x21B468,
receives the original polygon centroid, calculates XYZ distance to the
query via the VU0 instructions and enters the **same** shared relaxation.
This pass exposes both original cost paths and strict-update behavior.
It does not reconstruct the entire outer graph search or game AI.

## Native entry points

`native/include/ae3/original_nav_type1_route.hpp` provides:
- `originalNavType1ChildDistance_21b194`: source-child bytes to the
  already implemented VU0 polyline math.
- `originalNavType1ActionDistance_220f28`: original action-index lookup
  in a decoded `OriginalStageSpatialLoaded` child-owner collection.
- `OriginalNavRouteCost_220e8c`: semantic values for the original
  0x38-byte search record's +0x30 predecessor and +0x34 score.
- `originalNavRelaxCandidate_220e8c`: original strict route update.
- `originalNavType1Relax_220f18`: type-1 child cost into that update.
- `originalNavCentroidRelax_220ec8`: polygon centroid/distance into
  that same update using the *existing* VU0 implementation.

The public native navgraph-loader header exposes these calls so future
original AI/navigation recovery can invoke them. Type-1 children remain
host-owned by the 0x21ED30 stage owner; this does NOT fabricate 32-bit
guest child heap VAs or replace original record pointer fields.

The score representation is native and typed, not binary-compatible
with the full original 0x38-byte search-node allocation. The original
outer search and its heap/actor ownership are separate missing steps.

## Verified evidence and tests

`verify_original_nav_type1_distance.py` SHA-pins **eight complete
retail instruction regions totaling 126 instruction words**,
checks **66 independently selected exact opcodes**, **six direct JAL
edges**, **eight branch destinations**, ten original VU0 operation
decodes including VSUB, VMUL, VADDy, VADDz and VADD/VMULx, the
literal reciprocal numerator 1.0f, the native source vector/count
accessors, and the original score/predecessor store offsets.

`original_nav_type1_route_test.cpp` tests zero-length/one-vertex
paths, a 5+12+13 route, W-lane independence, materialization of type-1
children from a source-format header and index list, indexed child
lookup, absent-child errors, strict less-than/equal/no-update behavior,
NaN safety, shared polygon centroid cost, and **20,000 randomized
source-format routes** against an independent double-precision oracle.

When `AE3_RETAIL_WOODS_C_NAVIGATION_PCK` is set to a privately owned
real original file, an extra regression traverses all 59 genuine
original stage polygons and the existing native 0x21B468 centroid
function. The original woods_c asset has **zero type-1 actions** and
**15 type-2 actions**; therefore positive type-1 route tests necessarily
use controlled source-format vectors and do not claim a full differential
against a live retail PS2 route. More stage assets are needed for that.

Local GCC Release, Clang Release, ASan+UBSan and UBSan passed.
The isolated builds use only the already committed VU0 core functions
mirrored in a minimal local test linkage, not the complete repository
game build. The private ELF verifier and all 59 polygon tests passed
with the original owner-provided files. **Full native Windows game
build and runtime boot are not tested and the game is not playable.**

No proprietary executable or archive bytes are committed. No GitHub
Actions runs were manually requested.
