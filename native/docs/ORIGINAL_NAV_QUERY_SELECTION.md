# Ape Escape 3 USA — original navigation radius query and route extraction

**Source-backed native code** from the owner's original USA retail
`SCUS_975.01` executable (SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`)
and privately owned original `woods_c/navigation.pck` (SHA-256
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`).
No original game binaries or commercial asset data are committed.

## 0x2207D4 — query workspace and setup

The original entry allocates **0x3800 bytes** for 256
**0x38-byte navigation search nodes**, initializes each entry's
visited flag to zero, distance to the original FLT_MAX sentinel, and
predecessor to -1. It calls the original spatial-selection function
`0x220AA4`, then `0x220C00` to build the filtered adjacency and
`0x220D5C` to find the shortest routes.

The source constructor has multiple post-search modes that branch
to `0x220F60` and `0x221030`. Those output consumers are **not**
reconstructed by this pass.

## 0x220AA4 — original nearby source polygon selection

The source function starts with an original spatial record pointer
and 16-byte query position in registers. On its first call it uses
`0x21AFB8` for the original record-generation marker; `0x21B014`
checks whether a record has already been marked. When the original
optional spatial-eligibility mode is active, source `0x21F89C` may
reject a record.

The function obtains the record's polygon centroid from the original
`0x21B468` routine, computes XYZ distance using VU0 and
`SQRT.S`, and appends the spatial-record pointer to the query array
only for **distance strictly less than the requested radius**
(`0x220B5C..0x220B60`). It marks accepted records through
`0x21B038` and descends recursively into polygon adjacency in
source order via `0x21B3F8` and `0x21B404`.

`originalNavSelectByRadius_220aa4` translates that traversal to a
native iterative DFS, avoiding host stack overflow. It uses the already
recovered original centroid and XYZ distance functions. Source
record IDs stand in for retail 32-bit guest pointers; it does not
manufacture memory addresses. The source's unchecked fixed allocation
is represented by an explicit **256-node capacity guard**.

When the optional `0x21F89C` gate is requested, the native entry
requires a caller-supplied original eligibility predicate and
otherwise refuses to silently treat that missing engine operation
as successful. Source record generation-counter and reset behavior
is modeled as a per-query visited set, not a bit-exact 32-bit guest
memory epoch.

## 0x220994 — original path output

The original operation follows the search records' predecessor words
at **+0x30**, counts the records along the route, allocates one
32-bit slot per entry, and writes each **spatial-record pointer**
from the start to the destination (not a reversed list). Native
`originalNavExtractPath_220994` produces the corresponding
**source record indices**, with host rejection of invalid or cyclical
predecessor chains. The original -1 no-destination marker returns
an empty result.

`originalNavQueryAndSearch_2207d4` composes:
`originalNavSelectByRadius_220aa4` ->
`originalNavBuildSearchNodes_220c00` ->
`originalNavExecuteSearch_220d5c`.
The native public NavGraph header now exposes these functions for
future original actor navigation integration.

## Verification — original assets and actual path outputs

`verify_original_nav_query_selection.py` SHA-pins **four original
instruction regions totaling 251 instruction words**:
`0x2207D4..0x220924`, `0x220994..0x220A4C`,
`0x220AA4..0x220C00`, and `0x21AFB8..0x21B040`.
It verifies **45 exact opcode anchors**, **13 original JAL call
edges**, **12 branch destinations**, the COP1 strict-distance
branch, source capacity, source path predecessor fields, the
original query marker access, and the authentic PCK graph sizes.

The new native regression
`original_nav_query_selection_test.cpp` checks:
- Full native query selection and shortest-path traversal on
  both original `woods_c` graph assets (39 monkey, 20 enemy).
- Forward route extraction to **every original source polygon**,
  validating that the resulting path begins at the query source
  and ends at the selected destination.
- Radius cutoffs 0, 1, 30, 100, 500, 10,000 using a separately
  constructed source adjacency DFS oracle for every original
  source polygon membership.
- Optional spatial eligibility, missing eligibility rejection,
  unreachable destinations, no-result markers, and cycles.

GCC optimized C++20 Release, Clang Release, AddressSanitizer+UBSan
and standalone UBSan passed locally on the actual original
`navigation.pck`. Exact source verification passed on both SHA-
pinned original private files. This isolated test links the
previously recovered native geometry functions; it is **not**
a full game/Windows build.

The synthetic regression is part of standard CTest. The real-
retail test runs only with the user's
`AE3_RETAIL_WOODS_C_NAVIGATION_PCK` CMake variable. No private
assets are committed and no GitHub Actions are manually run.

## Remaining work

The full `0x21F89C` live eligibility check, source query-epoch
memory side effects, original actor route consumption, original
post-search mode handlers, animation, stage renderer, Lua/game scripts,
and native game bootstrap are unfinished.

**Ape Escape 3 is still not playable natively on Windows.**
