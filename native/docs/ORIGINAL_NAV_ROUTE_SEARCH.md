# Ape Escape 3 USA original NavGraph route-search source recovery

Source: private retail USA SCUS_975.01, SHA-256 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d.

This batch translates actual path-search procedures from the original MIPS executable into native C++20. It is not a PS2 emulator, a generated replacement game, or a playable Windows executable.

## Corrected earlier target

Original 0x21DB64 (invoked by 0x21E648) repeatedly draws navigation geometry via original drawing routines 0x44BD80, 0x247B8C, 0x248280 and 0x248628. The previous report incorrectly suggested this routine was directly responsible for route AI. The actual source search-record update functions are 0x220C00 through 0x221030; original caller locations 0x2208A0 and 0x22097C run their relaxation step.

## Implemented original game routines

- 0x220C00: maps original linked polygon records into 0x38-byte search-node edge fields, including target indices, link kind and source adjacency slot.
- 0x220D14: returns the index of a matching original polygon pointer in a node array, or -1.
- 0x220D5C: seeds node 0, picks the unvisited record with strictly lowest accumulated cost, relaxes each neighbor edge, and stores destination predecessor at source offset +0x30 and distance at +0x34. Equal-cost routes keep the original predecessor.
- 0x220F60: chooses the closest node centroid to the original query position.
- 0x221030: chooses the farthest node centroid.
- 0x220994: walks predecessor indices from the destination back to node zero and returns the source polygon-pointer path in forward order.

The original game node is 56 bytes (0x38). Source offsets: visited +0x00, polygon pointer +0x04, outgoing-count +0x08, three destination indices +0x0C/+0x10/+0x14, original type codes +0x18/+0x1C/+0x20, source adjacency slots +0x24/+0x28/+0x2C, predecessor +0x30, cost +0x34. Native pointers are represented as host types; no guest-memory emulator is used.

Original types 0 and 2 use the already recovered VU0 polygon-centroid XYZ Euclidean distance, through the shared branch at 0x220EC8. Type 1 follows the original 0x21B4E0 source-spline accessor and 0x21B194 polyline length; that genuine engine method is an explicit unresolved callback. Unknown types are rejected rather than assigned arbitrary costs.

## Full authentic retail navigation source verification

All 290 privately extracted USA navigation files were passed through the original stage-to-0x70-record loader, the new 0x38 search-node builder, and the original cost-relaxation algorithm. Results: 19,377 real records; 43,639 original directed link entries; 42,346 source type-0 links; 1,293 source type-2 links; no type-1 or unknown edges; 37 source records with zero links.

The retail files contain exactly the types for which the recovered original centroid-based edge cost is implemented. The optimized source-code test also checked constructed diamond routes, strict tie-breaking, type-1 callbacks, route backtraces, and bad source-data rejection.

**Important limitation:** original 0x220AA4 recursively admits/searches a particular subset of polygon records for a live game query. Its original selection/filtering behavior remains to be recovered. The native helper in this pass loads every record in a source file before applying the recovered 0x220D5C algorithm. Consequently the overall workspace is not yet guaranteed to be identical to the live game's chosen workspace, despite using the original source data, edge types and route-relaxation behavior. No original Monkey or Player path-following controller has been integrated.

## Source and tests

Native source: native/src/original_nav_route_search.cpp
Header: native/include/ae3/original_nav_route_search.hpp
Regression: native/tests/original_nav_route_search_test.cpp
Original ELF auditor: native/tools/verify_original_nav_route_search.py

The private ELF verifier passed six complete function code windows (366 original machine instructions), 41 independent instruction anchors, and 11 original direct calls. Optimized native g++ C++20 compilation with -Wall -Wextra -Wpedantic -Werror passed. UndefinedBehaviorSanitizer passed. A local optimized executable then processed all 290 real USA navigation files without failure.

CMake adds original_nav_route_search.cpp to Getchu3Assets and registers Getchu3OriginalNavRouteSearchTests. No Windows retail-game boot, game level rendering, original actor path application, or GitHub Actions execution was performed.
