# Correct original AE3 USA VU0 navigation plane normal (0x21B788)

## Why this fixes a real native-port blocker

The previous native `originalVU0PolygonEligible_21b788` used
`normal=(edge.y,0,-edge.z)`, which was an incorrect interpretation
of original source `0x21B7D0..0x21B7E4` MMI shuffle and VU0
cross product. Genuine `woods_c` navigation polygons are in
the original **X/Z** world plane, with **Y** as vertical/up.

The source at `0x21B7C0` builds `VF1=(0,1,0,0)`.
After the packed original EE register shuffle at
`0x21B7D0..0x21B7DC`, the `VOPMULA/VOPMSUB` operations
at `0x21B7E0..0x21B7E4` implement the source inward horizontal
normal equivalent to **up CROSS (current - previous)**:

```text
edge = current - previous
normal.x = edge.z
normal.y = 0
normal.z = -edge.x

outside = (dot(normal,query) < dot(normal,previous) - f12)
```

The native routine now uses the source-correct X/Z components,
retaining the original directed polygon winding and f12 margin.

## Evidence from actual retail binaries and polygons

User-owned original US `SCUS_975.01` SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original `woods_c/navigation.pck` SHA-256:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

`native/tools/verify_original_vu0_retail_planes.py`
SHA-pins 51 original machine-code instructions and 13 independent
original opcodes from `0x21B788..0x21B850`.

It evaluates the actual decoded source polygon data:
- **59 / 59** real original triangle centroids satisfy every
  source X/Z inward half-plane.
- **177 / 177** points placed outside the 177 directed source
  polygon edges are rejected.
- The old wrong `edge.y,-edge.z` normal accepted **0 / 59**
  original polygon centroids, so it was objectively incorrect.

This changes more than an isolated helper: the source geometry
selector `0x21D898` and the actor target-vector eligibility
branch `0x57C930` both use `0x21B788` and now receive a
plane predicate that works with authentic retail asset coordinates.

## Native tests

`native/tests/original_vu0_geometry_test.cpp` now uses an X/Z
synthetic polygon rather than an XY ring whose geometry was
inconsistent with the actual game's coordinate axes.

New `native/tests/original_vu0_retail_planes_test.cpp`
exercises the original predicate with genuine `woods_c` vertex
buffers, including all 59 interior tests, all 177 edge exterior
probes, tests changing only Y altitude, and original nonzero
f12 tolerance. GCC Release, Clang Release and ASan+UBSan passed
locally using authentic user-private PCK data.

This fixes an incorrect earlier recovery without introducing
emulation, replacement collision, synthetic gameplay behavior,
or original copyrighted binary data.

**Important**: source routine `0x21EE00` (segment/polygon
intersection) and `0x57CA68` randomized VU0 fallback remain
untranslated. Full original native Windows game still does not boot.
