# Ape Escape 3 USA — original VU0 geometry calculation reconstruction

**Game executable:** privately owned original `SCUS_975.01` (USA),
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This batch implements **original retail MIPS/COP2 source behavior in ordinary
native C++**. It does not emulate the console and does not invent substitute
gameplay or procedural geometry.

## What is implemented from the original ELF

| Original source region | Original EE/VU0 code sequence | Native reconstruction |
| --- | --- | --- |
| `0x21B468–0x21B4DC` | `PPACW` zeroes GPR2; `QMTC2` initializes VF2; `LQC2` reads 16-byte source vertices; `VADD.xyzw` accumulates; `DIV.S` calculates `1 / uint16_count`; `VMULx.xyzw` scales | `originalVU0Mean_21b468` |
| `0x21B210–0x21B234` | `VSUB.xyzw`, `VMUL.xyzw`, `VADDy.xyw`, `VADDz.xyzw`, `SQRT.S` | `originalVU0SquaredDistanceXYZ_21b210`, `originalVU0DistanceXYZ_21b234` |
| `0x21B194–0x21B23C` | Indexed source-point loads, distance, single-precision accumulation for successive points | `originalVU0PolylineLength_21b194` |
| `0x21B788–0x21B850` | Plane count; previous vertex initialized to final vertex; edge subtraction; original `PEXLW`/`PCPYUD` lane manipulation; `VOPMULA.xyz`/`VOPMSUB.xyz` normal; plane/query dot reductions; `C.OLT.S` | `originalVU0PolygonEligible_21b788` |
| `0x21D950`, `0x21D9FC`, `0x21DA98` | Same VU0 XYZ distance reductions in source nearest-selector loops | The existing `originalSpatialNearest_21d898` and `originalSpatialNearestAll_21da2c` now have adapters using real recovered math rather than arbitrary scoring callbacks |

The exact original `0x21B7D0–0x21B7D8` packed-data sequence moves
the edge vector's lanes from `[x,y,z,w]` to `[z,w,y,0]`.
The source `VOPMULA/VOPMSUB` with unit Y thus gives normal
`[edge.y, 0, -edge.z]`. Source `0x21B80C` applies the original
float margin, and `0x21B81C` rejects a plane only when the query
projection is **strictly** below the source projection minus that
margin. The original `0x21D904` passes margin `0.0f`.

## Connected source engine path (not synthetic gameplay)

`originalSpatialNearestGeometry_21d898` now accepts the actual
**0x70-byte original registry records, matching original record+0x30/+0x40
polygon point data, the true grid hit list and the actual query vector**.

It performs the following **real original** path:

```text
0x4E3C14 source grid hit list (ported in previous batch)
       ↓
0x21B788 real original per-plane eligibility (this batch)
       ↓
0x21B468 original VU0 vertex centroid (this batch)
       ↓
0x21D950 original VU0 X/Y/Z distance (this batch)
       ↓
0x21D898 original nearest accepted record (already ported)
```

The alternate `0x21DA2C` full-registry scan also calls the native
original centroid/distance implementation. The recovered
`0x21B194` path-length loop now has a source-grounded native math
implementation with no arbitrary distance callback.

**Data provenance:** The native functions require *resolved real source
vertices* and a query vector; the game-owned level/actor code that
produces those has **not** yet been fully reconstructed. Fixtures are used
**only in the unit tests**. The C++ code does not fabricate maps,
characters, levels, positions, enemy states, or original engine data.

## Auditing and tests

The hash-pinned private original ELF checker
`native/tools/verify_original_vu0_geometry.py` validates **6 original
code windows containing 169 actual original instruction words**,
independently checks **55 MIPS/COP2/MMI opcode sites**, and
checks **7 direct call targets**. The audit passed on the same
original USA executable; no game binary or extracted assets are
published.

The native C++ source and real spatial selector were compiled and
tested locally under optimized C++20 warning-as-error flags and
UndefinedBehaviorSanitizer. Tests cover centroid arithmetic, XYZ distance,
ignored W lane, sequential path length, strict two-sided plane decisions,
float margin, NaNs, source hit ordering, direct-registry scoring, null
results and invalid source metadata. A new CMake/CTest target
`Getchu3OriginalVU0GeometryTests` is registered.

Reference specifications used for checking **instruction meanings**
(not as a replacement source for the game):
- [PCSX2 COP2 disassembler](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/DebugTools/DisR5900asm.cpp)
- [PCSX2 VU floating-point operations](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/VUops.cpp)
- [EE R5900 packed-MMI opcode overview](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/R5900OpcodeTables.cpp)

## Still unimplemented

- Some PS2-specific floating-point exceptions, flush/rounding behavior,
  and zero-point `DIV.S`; these are **not** claimed bit-exact.
- Exact original source-pointer owner lifecycle and query-vector
  extraction from live Player/Monkey instances.
- Stage boot, scene/entity updates, original geometry initialization,
  native rendering/input and all other systems needed for actual gameplay.

**There is no working, playable native Windows Ape Escape 3 build yet.
Windows CI was not run and GitHub Actions were not dispatched.**
