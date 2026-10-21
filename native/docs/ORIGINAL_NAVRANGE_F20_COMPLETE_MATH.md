# Ape Escape 3 USA — source-derived navrange eligibility F20 recovery

Source: original owner-private retail USA `SCUS_975.01`, ELF SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original game binary, Lua and resource bytes are not published.

## Key advancement: the required native callback now has a source-derived implementation

The original `0x4E4060` resource registration scans nav polygons using
`0x21F6D4`. Source fields and centroid are passed to `0x21EF3C`,
and only a returned `F0 == 0` permits polygon insertion.

This pass resolves the previously unknown *three MMI vector preparations*:

- 0x21EFFC / 0x21F000 / 0x21F008 / 0x21F010: MTSAB,
  QFSRV and two PPACW shuffles place original header +0x40
  **first float** into VF5 = [extentX,0,0,0].
- 0x21F094 / 0x21F098 / 0x21F0A0 / 0x21F0A4:
  shift the same source by four bytes and pack VF5 = [0,extentY,0,0].
- 0x21F15C / 0x21F160 / 0x21F168 / 0x21F16C:
  shift eight bytes and pack VF3 = [0,0,extentZ,0].

The source normalizes those vectors with VRSQRT and computes three
XYZ dot products against the local-space centroid. Each resulting
scalar is written to index 0, 1 or 2 of the intermediate through
the previously reconstructed 0x44BD80 scalar lane setter. The
original COP1 code then compares those projections to positive and
negative values from source header +0x40/+0x44/+0x48. For each
out-of-range component it adds the squared excess to F20; inside
each bound it adds zero. The original caller accepts exactly F20=0.

**Important earlier decompiler correction:** source opcodes
`0x21EF5C = 700684A9` and `0x21EF9C = 70102CA9` are
**POR** (MMI3 subtype 18), not PCPYUD (subtype 14). They copy
the *full* original quaternion. There is no Z/W-only truncation
at those sites. Previous full-quaternion native source handling
was correct; no modification to the resource frame was needed.

## Native integration

- `original_navrange_extent_score.hpp`: actual 32-bit source
  extent lane selection; original inverse quaternion transformation;
  original three normalizations/dots; source-derived F20
  outside-distance calculation and diagnostic intermediate values.
- `original_navrange_recovered_eligibility.hpp`: an explicit
  `OriginalNavrangeRecoveredEligibility` implementation of the
  existing source-aware callback, and
  `originalNavBuildGroupsRecovered_4e4060` that connects actual
  source list-owner building to the recovered mathematical decision.
  The previous required-callback API remains intact. Nothing changes
  in the normal game bootstrap or silently swaps other predicates.

For finite, nondegenerate inputs, this is a complete *mathematical*
source-derived navrange decision path. The implementation is native
C++20 and does **not** run any MIPS/VU instructions or emulate PS2.

## Verification and limitations

`native/tools/verify_original_navrange_extent_score.py` pins the
exact original 410-instruction routine at 0x21EF3C..0x21F5A4 and
checks **37 independent opcode anchors**, correct MMI3 POR
classification, all six PPACW vector-shuffle instructions, three
MTSAB instructions, and the original F20 return/caller gate.

Local source-header arithmetic tests passed with optimized C++20,
UndefinedBehaviorSanitizer and AddressSanitizer+UBSan. They covered
source-axis extraction, inside/outside/boundary points, full quaternion
X/Y retention, a rotated test, zero extent, and 1,000 deterministic
outside-distance samples. An additional regression test is added
to the repository for the *real* existing source-aware registry,
including a list of two admitted and one rejected polygon; the full
game-library CMake build and that end-to-end registry test were **not**
executed locally because the complete source dependency tree is not
available in this workspace.

This is **not yet bit-exact PS2 VU0 arithmetic**: unusual values,
rounding/underflow/overflow/NaNs, source VRSQRT behavior and cofactor
inverse precision are not differential-tested. A real retail
`navrange` resource allocation has not yet been recovered from
the ISO. Do not claim retail-accurate gameplay parity before those
tests. No GitHub Actions or CI were manually triggered.

## Native game blockers

Even with source-derived polygon-list eligibility, retail resource
ownership/lifetime, actor update consumers, stage graphics, input
and game bootstrap remain unfinished. A playable Windows port is
not yet available.
