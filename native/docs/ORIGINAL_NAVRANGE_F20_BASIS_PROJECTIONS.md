# Original Ape Escape 3 USA — native navrange basis projections

Original owner-private `SCUS_975.01` ELF SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The original executable, Lua scripts and game data remain private.

## Original 0x21F020..0x21F1B8 traced further

The previous recovery established the navrange resource inputs, original
quaternion rotation/inverse, and transformation of polygon centroid into
local space. This pass recovers three VU0 scalar projection subgraphs in the
same original 0x21EF3C eligibility routine. They normalize a *previously
prepared* 128-bit source vector and project the local coordinate onto its
first three components:

* 0x21F028 squares prepared VF5, 0x21F040 and 0x21F058 accumulate
  its XYZ squared length into VF4.x; 0x21F064 computes Q=1/sqrt(VF4.x),
  0x21F080 multiplies VF5 by Q, and 0x21F0B0/0x21F0D0/0x21F0DC
  calculate a dot product against the transformed polygon coordinate.
* 0x21F0B8, 0x21F0D8, 0x21F0E0, 0x21F0E8, 0x21F120 similarly normalize
  a second prepared VF5 vector. 0x21F12C/0x21F130/0x21F134 produce the
  second projection.
* 0x21F180, 0x21F190, 0x21F194, 0x21F198 and 0x21F1A0
  normalize a prepared VF3 vector, then 0x21F1A8 reloads the original
  transformed position from stack +0x380 before the third dot at
  0x21F1B0/0x21F1B4/0x21F1B8.

In each case, original VRSQRT selectors use VF0.w divided by the
square root of the respective vector's X-lane XYZ sum. VADDy.xyw
followed by VADDz.xyzw produces the XYZ dot in the X lane. The W
component is scaled by Q but is not included in the scalar dot.

## Native implementation

`native/include/ae3/original_navrange_basis_projection.hpp` adds
`originalNavrangeNormalizeBasis_21f024`,
`originalNavrangeProjectionXYZ_21f0b0`, and
`originalNavrangePreparedBasisProjections_21f024_21f1b8`.
They are source-level native C++20 mathematical translations, not
PlayStation 2 instruction emulation. Their inputs **must** be the three
already-prepared VF5/VF5/VF3 vectors from the original path. They are
**not** raw navrange header bytes, and no invented defaults are supplied.

The original MMI preparation at 0x21EFFC..0x21F1A8 remains unfinished.
`audit_original_navrange_mmi_basis.py` now reports its 21 MTSAB, QFSRV,
PPACW, POR, PAND, PNOR operations with original register operands, to
support recovering those input vectors rather than guessing. It is a
static source decoder, *not* an interpreter.

## Verification

`verify_original_navrange_basis_projection.py` validates the pinned private
original ELF and a second SHA-256 covering **all 103 instruction words**
from 0x21F020 through 0x21F1B8, including **30 explicit opcode anchors**,
three VRSQRT field selectors, three VMULq operand pairs, per-instruction
component masks, and the original stack +0x380 reload. The private
ELF audit and Python decoder regression tests passed locally.

Native tests exercise three independent basis directions, nonunit and
oblique vectors, actual four-lane W scaling, 1,000 deterministic projections,
and a zero-length basis without an invented safe fallback. Optimized strict
C++20, UndefinedBehaviorSanitizer, and AddressSanitizer/UBSan passed with an
identical-layout minimal test-only `OriginalVU0Vec4` definition.
This pass did **not** execute the full Windows game CMake build or a
bit-for-bit PS2 VU0 floating-point differential.

## Remaining blockers

Original input-register preparation, actual retail resource allocation,
full F20 scalar condition (including linkage between projection values
and radius/extent values), live actor navigation, stage rendering and
native Windows boot. The existing required eligibility callback remains
unchanged. The complete game is **not playable**.

No GitHub Actions were dispatched; no PS2 emulator or invented replacement
gameplay systems were introduced.
