# Authentic AE3 USA engine recovery — spatial index and lifecycle

Original source: privately supplied USA `SCUS_975.01` ELF,
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This is native C++ source-level reconstruction of **actual game routines**,
not an emulator and not a replacement gameplay implementation.

## Connected original functions recovered

| Original VA | Verified code behavior |
| --- | --- |
| `0x21D898` | Query the original spatial grid via `0x4E3C14`, read u16 hit count/indices, examine candidates in order, call original eligibility at `0x21B788`, select the nearest eligible by original float distance. Equal distances keep the earlier record |
| `0x21DA2C` | Iterate the complete original contiguous record registry using count at vtable[1]+0x24, same nearest-distance comparison, no `0x21B788` candidate gate |
| `0x21D7D8` | Release owned +0 allocation via `0x41A3B0`; run original destructor `0x21B384` for each +0x70 record when allocation +4 exists, release it with `0x36EDE8`; release +0x10 auxiliary allocation; set +0xC to -1 **only when that auxiliary allocation existed** |
| `0x21D884` | Resolve source record pointer from original +4 data base and index times 0x70 |
| `0x21B404` | Check index against original header count at +2, load u16 index at header+4+2*i, tail-call `0x21D884` |
| `0x21B444` | Load a halfword at header+0xA+2*i |
| `0x21B458` | Read an **unaltered 16-byte original vector** at record+0x40+16*i |
| `0x21B4E0` | Load original 32-bit indexed word at +4+4*i |
| `0x21B4F0` | Load original 32-bit indexed word at +0x10+4*i |
| `0x21B500` | Read original 32-bit word at +0x1C |

**Significant details:** Source-game record stride is **112 bytes (0x70)**.
Both selectors initialize their best-distance register from actual
float literal at original `0x6E3E7C` or `0x6E3E80`:
IEEE-754 `0x7F7FFFFF`. Ties do **not** replace an earlier candidate.
Original memory teardown uses distinct allocation functions and clears
specific fields without automatically resetting the retained count.

## What is translated, and what remains

The original **outer nearest-selection algorithms, data accesses and
registry teardown control flow** have native C++ translations in
`native/src/original_spatial_index.cpp`.
The opaque engine methods are still dependencies: original
`0x4E3C14` spatial-grid query,
`0x21B788` candidate eligibility,
`0x21B468` position extraction and the VU0 floating-point distance
calculation; original `0x21B384` record destructor and
`0x36EDE8` allocation release.

The native callback interface **does not invent positions, eligibility,
distances, or scene objects**. Unit tests exercise call order using test
fixtures only. Complete authentic runtime integration still requires
translating those originals and the manager's usage of global `0x647B80`.

The verifier at `native/tools/verify_original_spatial_selection.py` checks
the *exact* source ELF SHA, **115 specific MIPS instruction words,
11 direct original J/JAL targets**, and both original
IEEE-754 distance-bound constants, without publishing game bytes.

To run the private retail verification:
`python native/tools/verify_original_spatial_selection.py C:\path\to\SCUS_975.01`

A **local Linux g++ C++20 optimized unit test and UBSan run passed** for
the initial full source translation. The source is registered in the
existing Getchu3Assets target and the new CMake/CTest target is
`Getchu3OriginalSpatialRegistryTests`. The complete
Windows native application has **not** been built or booted, and no
GitHub Actions/CI were triggered.

Next genuine executable recovery targets:
the VU0 calculation within the two selectors and the original
`0x21B468` averaging/position method, plus completing
the source-driven `0x4E3C14` spatial-grid query and the
registry's record destruction. Only after those can the real actor
manager execute this logic on original level records.
