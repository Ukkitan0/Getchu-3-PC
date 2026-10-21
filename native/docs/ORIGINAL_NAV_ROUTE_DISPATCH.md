# Ape Escape 3 USA: original top-level NavGraph route dispatcher

Original executable: private owner-provided USA SCUS_975.01 SHA-256
5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63.

This pass reconstructs original compiled game functions using native
C++20; it does not emulate a PS2 or invent replacement gameplay.

## 0x2207D0 route modes

Original source $7 is an optional linked-list owner, NOT a polygon.
`0x22086C` forwards owner $7 and seed polygon $5 into `0x21F89C`,
which reads owner+4 and calls `0x4E3DAC` for exact pointer membership.
Null $7 proceeds directly to normal route processing.

Normal route request behavior after original 0x220AA4 selection,
0x220C00 workspace construction and 0x220D5C cost relaxation:
- Mode 0 invokes 0x221030 and chooses the farthest admitted node.
- Mode 1 invokes 0x220F60 and chooses the nearest admitted node.
- Other modes pass independent original $8 search index to 0x220994
  (-1 means no route). The mode value is held separately in $9.

Fallback behavior when the original membership candidate is absent
from its filter list:
- Original 0x22085C stores incoming $7 owner into context+0x8,
  and 0x220948 reads **the same owner**. Wrapper 0x21F8B8 then
  reads its list and tailcalls 0x4E3DEC.
- 0x4E3DEC traverses actual ordered members and computes their
  polygon centroid via original 0x21B468, then chooses the first
  strictly nearest member to the source query point, with source
  FLT_MAX literal at 0x6E3F00.
- The fallback keeps the original input seed polygon unchanged.
  It disables 0x220AA4 membership filtering, reconstructs and
  relaxes the original route workspace, then selects the nearest
  endpoint to the fallback member's centroid, regardless of the
  normal route mode.

The original 0x220A4C and 0x220A78 each release nonnull workspace
allocation +0 via real game deallocator 0x41A3D8.

## Native implementation

New source:
- native/include/ae3/original_nav_route_dispatch.hpp
- native/src/original_nav_route_dispatch.cpp
- native/tests/original_nav_route_dispatch_test.cpp
- native/tools/verify_original_nav_dispatch.py

The top-level dispatcher is originalNavRouteDispatch_2207d0.
The native source-linked-list selector is originalNavListNearest_4e3dec.

Correct source roles: $5 is the seed polygon, $7 is its optional
membership-list owner, and context+8 retains the **same $7 owner**
for fallback. Prior native code incorrectly interpreted $7 as a
polygon and invented a separately owned fallback membership list. The original 256-node workspace
bound, strict comparisons and native source polygon identity are
preserved by previously recovered methods.

## Original executable and local testing

Private hash-pinned original ELF audit verified four whole instruction
regions covering **198 original MIPS words**, **34 independent opcode
checks**, and **19 direct J/JAL original call targets**.

Native optimized g++ C++20 strict warnings-as-errors test passed;
UndefinedBehaviorSanitizer test passed. The tests cover normal
modes 0, 1, and 2; tie handling; original membership pruning;
independent fallback-list lookup; fallback mode override;
foreign/missing polygon source errors; and both allocator wrappers.

A separate local native executable processed all **290 original USA
retail navigation files**, loading **19,377 original polygons**.
Of those files, **289 nonempty graphs** exercised modes 0, 1
and 2, and **259 graphs** also exercised the fallback branch
using authentic polygon records in the ordered source list.
With a fixed test reference at the first original source vertex
and test radius 10, **75** records were admitted across the
289 nonempty graphs. These are fixed-test parameters, not claims
about actual in-game Monkey AI request positions.

No original private navigation assets were published or included
in the GitHub changes.

## Remaining work

Original runtime circular-list owner creation and lifetime, live Actor
VU query and input bindings, actor path consumption, rendering and
gameplay boot remain incomplete. The native calls require actual
game source lists/reference vectors; none are fabricated.
PS2-specific FPU corner cases are not yet proven bit-exact.

**The full game is not playable natively on Windows.
No GitHub Actions or CI runs were started.**
