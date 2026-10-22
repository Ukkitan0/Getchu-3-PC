# Original Ape Escape 3 stage loader -> native actor polygon raycast

The game already has the original USA source stage loader
`originalStageBuildSourceSpatial_21d46c`. It loads **the actual
0x70-byte polygon records** from each original stage's 0x1E-byte
polygon descriptors, using the game-owned vertex lookup tables
and original source relative addresses.

This commit closes the gap from those loaded source records to
the newly recovered original native actor raycast:

```text
Original stage resource (user-owned)
  -> originalStageBuildSourceSpatial_21d46c
      -> original 0x70 polygon record array
  -> originalStagePolygonLinks_21bd3c
      -> read original descriptor +0x02 degree
      -> original descriptor +0x04 edge destination indices
      -> original descriptor +0x0A edge action types
  -> originalStagePolygonRaycast_4c1b3c
      -> 0x4C1B3C original normalized ray calculation
      -> 0x21BD3C original triangle traversal/type-zero recursion
      -> 0x21EE00 original XZ edge clipping
      -> 0x4748AC original XYZ dot
```

`native/include/ae3/original_stage_polygon_raycast.hpp` and
`native/src/original_stage_polygon_raycast.cpp` make this
source-owned staging interface available to other actual engine
routines. The user supplies an already decoded
`OriginalStageSpatialLoaded` and the exact source
`OriginalStageSpatialSource`. No memory-map emulator and no
fake NavGraph are constructed.

The loader owns record allocations and lifetime. The new helper
returns pointers only into those stable original-source-owned
records, and never returns a dangling pointer into its own
temporary adjacency vector.

## Original game data validation

The owner-private genuine USA `SCUS_975.01` has SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The genuine `woods_c/navigation.pck` has SHA-256:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

Both original resource subgraphs have been audited using the
source's own +0x24 count and +0x3C descriptor offsets:
39+20 polygons, 95+42 directed edges, and 15 original
type-two actions.

`native/tests/original_stage_polygon_raycast_test.cpp`
now exercises the existing real source stage loader directly,
then decodes genuine adjacency headers and performs original
native raycasts on interior segments of all 59 polygons.
The CMake target can use the original private PCK through
`AE3_RETAIL_WOODS_C_NAVIGATION_PCK`; game assets remain private.

**Testing boundary:** The connected native clip, traversal and
composed raycast passed GCC and Clang local tests with genuine
source PCK data, including ASan/UBSan. This new stage-loader
integration has been structurally audited against original
source headers, but the full CMake/Windows project build is
not yet confirmed in this environment.

## Remaining source engine work

The actor controller needs to supply two distinct original
ray vectors at `0x57C814`; later route candidates use
the current source target-vector output at `0x57C890`.
The actor state update and original randomized fallback
`0x57CA68` remain incomplete. The actual game has not
booted as a complete native Windows program.

No copyrighted ISO, original game executable, game Lua,
or source navigation resources are checked into GitHub.
No emulation and no manually triggered GitHub Actions.
