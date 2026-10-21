# Ape Escape 3 USA — Original navigation type-1 vector child recovery

Private inputs verified: original USA `SCUS_975.01`, SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`; retail `woods_c/navigation.pck`, SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`. Neither file is stored in this repository.

## Original source-level operation

When a polygon action has kind **1**, original code `0x21D70C..0x21D7B8` takes an unsigned 16-bit selector from the polygon descriptor at action slot `+0x10+2*j`.

The six source-relative offsets in the navigation header come from `source+0x38..+0x4C`. The type-1 branch uses:

- `source+0x4C` (offset element 5) as a packed 16-bit count/index-list table. The action selector indexes this table in **two-byte words**.
- The first selected 16-bit entry is the child vector count.
- The following count entries are unsigned 16-bit indices into the **16-byte source vector table** at `source+0x40` (offset element 2).
- Source routine `0x21B0B0` stores the count, allocates `count * 16` bytes through `0x41B8B0`, and installs the allocation pointer at child+0x04. The containing 8-byte child allocation is requested via `0x41B800`.
- Source `0x21B240` calculates `child_array + vector_index*16` and uses the original **SQ 128-bit store** to copy each loaded vector into that destination.
- The spatial record originally stores a *guest pointer* to that child allocation at `record+0x04+action_index*4`.
- Original `0x21B14C` releases an allocated 16-byte vector array, and `0x21B384` invokes it while tearing down the owner graph's records.

This is the actual original data-structure construction path, **not** a behavior script or monkey AI reconstruction.

## New native code

`native/include/ae3/original_nav_type1_children.hpp` implements `originalNavMaterializeType1_21d70c`. It accepts the exact original navigation bytes and existing `OriginalStageSpatialLoaded` record structure; reads the indexed count and vector references from the original source-relative offsets, copies every source vector's **16 raw bytes**, checks all referenced offsets before mutating the decoded graph, and transfers ownership into `OriginalNavType1Child_21b0b0` records.

The production `native/src/original_navgraph_loader.cpp` invokes this new step after `originalStageBuildSourceSpatial_21d46c` and the previously implemented type-2 source-reference binder `originalNavResolveType2_21d6dc`. Type-1 vectors are now retained as **host-owned decoded objects** and released automatically with the existing stage NavGraph owner lifecycle.

Important distinction: the original 8-byte child and its heap vector are represented as native typed owned data, **not** binary-compatible 32-bit guest allocations. The native loader does NOT invent guest VAs and therefore does not write fabricated child pointer values into the original record's pointer slots. Reconstructing guest-compatible allocation identities is a separate dependency of live runtime actor attachment.

## Verification and limits

`native/tools/verify_original_nav_type1_children.py` validates both private SHA-256 values, hashes **seven complete original instruction regions totaling 168 original instructions**, checks **35 exact instruction anchors**, **six original direct-call edges**, **four source branch targets**, count*16 allocation, 16-byte SQ source copy, and source header table references.

`native/tests/original_nav_type1_children_test.cpp` exercises a source-format input, multiple indexed vectors, zero-count children, type-1/type-2 coexistence, record/action bounds, invalid table/vector references, a native child that deliberately has no fake guest pointer, and **5,000 deterministic indexed list cases**. The test optionally checks the original retail woods_c package's action counts.

The actual owner-provided **woods_c** PCK contains **zero kind-1 navigation actions** and **15 kind-2 actions** (all in nav_monkey.nav). Thus the source-derived type-1 algorithm was exercised by controlled input, not by a positive type-1 example from woods_c. A different original stage's navigation package with positive type-1 actions is needed for a genuine live-asset differential test.

Locally GCC and Clang optimized Release, UBSan, and ASan+UBSan tests passed against the recovered type-1 binder and previously committed original type-2 binder interface. The new verifier passed on the exact retail ELF and woods_c PCK. The entire repository/Windows game build was **not tested**.

## Still to recover

Original 0x21B194's VU0-backed vector path-distance calculation, full guest allocation and destruction parity, actor/AI spawning and update, original stage graphics, script runtime, and the Windows native executable boot path remain unfinished. This is not a playable native Windows game. No manual GitHub Actions runs or copyrighted game uploads were performed.
