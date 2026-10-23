# Original Ape Escape 3 — native stage-to-spatial-registry construction

**Source:** owner-private USA retail \`SCUS_975.01\`, SHA-256
\`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63\`.

This batch reconstructs a **real upstream loader** for spatial geometry,
rather than asking tests to fabricate or separately assemble a polygon
collection. It continues the earlier source-level recovery of the
grid lookup, original VU0 polygon tests, centroid and nearest selectors.

## Actual original game's constructor

The original stage-to-spatial-registry constructor begins at **0x21D46C**
and accesses a loaded stage data block:

| Source field | Actual executable access |
| --- | --- |
| \`source + 0x24\` | Original number of 0x70-byte spatial records |
| \`source + 0x38..0x4C\` | Six 32-bit source-relative table offsets |
| \`source + offset(+0x3C)\` | Source per-record descriptors, **0x1E bytes each** |
| \`descriptor + 0x16\` | Unsigned 16-bit polygon vertex count |
| \`descriptor + 0x18 + 2*i\` | Unsigned 16-bit source vertex index |
| \`source + offset(+0x40) + 16*vertex_index\` | Original vertex table entry: 16-byte stride |
| \`native record + 0x30\` | Original pointer identity of that source descriptor |
| \`native record + 0x40 + 16*i\` | Original vertex's three 32-bit coordinate lanes and source-defined fourth lane |
| \`native record + 0x00\` | Original registry owner pointer written by \`0x21B254\` |
| \`native record + 0x24\` | Original registry owner plus 0x18, written by \`0x21B254\` |

The original record initializer \`0x21B254\` clears its child slots.
Its caller \`0x21D46C\` also branches into *game-owned object
construction* based on source descriptor action types 1 and 2.

The actual original machine-code control-flow windows from
\`0x21D46C..0x21D7D8\` plus the record initializer
\`0x21B254..0x21B288\` are checked against the owner-private ELF.

## Source-level native C++ implementation

\`native/src/original_stage_spatial_loader.cpp\` and
\`native/include/ae3/original_stage_spatial_loader.hpp\` contain the
native reconstruction.

The native parser requires the original already-loaded stage bytes and
the **actual source allocation's original runtime address**. It:

1. Reads and bounds-checks all six relative pointers and the original
   record count, with overflow protection.
2. Creates the original **112-byte** record layout, preserving source
   header pointer identity and optional registry-owner reference.
3. Resolves each descriptor's original local vertex indexes through
   its actual source vertex table; writes original XYZ lane bits and
   \`1.0f\` W lane, preserving original geometry rather than inventing it.
4. Reads source-specified extra selectors and captures original
   action types 1 and 2 in \`pendingOriginalActions\`, without
   creating substitute actors/controllers.
5. Returns source-backed records and validated polygon header bindings
   directly compatible with the earlier original VU0 geometry recovery.
6. Provides two native end-to-end calls:
   \`originalStageNearestGrid_21d898\` and
   \`originalStageNearestAll_21da2c\`.

The complete translated chain supported by this batch is:

\`\`\`text
original loaded stage bytes
  -> 0x21D46C source offset table and 0x1E descriptor traversal
  -> 0x21B254 original 0x70 record layout
  -> original vertex-index table + original source vertex data
  -> 0x21B788 original polygon plane condition
  -> 0x21B468 VU0 polygon centroid
  -> original XYZ/VU0 distance
  -> 0x21D898 grid nearest or 0x21DA2C full registry nearest
\`\`\`

This code does **not** use an EE interpreter, a PS2 virtual machine or
generated replacement level geometry.

## Executable verification and tests

The new \`native/tools/verify_original_stage_spatial_loader.py\`
requires exact SHA-256 of the owner's original USA ELF, and verifies:

- **3 complete original code regions covering 232 instruction words**
- **32 independent source instruction sites**
- **6 original J/JAL function destinations**

It also records unresolved game engine calls and exact structural
field offsets. No commercial executable bytes are committed.

\`native/tests/original_stage_spatial_loader_test.cpp\` runs the
complete stage-source-to-spatial-selection path with *only test fixture
source bytes*. It covers multi-record construction, 16-bit vertex
indices, bit-preserving original XYZ and W lanes, original header
pointers, registry ownership, nearest-object selection, action
type capture, large count rejection and invalid pointer protection.

The source passed a local \`g++\` C++20 **-O2 -Wall -Wextra
-Wpedantic -Werror** build and a separate
\`-fsanitize=undefined -fno-sanitize-recover=undefined\` run.
The CMake/CTest target is
\`Getchu3OriginalStageSpatialLoaderTests\`.

## Remaining essential original game work

- **Real stage asset/loading path** that supplies the retail loaded stage
  allocation, actual runtime pointer identities and source descriptor
  bytes. Tests use fixtures, not the original game's in-level objects.
- Original \`0x36EDAC\` spatial allocator/grid initialization and its
  deeper \`0x36ED64\`/\`0x36EE4C\` dependencies.
- Actual actor/action creation for types **1/2**, calling the original
  \`0x21B0B0\`/\`0x21B240\` subobject constructors.
- Real Player and Monkey lifecycle/update loop, stage entity setup,
  native rendering and input.
- Original polygons requiring more than 3 source vertex slots and
  exact PS2 floating-point corner cases: these are rejected/qualified,
  not filled with guessed game data.

**This is substantial genuine original executable source recovery,
not a playable native Windows build.** No GitHub Actions runs were
created or dispatched; Windows compilation remains unverified.

## Unreal native bridge integration (2026-10-23)

The existing source spatial loader and original VU0 nearest selectors
are linked into `Getchu3OriginalLuaBridge`. The new C ABI functions

- `ae3_original_stage_nearest_all_21d46c_21da2c`
- `ae3_original_stage_nearest_grid_21d46c_21d898`

can be called by the real game manager with its owner-loaded source stage
allocation bytes, original runtime base VA, genuine query vector and,
for grid queries, the original 0x4E3C14 grid metadata and resolved two
scalar operands. No one must first fabricate 0x70 records, manually
pick descriptor point counts, or allocate a synthetic Actor/Monkey.

The ABI instantiates the translated `originalStageBuildSourceSpatial_21d46c`
and runs `originalStageNearestAll_21da2c` or
`originalStageNearestGrid_21d898` from the actual decoded stage.
Results contain the ORIGINAL source polygon index, not a pointer to
temporary C++ stage records. The outer native session refuses reentrancy;
malformed source data does not publish an output.

`native/tests/original_stage_nearest_unreal_abi_test.cpp`, registered
as `OriginalLoadedStageSpatialNearestHostAbi`, verifies a native
loaded-stage fixture through descriptor validation, genuine source
vertex-index lookups, nearest choice, grid-hit forwarding, invalid
source lengths/counts, missing source base VA, and output immutability.

This is still not a running playable Ape Escape 3. The original
`0x36EDAC` grid allocator / packed actor coordinate extraction,
dynamic action types 1/2 actor ownership, original `0x57C620`
controller update, complete Lua game bindings and rendering/runtime
integration remain outstanding. A successful Windows and Unreal
Editor compile has not been demonstrated for this bridge addition.
