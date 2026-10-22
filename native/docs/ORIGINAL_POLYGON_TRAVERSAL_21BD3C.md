# AE3 USA original 0x21BD3C recursive polygon traversal (native engine)

The original source `SCUS_975.01` SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
and owner-private `woods_c/navigation.pck` SHA-256
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`
were used to reconstruct the real source traversal.
Neither file is included in the public repository.

## Newly recovered source behavior

Original `0x21BD3C..0x21BEA4` (91 instruction words)
is called by the actor projection at `0x4C1BA8`. The new native
`originalPolygonTraverse_21bd3c` implements:

1. Original source `0x21BD74/78` obtains the first three **original**
   polygon vertices from record offsets `+0x40`, `+0x50`, `+0x60`.
   The native input uses actual `OriginalSpatialRecord70` objects
   produced by the already ported source stage loader.
2. Original `0x21BD7C/90` initializes shared intersection distance
   slots `packet+0x20=0` and `packet+0x24=$f12`.
3. `0x21BD8C`, `0x21BDA0`, `0x21BDB4` invoke original
   `0x21EE00` sequentially with three directed triangle edges
   **(vertex0,vertex1), (vertex1,vertex2), (vertex2,vertex0)**.
   Each call must succeed before the next; all calls mutate the same
   original bounds packet. This exact dispatch is implemented.
4. `0x21BDD8` subtracts the accepted intersection distance
   from the remaining segment; `0x21BDE0/21BDE8` use original
   VU0 `VMULx` and `VADD` to advance the ray position by
   `original_direction * distance`. All **four** vector lanes
   are preserved.
5. `0x21BDF4` stores the accepted polygon identity before recursion.
   `0x21BDFC` / `0x21BE00` distinguish exhausted distance
   from continuing the remaining ray.
6. `0x21BE30/48/60` use the original `0x21B3F8` edge count,
   `0x21B444` action kind and `0x21B404` source neighbor.
   Original action kind **nonzero** is skipped. Only **kind zero**
   neighbors are visited in their original order, recursively calling
   **the same** `0x21BD3C` routine at `0x21BE78`.
   The first nonempty recursive result returns immediately.

Native code adds explicit source index checks and a configurable
max-recursion safety budget. Those are host-safety measures only;
they do not invent extra original geometry or movement behavior.

## Important incomplete dependency

**The original edge-plane intersection routine `0x21EE00` is still
untranslated.** It depends on original VU0 math, calls
`0x4748AC`, and has a retail threshold
`0x6E3EF8=0x358637BD` (approximately `1e-6`).

The new `OriginalPolygonEdgeIntersection_21ee00` interface must
eventually be supplied with source-correct native intersection math.
Regression callbacks deliberately substitute deterministic intersection
RESULTS to test **the original traversal control flow only**;
they are **not** real collision and must never be used as gameplay.
The source 0x4C1B3C post-intersection return path also remains incomplete.

## Verification

`native/tools/verify_original_polygon_traversal.py` SHA-verifies
the original **91 retail instructions**, seven JAL edges,
eleven original branches, and eight independent opcode anchors.
It reads private owner-owned inputs and emits metadata only.

`native/tests/original_polygon_traversal_test.cpp`
exercises exact three-edge short-circuit ordering; source floating
distance/position updates; type-zero recursion; skipping kind-one/two
links; explicit recursion guard; and source data bounds. Its private
retail-data mode decodes **both real woods_c navigation graphs**:
39+20 polygons, 95+42 edges, 15 original type-two links, and confirms
source vertices/links select the original polygons.

GCC Release, Clang Release, UBSan and ASan+UBSan tests passed locally
using owner-private authentic `woods_c/navigation.pck`.

The Windows runtime, actual 0x21EE00 edge intersection, source actor
animation and engine boot remain unfinished. This commit does not add
emulation or an invented replacement gameplay engine.
No GitHub Actions were manually triggered.
