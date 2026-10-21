# Ape Escape 3 USA — native original ordered-attribute tree insertion

Owner-private commercial executable: SCUS_975.01, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No executable, retail assets, or PS2 runtime emulation are included.

## Source recovery

The existing 0x5419EC ordered attribute lookup is built on an original
red/black tree. The original 0x541CA0 insertion function allocates a
24-byte node through 0x44E420 and calls 0x541FC4 to copy the
source-owned attribute-key object at offset +0x10 and the associated
word at +0x14. It links the node using these 32-bit fields:

| Original node offset | Source data |
|---|---|
| +0x00 | color: zero red, one black |
| +0x04 | parent |
| +0x08 | left child |
| +0x0C | right child |
| +0x10 | original string-key object |
| +0x14 | pointer to original allocated 0x34-byte child record (source 0x2BA9C0) |

The original 0x49C26C routine fixes tree colors and performs red/black
rotations through the original 0x49C3B0 (right) and 0x49C3FC (left)
routines. The 0x541DAC wrapper handles duplicate keys and does not
replace an existing keyed value. 0x5419EC searches the same tree.

These source routines match the red/black tree invariants independently
verified against the libstdc++ implementation of the same algorithm.
They are reimplemented in native C++, not interpreted as guest code.

## Recovered native code

`native/include/ae3/original_attribute_tree_owner.hpp` provides a
native owner that performs unique source-key insertion with source-style
red/black recoloring and rotations. The native decoded model keeps flattened 32-bit source data offsets.
The actual original node +0x14 is a *child record pointer*, not directly
that offset; the original numeric/string readers dereference the child
record +0x08 to obtain the source data offset. Keys are copied into host-owned native
strings, and the existing `OriginalAttributeKeyNode` view is updated
after each rotation. `decodedRoot()` can be given directly to:

- `originalAttributeFind_5419ec` — retail ordered lookup
- `originalAttributeString_2bb3bc` — original source C-string and fallback
- `originalAttributeFloat_2bb1d0` — retail numeric getter
- `originalAudioDescriptorFromDecodedAttributes_1bb9a8` — five original
  audio strings and its original `volume_scale` value.

No `std::map` replaces the original algorithm in production: a map is
used **only as a test oracle**. The host nodes use real host pointers
instead of original 32-bit guest addresses and do not claim binary
memory-layout parity with the original node allocation.

## Verification

`verify_original_attribute_tree_insertion.py` audits **seven entire
original functions or instruction regions: 364 original instructions,
39 exact opcode anchors, 11 JAL edges**, node field accesses, original
24-byte allocation, both rotations, and original source-key lookup.
It passed against the SHA-pinned private original ELF.

`Getchu3OriginalAttributeTreeOwnerTests` exercises source-style left/right
rotations and unique inserts, 18,000 randomized insertion attempts,
checks the red/black properties and extrema, compares key/value
lookup with an independent std::map oracle, and then executes the
existing original audio descriptor path using the tree's **real native
built root**. Strict optimized C++20, NDEBUG Release, UBSan and
ASan+UBSan passed in local isolated builds with the actual committed
attribute lookup/string header APIs. The full repository CMake build,
Windows game build and retail runtime differential were not run.

## Recovered upstream attribute binding at 0x2BA9C0

Tracing into the original attribute parser at 0x2BA8D0..0x2BA9CC
identified the first concrete source key/value insertion site.
The caller checks its key against the source tree at 0x2BA8F0 and
only inserts the key through 0x5415E4 if it is absent. However,
**0x2BA9C0 unconditionally writes the allocated child record pointer to node+0x14**
on both existing-key and new-key paths. This matters: the regular
original 0x541DAC unique insert leaves the original keyed word alone,
while a later parsed attribute **overwrites** that child pointer in the map.
The existing host `originalBindParsed_2ba9c0` collapses the child-pointer
indirection and retains only child+0x08's source offset. This shortcut
must not be mistaken for the original binary pointer layout.

`originalBindParsed_2ba9c0` now reproduces that distinct source
operation using the same native red/black tree. Regression tests
verify duplicate values are overwritten without creating extra nodes
and that the new offset affects the original audio descriptor's
`volume_scale` immediately.

`verify_original_attribute_tree_population.py` pins the private
original 63-instruction parser window and checks 19 exact opcode
anchors, seven call edges, the source branch targets, the original
source-node value write, and the always-executed assignment. It passed
against the original ELF. The serialized input format and memory
ownership further upstream of this routine remain unported.

## Still incomplete

What is recovered: the native original tree's construction and lookup
**when source attribute key/value pairs are available**. What remains:
the earlier serialized asset-data parser that obtains those pairs and
constructs the original allocation, exact string-object ownership,
genuine sound bank loading and the separate stage/navigation resource
loading path. Rendering, actor systems and native Windows boot are
not complete. **The original game is not yet playable.**

No GitHub Actions were dispatched; no copyrighted game bytes uploaded.
