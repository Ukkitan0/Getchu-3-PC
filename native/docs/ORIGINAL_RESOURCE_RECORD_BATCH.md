# Ape Escape 3 USA — source record pool to resource-bucket insertion

Retail evidence: private original USA `SCUS_975.01` ELF, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No commercial executable, Lua bytecode, or licensed game resources are committed.

## Verified source chain

The previously recovered `0x36D610` 32-byte-key bucket insertion has a
concrete upstream producer:
`0x1BB558 -> 0x1BB458 -> 0x1BB9A8 -> 0x1BB3EC -> 0x1BB4E4 -> 0x36D610`.

- `0x1BB558` allocates original 0x18-byte owner state and initializes
  its bucket table at owner-state `+0x10`. Source population stores
  its record buffer at `+0x04`, and `0x2BB0D0` provides the signed
  record count at `+0x08`.
- `0x1BB458` obtains each descriptor through `0x2BB0F4`, computes
  an array stride of **0xB0 bytes**, and calls `0x1BB9A8` to populate
  the record. The underlying descriptor parser is still incomplete.
- `0x1BB3EC` iterates the same original count, taking BLEZ for
  nonpositive values. `0x1BB4E4` resolves the record-base pointer
  through the original owner and calculates `base+index*0xB0`.
  The R5900 MULT instruction writes the low-word result into
  destination register t6, which the following ADDU uses.
- `0x1BB424` and its JAL delay slot at `0x1BB430` supply the same
  source-record pointer as both A1 (key) and A2 (payload). The
  `0x36D610` routine copies the first two 16-byte key blocks from
  record offsets `+0x00` and `+0x10` and stores the payload pointer
  at bucket-node `+0x20`.
- Four original initializers invoke this batch insertion routine:
  `0x1BB5F8`, `0x1BB6DC`, `0x1BB7BC`, and `0x1BB89C`.

**Corrected classification (retail literal audit):** This source chain is
part of the original AUDIO configuration descriptor system. The parser
at `0x1BB9A8` uses the original keys `name`, `midi`, `reverb`,
`vb`, `vh`, and `volume_scale` with audio defaults including
`M01.mid`, `M01.bd`, and `M01.hd`. It must NOT be classified as
the confirmed `navrange` stage allocation producer.
Its generic 128-bucket insertion code remains source-derived and
reusable, but the connection to navigation resources is unproven.
See `ORIGINAL_AUDIO_DESCRIPTOR_SOURCE.md`.

## Native recovery

`native/include/ae3/original_resource_record_batch.hpp` adds
`OriginalResourceRecordBatchOwner` and
`originalAppendPopulatedRecords_1bb3ec`. A caller supplies an
**already populated and externally owned** contiguous record buffer
and its actual original signed count. The routine bounds-checks
all 0xB0-byte records, preserves each raw record as a read-only span,
and inserts its first 32 bytes as a key through the recovered
`OriginalResourceBucketOwner`. Stable host-side descriptors allow
the original 128-bucket broadcaster to deliver every source record
without transferring ownership or interpreting guest instructions.

The underlying source buffer must remain allocated and unmoved until
the owner is cleared. This path does **not** fabricate original
records or claim to be an ISO/VFI loader.

## Verification

`native/tools/verify_original_resource_record_batch.py` pins
**eight complete original code windows totaling 287 instructions**,
**35 exact instruction words**, **13 JAL edges**, both original loop
targets, original record stride and owner offsets, and all four
builder call sites. It passed against the user's original ELF.

`Getchu3OriginalResourceRecordBatchTests` tests nonpositive counts,
truncated input, colliding keys, duplicate insertions, original
bucket order, repeated insertion, cleanup, and stable references
over 4,096 records. Locally it passed optimized C++20, UBSan and
ASan+UBSan runs with a minimal test-only compatible version of
the already committed bucket-owner API. Full Windows CMake
game build **was not run**.

## Blockers

The `0x1BB9A8` descriptor-construction chain must be traced to
authentic DATA.BIN/VFI allocations before any real stage resource can
be supplied. Original navrange ownership, live actor update,
graphics, input, and native Windows bootstrap remain incomplete.
**The native game is not playable.** No GitHub Actions were manually triggered.
