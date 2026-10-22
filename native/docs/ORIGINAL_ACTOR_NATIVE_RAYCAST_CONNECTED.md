# Ape Escape 3 USA original actor raycast: connected native 0x4C1B3C call path

This change composes three previously isolated recovered source
engine functions into an original actor raycast operation:

```text
0x4C1B3C source ray normalization -> native originalActorProjectionRay_4c1b3c
0x4C1BA8 JAL 0x21BD3C -> native originalPolygonTraverse_21bd3c
  original 0x21BD8C / 0x21BDA0 / 0x21BDB4 JAL 0x21EE00
    -> native originalPolygonClipEdge_21ee00
       -> native originalVU0DotXYZ_4748ac
  source 0x21BE30..0x21BE98 recursively follows original
  source adjacency headers, accepting only original kind-zero links
```

The owner-private original USA executable is SHA-pinned as
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The original `woods_c/navigation.pck` is SHA-pinned as
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

## Original actor input distinction

At original `0x57C7CC`, the current actor controller receives a
packed vector in its original **incoming second argument ($a1)**.
Original `0x57C810` then loads an independent packed vector from
**controller+0x40 into $a2** before `0x57C814` calls `0x4C1B3C`.
These are not the controller+0x20 vector used later by
`0x21B788` eligibility in actor target selection.

The native `originalPolygonRaycast_4c1b3c` accordingly requires
**both original endpoint vectors explicitly**. It never makes up
those operands or silently replaces one with controller+0x20.

The source original `0x4C1B4C..0x4C1BA4` subtracts the two vectors,
calculates XYZ distance with a VU0 dot/sqrt, and normalizes all
four components. It calls `0x21BD3C` with that normalized ray
and original source length. This composable native function
calls the exact already recovered source routines with original
stage record and adjacency spans.

## Local test evidence

`native/tests/original_polygon_raycast_connected_test.cpp`
tests original source endpoint and polygon identity across
two adjacent triangles with the original kind-zero route,
confirms kind-two blocks traversal, then uses **every one of
the 59 original woods_c polygon vertex triangles** and source
adjacency lists. Each genuine interior segment is verified
through the entire native calculation:

original start/end -> original 0x4C1B3C arithmetic ->
original 0x21BD3C traversal -> original 0x21EE00 plane clip ->
original 0x4748AC dot.

GCC Release and AddressSanitizer+UndefinedBehaviorSanitizer passed
on the owner-private source navigation PCK.

## Remaining native port boundaries

The full original `0x4C1B3C` MMI return-value packing
and original actor state update integration still need
verification. The new function returns typed native results,
not fabricated 32-bit guest pointers. Source `0x57CA68`
randomized actor fallback and full game renderer/animation/
original game loop remain unported. No native Windows game
boot has been achieved.

No original copyrighted game assets were committed.
No emulation or replacement gameplay engine was added.
No GitHub Actions were manually triggered.
