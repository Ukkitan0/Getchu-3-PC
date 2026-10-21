# Ape Escape 3 USA — Original stage NavGraph owner lifecycle

Private verification inputs: original USA `SCUS_975.01` (SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`) and original `woods_c/navigation.pck` (SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`). Neither commercial file is committed.

## Source-derived owner lifecycle

At stage startup, **0x22D8B0 calls 0x21ED30** before **0x22D8C8 calls 0x4F0A5C** to consume navigation data. The original 0x21ED30 loops exactly three times, allocates **0x28 bytes** for each owner at 0x41B800, calls **0x21D3D0**, and installs pointers in the global array at **0x647B80** (four-byte stride).

The source initializer at 0x21D3D0 clears owner pointer words at +0x00, +0x04 and +0x10, sets owner+0x0C to -1 and initializes the owner substate at +0x18..+0x24 through 0x21AF94. The later source loader at 0x21D46C overwrites some of those words: +0x18 becomes the record-array pointer; +0x14 and +0x20 record counts; +0x04 the allocated record array. The source loader does not populate owner+0x10 at this point; a separate later grid allocation is required. The native graph owner records source-array pointer presence and keeps +0x10 inactive until verified. Do **not** model +0x18..+0x24 as an independent persistent four-word array.

At stage teardown, **0x22E06C calls 0x21ED94**, which iterates the same three owner slots, calls **0x21D43C** then frees each 0x28-byte owner through 0x41A3B0 and clears the global slot. The nested 0x21D43C calls **0x21D7D8** and 0x21AFB0.

**0x21D7D8 cleanup order:** release the source/header at +0; if a record array exists, traverse the original +0x14 count with 0x70-byte strides and call **0x21B384** on each source record. Original 0x21B384 examines three child pointers at record+0x04/+0x08/+0x0C, invokes original 0x21B14C and frees nonnull children, then calls 0x21AFB0 on its embedded +0x20 list. After traversing records, the original releases the record array via 0x36EDE8 and clears +0x04. Finally, it frees grid allocation at +0x10 via 0x36EDE8 and restores owner+0x0C to -1.

## Native translation

`native/include/ae3/original_nav_stage_owners.hpp` now provides `OriginalNavStageOwners_21ed30` with three separately host-owned slots. Each holds the original resource name, owned source allocation, decoded original graph records and typed semantic owner status. It exposes the source-ordered initialization, graph attachment through the existing `originalNavLoadGraph_4f0a5c`, single-slot cleanup, and full stage teardown.

The typed `OriginalNavOwnerFields_21d3d0` is **not** a byte-exact original 40-byte guest pointer struct: it records whether the original pointer fields are active, and retains the original count/sentinel semantics. Original runtime source allocation and owner addresses are required as explicit inputs to graph construction; they are never manufactured by the PCK reader.

The native graph release destroys native owned data with C++ RAII. It does **not** call original actor-child destructor code without actual actor allocations. Original pending actor actions from stage data are not yet constructed runtime objects. Thus this advances graph-owner lifecycle, not full NPC/monkey gameplay.

## Verification

`native/tools/verify_original_nav_owner_lifecycle.py` checks **nine entire original instruction windows (175 instruction words)**, **35 exact opcode anchors**, **17 direct call edges**, original owner allocation size 0x28, record stride 0x70, three global owner slots and the exact owner cleanup call path, plus both real navigation resource counts (39 monkey polygons, 20 enemy polygons).

`native/tests/original_nav_stage_owners_test.cpp` is an optional original-data regression using the owner's real `woods_c/navigation.pck`. It tests all three slots, source startup and attachment, retention of both authentic original source allocations, per-slot cleanup, repeated full teardown, restart and rejection of missing runtime owner identities. The fixture uses **test-only provided runtime VA identities**, not live game addresses. The test is isolated with a minimal stand-in for the already committed navigation graph constructor, so it does **not** establish that the complete Windows CMake game links or boots.

GCC C++20 optimized Release, Clang Release, UBSan and ASan+UBSan tests passed. The exact original ELF and private navigation.pck verifier passed. No retail game assets are checked into the repository.

## Remaining blocker

The original 0x36EDAC/0x36E368 allocator and 0x36E178 allocator bookkeeping, real source allocation addresses, graph-grid initialization and actor-child creation/destructors are still incomplete. The native game runtime, renderer, input, script execution and original stage update path remain nonplayable. This is a source-level translation, not emulation.
