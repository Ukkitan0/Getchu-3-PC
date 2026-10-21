# Ape Escape 3 USA — Original navigation type-2 source pointer binding

Retail evidence: private original USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`; original `woods_c/navigation.pck` SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`. No retail bytes are committed.

## Recovered original engine behavior

The original `0x21D46C` constructor iterates a graph's 0x1E-byte polygon headers. The original `0x21D658` reads each action kind from header+0x0A+2*actionIndex. If kind is **2**, `0x21D6DC` uses the action's unsigned 16-bit source index, reads the source table pointer at graph header+0x14 (loaded from source+0x44), multiplies the index by **0x20**, and writes the resulting original 32-bit source pointer into the 0x70-byte native spatial record at **+0x10+4*actionIndex** (`0x21D708`).

This is a pointer into the **original loaded navigation payload**, not a constructed Monkey/Enemy actor. It requires the actual source allocation runtime VA to calculate the original pointer identity.

Original kind **1** takes a different path at `0x21D70C`: it allocates an eight-byte child object through `0x41B800`, initializes it with `0x21B0B0` (whose payload is a dynamically allocated list of 16-byte vectors), then fills those vectors using `0x21B240`. That separate original child runtime is **not** reconstructed in this pass.

## Real retail observations

The authentic woods_c PCK has **39 monkey navigation polygons** and **20 enemy navigation polygons**. Inspecting all of their descriptors shows **15 kind-2 source actions** in `nav_monkey.nav` and zero in `nav_enemy.nav`. All 15 reference original table index 0 and therefore the original source-record offset **2064**. This table is exactly 32-byte indexed. Other action kinds are left pending rather than fabricated.

## New native work

`native/include/ae3/original_nav_source_refs.hpp` implements source-derived `originalNavResolveType2_21d6dc`, checks each original record/action index and 32-byte source table entry against the loaded allocation, resolves the real source runtime VA provided by the caller, writes exact little-endian pointer bits at the source record's original offset, and removes only completed kind-2 actions from `pendingOriginalActions`.

`native/src/original_navgraph_loader.cpp` now calls this routine immediately after `originalStageBuildSourceSpatial_21d46c` to make the original source-referencing action path part of the **actual native NavGraph loader**, rather than a standalone prototype.

For invalid input, the binder first validates every source reference before writing any record fields. This native exception-safety behavior is additional to the original unchecked source execution.

## Verification and open issues

`verify_original_nav_source_refs.py` checks the private executable and PCK SHA values; **three complete instruction windows (129 original instructions)** with pinned independent digests; **15 opcode anchors**, **three JAL edges**, **four branch destinations**, original table offset and 32-byte stride, plus every one of the 15 type-2 references observed in the retail PCK.

`original_nav_source_refs_test.cpp` tests the exact original on-disk records, all 15 pointers and their destination slots, absence of type-2 in enemy navigation, preserving pending type-1 child allocations, and rejecting invalid table indexes. GCC Release, Clang Release, UBSan and ASan+UBSan passed locally using a minimal compatible stand-in for the existing spatial data classes. It is a real private-asset regression but **not** a complete Windows game build. It is enabled in CTest only when the owner supplies `AE3_RETAIL_WOODS_C_NAVIGATION_PCK`.

The actual type-1 child allocator, runtime actor ownership, stage update loop, renderer and native Windows bootstrap are unfinished. This pass does not make the original game playable.
