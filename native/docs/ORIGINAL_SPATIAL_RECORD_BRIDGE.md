# Original Ape Escape 3 native source: authentic spatial-record binding

**Verified source:** original owner-private USA `SCUS_975.01` ELF,
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This pass bridges the previous original VU0/spatial algorithms directly
to **the original 0x70-byte record layout**. It does not emulate the PS2,
invent polygon geometry or substitute new collision mechanics.

## What the real game executable does

Original `0x21B468` and `0x21B788` both obtain a header pointer using
`LW` at source record **+0x30** and obtain the original number of source
vertices using `LHU` at header **+0x16**. The corresponding 16-byte
original source vertices start at record **+0x40** and advance by
**+0x10** each iteration.

Original `0x21D898` reads unsigned 16-bit hit indices from the grid
and multiplies each by the original record stride **0x70 bytes**.
Original `0x21DA2C` uses the same 0x70 stride when scanning all records.

All fields and original access instructions are checked against the
original ELF by the new `verify_original_record_layout.py`:
**four original code regions covering 51 instructions and 17 exact
independent MIPS opcode checks**.

## Native code now included

- `originalSpatialHeaderPointer_21b468` reads the **original little-endian
  32-bit pointer identity** from record bytes +0x30, not a host pointer.
- `originalSpatialVertices_21b468` preserves all three possible
  original 128-bit vector slots from raw record bytes, including their
  bit patterns, without alignment/aliasing violations.
- `originalSpatialResolveHeaders_21b468` resolves each actual original
  pointer against owner-loaded original byte allocations with their
  corresponding original source VAs, and loads its 16-bit count at +0x16.
  It refuses missing, overlapping, mismatched or out-of-bounds headers.
- `OriginalResolvedSpatialGeometry` constructs immutable source-vector
  spans directly from the actual records with validated source counts.
  The number of vertices is **not guessed or injected as an arbitrary
  sample polygon**.
- `originalSpatialNearestOriginalDataGrid_21d898` and
  `originalSpatialNearestOriginalDataAll_21da2c` connect these raw
  records and resolved headers to the recovered **grid lookup,
  per-plane eligibility, VU0 centroid, XYZ distance and nearest-object
  selection** code already in the project.

The interface deliberately accepts **original loaded allocation bytes
tagged with an original runtime address**, not a PS2 virtual memory
interpreter. The native game's asset/actor loader must eventually provide
those real allocations as part of reconstructing the original engine.

## Safety and correctness limitations

- A 0x70 record holds **three** 16-byte vertex slots after offset +0x40;
  this module supports valid header counts 1–3. Source layouts with
  more points need separate source-driven reconstruction; they are
  rejected, not silently truncated.
- Header count zero would execute the original game's unverified PS2
  floating-point division-by-zero path; it is rejected rather than
  assigned invented results.
- Original source pointers, headers, asset allocations, and query
  vectors must be provided by a **genuine game-data loader**. This
  module does not by itself extract the game's stage geometry or boot
  the original Player/Monkey objects.
- PS2-specific FPU rounding/exception differences in earlier VU0
  translations remain unresolved.

## Verification

The source is linked into `Getchu3Assets` and has a new CTest
`Getchu3OriginalSpatialRawBridgeTests`. The owner-private ELF
checker and optimized native C++20 test passed locally. A separate
`-fsanitize=undefined` native run also passed.

Unit-test records and source allocation bytes are **fixtures only**.
They are never used as substitute runtime geometry.

**Windows native gameplay has not been verified or produced.**
No GitHub Actions were run; the commercial executable remains private.
