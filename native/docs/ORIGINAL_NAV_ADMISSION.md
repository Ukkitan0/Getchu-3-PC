# Original Ape Escape 3 — route workspace selection (0x220AA4)

Source: owner-private USA retail SCUS_975.01,
SHA-256 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d.
Only reconstructed C++ source and opcode/address metadata are committed.

## Why this matters

The previous native original NavGraph search at 0x220D5C ran on a
workspace containing all polygons from a retail .nav file. The original
game does **not** always do that: 0x2207D0 builds a bounded subset
by calling 0x220AA4 before invoking 0x220C00/0x220D5C.

The source-verified native code now constructs that same style of
DFS-admitted workspace from actual original navigation records.

## Recovered original methods and control flow

- 0x2207D0: allocate exactly 0x3800 bytes for 0x38-byte
  navigation-search nodes (256 records); initialize visited byte+0,
  link count+8, predecessor+0x30=-1, and float cost+0x34=FLT_MAX.
  Dispatch 0x220AA4 admission, 0x220C00 adjacency mapping and
  0x220D5C relaxation, then choose endpoints through additional modes.
- 0x220AA4: on the outer call, begin a new polygon visit epoch.
  Call 0x21B014 to test the current epoch's existing visit stamp.
  Optionally require membership through 0x21F89C/0x4E3DAC when
  original graph+8 and the caller's filter flag are both set.
  Compute original centroid via 0x21B468, then compare XYZ distance
  against the original float F12 radius using **strict less-than**.
  Only accepted polygons get a 0x38 workspace record and a
  0x21B038 mark. Recurse through original links returned by
  0x21B3F8 and 0x21B404, preserving source DFS order.
- 0x21AFB8: move the original polygon registry to a new visit
  epoch. The unusual 32-bit wrapping path clears offset +0x28
  markers in all records rather than preserving a stale visit state.
- 0x21B014: lazily synchronize each polygon's cached epoch with
  its current owner's epoch, clearing its visited byte when old.
  Return whether the polygon is currently marked.
- 0x21B038: store the accepted visited marker.
- 0x4E3DAC: test whether a source polygon pointer appears in an
  original linked-list membership set. The wrapper at 0x21F89C
  supplies that list to the predicate.
- 0x220C00: populate the 56-byte route node's source-linked
  destination indices, edge types and original source edge slot
  **only for neighbors that were admitted**. Missing workspace
  neighbors are skipped, as in the source original 0x220CBC check.

The new C++ files:
- native/include/ae3/original_nav_route_admission.hpp
- native/src/original_nav_route_admission.cpp
- native/tests/original_nav_route_admission_test.cpp

An explicit, iterative host DFS preserves original recursive pre-order
without risking a stack overflow. Actual source polygon vertices and
adjacency descriptors are required; no synthetic game geometry is
created in production. Test fixtures are only for regression tests.
The original 256-record capacity is checked and exceeded inputs
fail visibly instead of overwriting a host allocation.

## Retail validation

The complete USA NavGraph dataset contains 290 original .nav files
and 19,377 original polygons, previously extracted privately from
the owner-provided USA retail ISO.

The new native source-admission and shortest-cost relaxation were run
on every one of the 290 genuine source files with radius 10 game-world
units around the centroid of each first polygon. Results:
- 290 retail files processed; 19,377 real spatial records built.
- 1 graph was empty and was appropriately skipped.
- Across the remaining sources 325 records were admitted and 779
  candidate visits attempted. These counts depend on **this particular
  fixed test radius**, not on actual monkey navigation requests.
- Zero fixed-allocation overflows or malformed-link errors.

The strict g++ C++20 optimized unit test, UndefinedBehaviorSanitizer
regression test, and retail 290-source-file survey all passed locally.

The original owner-private ELF audit validates four entire instruction
windows totaling **250 source machine words**, **47 independent
opcodes**, **21 genuine J/JAL targets**, and original FLT_MAX.

Run the private source audit with:

  python native/tools/verify_original_nav_admission.py SCUS_975.01

## Explicit remaining limitations

This is a faithful native reconstruction of the documented bounded
0x220AA4 admission + 0x220C00 source adjacency mapping, not
proof that all of the original top-level 0x2207D0 is ported.
The game also follows a fallback branch through original 0x21F8B8
and supports endpoint-selection mode variants. The convenience API
originalNavRouteFromSource_2207d0 currently assembles only a
nearest-to-query result and must not be treated as a full translation
of those branches.

The native runtime must still establish live source pointer ownership,
original actor route consumption, animation and movement loops,
input, graphics and stage gameplay. No retail Windows game was
booted and GitHub Actions were not dispatched.
