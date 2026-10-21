# Ape Escape 3 USA — Original resource bucket insertion and exact hash

Source: user-owned original retail `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The original ELF and game assets are not distributed or included.

## Recovered original resource data structure

Previous commit reconstructed the 128-bucket source traversal at
`0x36D844`. This pass recovers its producer `0x36D610`:

1. The caller passes a 32-byte original lookup key and a source
   resource pointer. `0x36F1EC` allocates a node through the original
   pool allocator.
2. Original key is copied as two 128-bit words to node `+0x00` and
   `+0x10`. Source resource pointer is at `+0x20`; the next pointer at
   `+0x24` starts at zero.
3. The code hashes the first and second 16-byte halves via
   `0x36CFD0`, adds the 8-bit results, and masks with `0x7F`.
4. It appends the new node to the END of the selected linked list;
   no hash collision is treated as equality and keys are not deduplicated.

## Exact source hash

The 11 MMI instructions at `0x36CFD0..0x36CFFC`
use PEXTLB/PEXTUB, PADDB, PEXTLH/PEXTUH, PADDH,
PEXTLW/PEXTUW, PADDW and PCPYUD, finishing with an 8-bit mask.
The fixed source sequence reduces each half to an 8-bit unsigned
byte sum. The caller folds two halves as:

```cpp
uint8_t bucket = sum(originalKey[0..31]) & 0x7f;
```

This is **derived from the original EE instructions**, then simplified
to a closed-form native C++ expression. It neither translates nor runs
any EE instructions at game runtime. The original MMI register
semantics used for private verification were cross-checked with the
PCSX2 open-source reference
(`pcsx2/R5900OpcodeTables.cpp`, `pcsx2/MMI.cpp`);
no emulator or copied emulator runtime is incorporated in the mod.

## Native integration

`original_resource_bucket_owner.hpp` adds:

- Source-key-preserving `OriginalResourceKey32` and the exact
  `originalResourceBucketIndex_36cfd0_36d6a0` helper.
- `OriginalResourceBucketOwner<Resource>` for source-order
  allocation and appending of resource records to 128 buckets.
- An original-order broadcast through the previously recovered
  `originalResourceBroadcast_36d844` function.
- Clearing bucket heads and releasing the native node ownership,
  while payload resource ownership remains **external**.

A typed `OriginalResourceBucketOwner<OriginalNavrangeResourceDelivery>`
can pass its real bucket heads into
`originalNavrangeDispatchRegistered_27242c`: this bridges **insertion
and traversal of the original table** without inventing a resource
decoder or stage bootstrap.

## Verification

`verify_original_resource_bucket_insertion.py` checks the
SHA-pinned private ELF, **104 original instruction words**
covering 0x36CFD0, 0x36D610 and 0x36D844,
**24 independently pinned exact opcode anchors** and all
**11 MMI decoder field patterns**, two calls to the fixed hash
helper, original resource pointer/next offsets and the mask.
It independently runs the fixed MMI byte-lane reduction against
the simplified source-level formula on **10,004 keys**, including
10,000 pseudorandom cases, extreme bytes, and sequential bytes.

`Getchu3OriginalResourceBucketOwnerTests` checks hash collisions,
first/last bucket, source append order, duplicate key insertion,
correct broadcast order, cleanup/reuse, and a typed navrange receiver
using the same recovered table. Local isolated optimized C++20,
ASan+UBSan, and UBSan tests passed with a stub of the previously
recovered navigation API. Full native Windows game build was
**not** executed.

## Remaining limitations

The original container allocator is reconstructed at a source-level
ownership boundary, but **actual game-file resource allocation and
population of these source records are not yet recovered**. This pass
does not manufacture a `navrange` asset or substitute a generic
resource for it. Other unfinished work includes the original stage
resource loader, actual retail asset differential tests, bit-exact
VU0 FPU, live actors/scene, rendering and startup.

The game remains **nonplayable** as a native Windows port.
No GitHub Actions were manually dispatched.
