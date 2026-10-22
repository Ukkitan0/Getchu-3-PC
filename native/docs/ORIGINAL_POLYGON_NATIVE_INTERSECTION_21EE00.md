# Ape Escape 3 USA — FULL native original edge clip + polygon traversal

## Original game executable and asset provenance

User-private USA `SCUS_975.01` source SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original `woods_c/navigation.pck` SHA-256:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

**This pass removes the last geometry callback in the low-level
original segment-to-polygon traversal path.** It does not add a
replacement game engine, PS2 emulator, synthetic pathfinding,
or copyrighted original assets.

## 0x21EE00 — native exact source edge-plane clipping

Source instruction words `0x21EE00..0x21EF38` (79 words).

The original game receives two packed original polygon vertices,
a ray origin and direction, and shared source float packet slots
`+0x20=lower`, `+0x24=upper`. Native
`originalPolygonClipEdge_21ee00` reconstructs:

1. `0x21EE10..0x21EE5C`: original up-axis X/Z half-plane
   normal constructed using the source VU0 edge subtraction and
   VOPMULA/VOPMSUB. In actual source units it equals
   `(edge.z,0,-edge.x)`.
2. `0x21EE68`: original `0x4748AC` XYZ normal/ray
   dot product — reuses `originalVU0DotXYZ_4748ac`
   from the earlier original actor ray-geometry recovery.
3. `0x21EE78..0x21EE88`: original ABS.S, C.OLT.S and
   condition on the genuine float32 source epsilon
   `0x6E3EF8=0x358637BD` (approximately **1e-6**).
4. For near-parallel rays, original calls at
   `0x21EE90` and `0x21EEA0` measure source edge
   and ray dot products. Original `C.OLE.S` allows rays
   on or inside the directed half-plane, rejects exterior.
5. Otherwise original `0x21EEDC`, `0x21EEEC`
   calculate the edge/ray dot products and original
   `SUB.S` / `DIV.S` determine distance fraction.
6. Original signed `C.OLT.S 0,denom` chooses:
   positive -> `MAX.S` of lower bound at packet+0x20,
   negative -> `MIN.S` of upper bound at packet+0x24.
   The original `0x21EF2C` requires **lower strictly
   less than upper**. We preserve this distinction.

No guessed radial thresholds, invented clipping planes or
new collision behavior. This is the original source
Liang–Barsky-style half-plane segment clipping, implemented
as native float32 and explicit source structs.

## Original actor navigation now has a connected source path

```text
0x4C1B3C  original actor ray preparation
    -> 0x21BD3C  original triangle and neighbor recursion
       -> 0x21EE00  original edge half-plane clip (3 calls)
          -> 0x4748AC  original native XYZ dot (up to 5 calls)
       -> original 0x21B3F8 / 0x21B444 / 0x21B404 neighbor routing
```

Original `0x21BD3C` was reconstructed previously;
`OriginalPolygonNativeEdgeClip_21ee00` now plugs the
true native clipping arithmetic into its original callback
interface. Source polygon headers and 0x70-byte record
vertices are decoded from original game assets.

Native host pointer identity is used instead of MIPS guest
pointers; original global output values are preserved as
source-typed records. Tiny float precision differences are
possible for NaNs or denormals between EE VU0 and host IEEE
FPU; bit-level differential runtime EE verification is
still pending.

## Verified original game data and native tests

`native/tests/original_polygon_native_raycast_test.cpp`
tests:
- Directed source edge positive/negative ray
  `MAX.S/MIN.S` bounds behavior.
- Source parallel exterior rejection / interior acceptance.
- An actual ray traversing a two-triangle mesh by a
  source kind-zero link; the same source link with
  kind two is not traversed.
- **59/59** authentic `woods_c` polygon interior rays;
  all source target positions remain within tolerance.
- **177/177** authentic directed-edge exterior rays
  are rejected by the source intersection math.
- Both authentic 39+20 polygon navigation graphs,
  with original adjacency and action-kind arrays.

Ran GCC, Clang, ASan+UBSan and standalone UBSan
against owner-private original PCK locally.

`native/tools/verify_original_polygon_native_clip.py`
SHA-verifies the full 79-instruction retail code body,
16 independent source opcode/constant anchors,
all five original calls to `0x4748AC`, and the key
original source branches.

## Remaining game-port blockers

The lower-level source triangle clipping and traversal
is now implemented. The complete caller `0x4C1B3C`
still has final output handling to connect to actor target
selection. `0x57CA68` randomized fallback and the wider
actor update/animation/render runtime are incomplete.
The game has not yet been built or booted as a complete
original native Windows executable.

No GitHub Actions have been manually triggered.
