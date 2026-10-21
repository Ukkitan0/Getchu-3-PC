# Original Ape Escape 3 USA: Spatial grid, plane tests and record lifecycle

**Original source:** Owner-private original USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This is **actual native C++ source-level reconstruction from the retail executable**, not a PS2 runtime, synthetic game, or fabricated character engine. The original commercial executable and assets are not committed.

## Recovered connected code paths

| Original function | Verified original behavior | Native source status |
|---|---|---|
| `0x4E3C14` | Read original grid metadata at +4 and original hit-list pool at +8. Normalize source X/Y against grid origin/spacing at +0,+4,+0x10,+0x14; perform **R5900 truncation toward zero**; clamp to grid dimensions +0x18/+0x1C; resolve the u16 cell offset at +0x50; return its u16 hit header | Scalar lookup and direct hookup to existing `0x21D898` nearest-object selector ported. Packed input-vector extraction not yet ported |
| `0x21B788` | Iterate source plane count at object+0x30+0x16; return false immediately if the **VU0-computed** float comparison `f1 < (f0-f12)` is true; return true if all planes pass or count is zero | Original loop, ordered float test and short-circuit ported; VU0 projections remain explicit inputs |
| `0x21B468` | Convert source plane count to single-precision float and divide original `1.0f` constant at `0x6E3E2C` by that count before VU scaling | Scalar reciprocal translated for positive counts; VU accumulation and PS2 zero-divide not guessed |
| `0x21B178`, `0x21B180` | Read original point count and indexed 16-byte vector at child+4 +16*i | Native raw-source point accessors ported |
| `0x21B194` | Iterate consecutive point pairs; load each original 16-byte vector; add computed VU0 sqrt distance to single-precision total | Actual loop and order translated, with original distance as an unresolved callback |
| `0x21B14C` | If child pointer at offset +4 exists, pass that allocated field to game deallocator `0x41A3D8` | Exact conditional cleanup forwarding translated |
| `0x21B384` | Traverse three child pointers stored at record offsets +4,+8,+0xC; for each present child call `0x21B14C` then `0x41A3B0`; finish by calling `0x21AFB0` on record+0x20 | Actual destructor order ported |
| `0x21AFB0` | Return with no side effects | Verified original no-op translated |

The code is in `native/src/original_spatial_support.cpp` with C++ declarations in `native/include/ae3/original_spatial_support.hpp`. It extends rather than replaces the source-derived spatial selectors `0x21D898` and `0x21DA2C`.

## Original executable verification

Run `python native/tools/verify_original_spatial_support.py C:\\private\\SCUS_975.01` against your own original USA executable.

The audit validates the complete original ELF hash, **eight independent code-region SHA-256 digests covering 236 original instruction words**, **28 semantic instruction anchors**, eight direct original J/JAL targets, and the float-one global at `0x6E3E2C`. The original executable code bytes are not included in the repository.

Local source tests passed with `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror` and UndefinedBehaviorSanitizer. They test grid-coordinate boundaries and header lengths; empty hit cells; lookup-to-nearest integration; candidate rejection, ties and NaN; vector-array access; polyline summation order; and all three child destruction paths. Previously translated selector tests also passed.

## What's **not** yet reconstructed

The packed-vector input extraction at the start of `0x4E3C14`, actual VU0 plane/collision projections, vector accumulation, and per-segment distance mathematics remain missing. The host interfaces require those values from the genuine engine; unit-test fixtures are not game implementations. Source-level object ownership is native rather than an emulated guest heap.

**No Windows retail-game boot, genuine stage geometry or complete Player/Monkey update loop is established by this pass.** Windows compilation is unverified. No GitHub Actions or workflow runs were triggered.
