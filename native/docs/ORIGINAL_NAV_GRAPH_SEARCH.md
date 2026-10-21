# Ape Escape 3 USA — Original 0x220C00 / 0x220D5C NavGraph search

This is a **source-derived native translation** of the original retail
Ape Escape 3 navigation node construction and complete shortest-path
traversal, not a generic replacement-game prototype or PS2 interpreter.

## Private evidence (not committed)

- User-owned original USA `SCUS_975.01`, SHA-256
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- User-owned original `woods_c/navigation.pck`, SHA-256
  `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

No retail assets, executable bytes, proprietary vector tables or
sampled commercial game data are added to the repository.

## Original graph-search node construction at 0x220C00

The original 0x220C00 routine traverses the currently selected original
0x70-byte spatial records. It uses 0x21B3F8 to obtain each polygon
header's neighbor count at +0x02; 0x21B404 / 0x21D884 to resolve each
neighboring original spatial record; 0x220D14 to find that record's
index in the selected original search array; and 0x21B444 to read the
original edge type from polygon header+0x0A+2*actionIndex.

For every present neighbor, the routine builds an original **0x38
(56)-byte search record** with these fields:

| Original search record | Original operation |
|---|---|
| +0x00 | visited byte |
| +0x04 | original 0x70-byte spatial-record pointer |
| +0x08 | number of usable neighbors |
| +0x0C..+0x14 | three original selected-neighbor indices |
| +0x18..+0x20 | three corresponding edge type words |
| +0x24..+0x2C | three original polygon action indices |
| +0x30 | previous search-node index |
| +0x34 | best accumulated path length |

The original 0x220D14 returns -1 for neighboring spatial records absent
from the *selected search subset*. The constructor skips those edges,
rather than creating fake missing graph nodes.

`original_nav_graph_search.hpp` provides
`originalNavBuildSearchNodes_220c00` and
`originalNavBuildAllSearchNodes_220c00`. They use the original
polygon header bytes and the already decoded native original spatial
records. The source polygon centroid is calculated using the
**existing** `originalVU0Mean_21b468` routine over the three copied
original 16-byte vertex records. Original runtime 32-bit pointers are
not manufactured or interpreted as host addresses.

## Original lowest-distance traversal at 0x220D5C

The source routine explicitly sets the **first selected search node's**
predecessor to -1 and distance to 0; its other nodes and visited flags
are provided by the earlier allocation/setup phase. Source code scans
for the first **unvisited** node with the strictly smallest accumulated
distance, starting at the literal **FLT_MAX = 0x7F7FFFFF** loaded from
retail ELF VA 0x6E47E8. A tie retains the earlier node in search-order.

On each iteration, the selected node is marked visited at +0x00 and
its neighbors processed. An already-visited neighbor is skipped.
Original edge type 1 follows the existing type-1 child-vector route
`0x21B4E0 -> 0x21B194`. Types 0 and 2 take the original source
polygon centroid / XYZ distance path at 0x220EC8. The shared
0x220E8C decision updates destination +0x34 (score) and +0x30
(predecessor) **only if** the new cost is strictly smaller than the
stored score. Source execution stops when no unvisited node has a
distance below FLT_MAX.

`originalNavExecuteSearch_220d5c` reconstructs that entire loop.
It calls already recovered and linked VU0 distance and type-1 child
functions; it does not implement duplicate math. Native host bounds
checks and validation of unimplemented edge types are intentionally
additional to the original unchecked failure behavior. Typed route
nodes are semantic reconstructions, not the original 32-bit PS2
heap allocations.

## Tests — real game navigation, not just constructed inputs

The authenticated private `woods_c/navigation.pck` contains:

| Original file | Spatial polygons | Directed source edges |
|---|---:|---:|
| `nav_monkey.nav` | 39 | 95 |
| `nav_enemy.nav` | 20 | 42 |
| **Total** | **59** | **137** |

The source edge classes are 122 type-0 and 15 type-2; **zero type-1
actions** are present in this particular stage. The native graph
builder constructs all original adjacency entries from the retail
polygon headers. The original source traversal reaches **all 39**
monkey polygons and **all 20** enemy polygons from their first nodes.

For each graph, the native costs are compared with an **independently
written double-precision Bellman–Ford oracle**, and each non-root
predecessor is checked against actual source graph adjacency. Native
tests also cover the original search-subset filtering, a constructed
positive type-1 child route (cost 7, indirect route cost 17),
unreachable FLT_MAX handling and invalid-adjacency rejection.

The committed verifier `verify_original_nav_graph_search.py`
checks **four SHA-pinned complete original instruction windows**
(`0x220C00..0x220D5C`, `0x220D5C..0x220F60`,
`0x21B3F8..0x21B458`, `0x21D884..0x21D898`) for
**245 original instructions, 73 exact opcode anchors, 8 JAL edges,
25 branch destinations**, original field offsets, the FLT_MAX
literal, and source navigation connection counts.

GCC optimized C++20 Release, Clang optimized C++20 Release, UBSan
and ASan+UBSan passed locally with both the synthetic test and the
user-private real asset; the local test uses a minimal already-recovered
VU0 source implementation to link the existing geometry functions.
The **full repository CMake build and Windows native game runtime were
not executed**.

CTest runs the synthetic test without private assets; when
`AE3_RETAIL_WOODS_C_NAVIGATION_PCK` points to the user's original
private resource, it also executes the real-asset shortest-path test.

## Remaining blocker

The native graph-search component now has source-backed adjacency,
route costs and predecessor traversal. The original 0x220AA4 search
candidate setup/selection, stage-owned graph-query lifecycle, actor
locomotion/AI consumers, original stage renderer/input/script runtime,
and Windows bootstrap remain separate work.

**Ape Escape 3 cannot yet be played as a fully native Windows game.**
No GitHub Actions were manually launched.
