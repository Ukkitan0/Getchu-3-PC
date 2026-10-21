# Ape Escape 3 USA — original NavGraph adjacency and local traversal

**Source authenticity:** owner-private USA `SCUS_975.01` ELF SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The game executable and retail data remain private. This implementation is
ordinary host-native C++20 source, not PS2 CPU/VU/GS emulation.

## Complete retail-data validation

The original ISO's `DATA.BIN` VFI table contains 168
`navigation.pck.sz` packages, with 290 .nav files (168 monkey, 122 enemy).
The private original navigation files were read from the ISO and passed
through **the actual new native topology parser**, combined with the
previously translated original stage record constructor.

| Original retail data validation | Count |
| --- | ---: |
| Original .nav resources processed | **290/290** |
| Authentic spatial records constructed from .nav files | **19,377** |
| 16-bit adjacency indices checked and resolved | **43,639** |
| Zero-link original navigation records | **37** |
| Out-of-range original links | **0** |
| Records with 1/2/3 links | **1,442 / 11,497 / 6,401** |

These numbers were measured using the owner-private retail ISO.
No commercially owned source .nav data or game assets were committed.

## Original function source verification

Directly disassembling the retail ELF established:

- **`0x21B3F8`**: return the original unsigned 16-bit adjacency count
  from the source descriptor pointer stored at record +0x30, offset +0x02.
- **`0x21B404`**: compare requested link index to that count using
  `SLT`; if in range, load unsigned 16-bit target from descriptor
  +0x04+2×index; follow the original record+0 owner and resolve the
  target record using **`0x21D884`** and the 0x70-byte record stride;
  if out of range, return null.
- **`0x21E648`**: attempt nearest-in-grid navigation selection via
  **`0x21D898`**; if null, fall back to nearest-in-all **`0x21DA2C`**.
  If selected, examine original descriptor adjacency count, invoke the
  complex original **`0x21DB64`** with the selected record,
  and advance through neighbors using **`0x21B404`**.
  The original loop performs **count+1** calls to `0x21DB64`: the
  selected record followed by each linked record. Its final requested
  neighbor index equals the link count, returning null.
- **`0x4F0FBC`** and **`0x4F0B00`**: identify and register the three
  original graph slots at global **0x647B80**. The first two exact
  names are `nav_monkey.nav` and `nav_enemy.nav`, while the
  remaining names select slot 2 (including `nav_extra0.nav`).
- **`0x22D8C8`**: game stage setup fetches a resource group named
  **`navigation`** at original string VA 0x6E7398, then calls
  the previous `0x4F0A5C` NavGraph loader.

`native/tools/verify_original_navgraph_topology.py` requires the
complete original ELF SHA, verifies three intact code regions totaling
**100 retail machine-code instruction words**, 24 exact
instruction anchors, seven decoded original J/JAL call destinations,
and the original navigation resource names.

## Native code and testing

`native/src/original_navgraph_topology.cpp` and its corresponding header
implement the original adjacency index and lookup, plus the connected
nearby-neighbor selection/traversal path and global slot binding.
The source has bounded pointer arithmetic and rejects malformed or
out-of-range source descriptor indices instead of guessing values.

The existing `Getchu3NavGraphProbe` now prints **the count of validated
original adjacency edges and zero-degree records** when run on a
user-owned USA ISO or extracted DATA.BIN, without extracting assets to
the user's directory.

CMake registers the source inside `Getchu3Assets` and provides the new
`Getchu3OriginalNavTopologyTests` CTest target. The actual C++20
optimized test passed with `-Wall -Wextra -Wpedantic -Werror`,
and a separate UndefinedBehaviorSanitizer test passed. A separate
native optimized C++ utility constructed all **290 real** navigation
graphs from their original source bytes and validated **every link**.

The native test fixtures used for CTest are independent synthetic
source records; the original data validation used genuine source
data privately, and those two types of evidence are not confused.

## Essential remaining dependencies

**Corrected after tracing actual original call targets:** 0x21DB64 repeatedly submits navigation vertices to original drawing procedures. The 0x21E648 neighbor traversal is part of this navigation-geometry visualization path, not completed pathfinding or Monkey AI. Actual original shortest-cost route search resides in 0x220C00, 0x220D5C and 0x220994. These source-derived native algorithms are now implemented in original_nav_route_search.cpp, but the game's original 0x220AA4 search-node filtering and the live actor movement/update integration remain incomplete.

Original 0x21DB64 drawing, resource/graph ownership, actor updates and genuine rendering startup still need recovery.

**No playable native Windows retail game build has been established.
No GitHub Actions or CI workflows were started.**
