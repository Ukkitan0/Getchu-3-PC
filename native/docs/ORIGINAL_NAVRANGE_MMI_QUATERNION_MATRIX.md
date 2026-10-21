# Ape Escape 3 USA: complete original quaternion-to-matrix routine

Recovered from the owner-private USA original `SCUS_975.01` ELF,
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original commercial binary, game data, and Lua files remain private.

## Complete native function: 0x359E74 through its return delay slot at 0x359EF4

This original routine is called by the still-incomplete spatial/navrange
eligibility code around `0x21EFA4`. The complete verified routine transforms
the GPR5 128-bit XYZW quaternion into four 16-byte rotation-matrix columns
using MMI packing followed by VU0 arithmetic. The return value is the
original output buffer pointer, while the C++ reconstruction returns
a host-owned four-vector value (the same matrix contents).

The original `MTSAH zero,4` at 0x359E80 sets a **64-bit** funnel shift.
`QFSRV` and `PEXCW` then rearrange the original four low-to-high
words `[x,y,z,w]` to populate:
- VF1: `[x,y,z,w]`
- VF2: `[y,x,w,z]`
- VF3: `[z,w,x,y]`
- VF4: `[w,z,y,x]`

Crucial operand detail: 0x359E84 writes to **GPR13/t5**, not GPR5/a1.
This was independently checked from the original rd/rs/rt instruction fields.
The following `QMTC2` operations copy all 128 bits without altering NaN payloads
or signed zero. The new `originalNavrangePack_359e80` function computes these
permutations directly; no runtime PlayStation instruction interpreter is used.

The already recovered `originalNavrangePrefix_359ea4` performs
0x359EA4..0x359EC8. This pass directly translates the final instructions:
- 0x359ED4 `VMULAz` initializes ACC from VF4 and VF1.z.
- 0x359ED8 `VMADDAw.y` adds 1 to ACC.y.
- 0x359EDC `VMSUBx` calculates the new VF2 column.
- 0x359EE0 `VMULAx` initializes ACC from VF3 and VF1.x.
- 0x359EE4 `VMADDAw.z` adds 1 to ACC.z.
- 0x359EEC `VMSUBy` calculates the new VF1 column.
- SQC2 writes VF5 at +0, VF2 at +16, VF1 at +32, VF0 at +48.
  The final VF1 store is the **JR return delay-slot** at 0x359EF4.

The matrix has the quaternion rotation form (source columns):
```text
col0 = [1 - 2(y*y+z*z), 2(x*y+z*w),   2(x*z-y*w),   0]
col1 = [2(x*y-z*w),   1 - 2(x*x+z*z), 2(y*z+x*w),   0]
col2 = [2(x*z+y*w),   2(y*z-x*w),     1 - 2(x*x+y*y), 0]
col3 = [0,0,0,1]
```
The implementation follows original accumulator and lane-write order, rather
than substituting the closed-form matrix expression. No normalization, source
coordination, or eligibility rule was invented.

## Verification

`native/tools/verify_original_navrange_mmi_pack.py` reads the private ELF,
checks its SHA and full 33-instruction source-region SHA, then independently
checks MTSAH, five original MMI instructions, four QMTC2 sources/destinations,
six VU0 arithmetic instructions, four SQC2 stores, and the JR return opcode.

Native `Getchu3OriginalNavrangeMMIPackTests` tests raw 128-bit permutations,
special IEEE754 payloads, 1,000 distinct word patterns, contiguous VU arithmetic,
identity, a quarter-turn, and a non-unit quaternion's exact four output vectors.
Local strict optimized C++20, UBSan, and ASan+UBSan runs passed with a test-only
reproduction of the existing `OriginalVU0Vec4` layout. A full native Windows
build was not run. No GitHub Actions or CI workflow was dispatched.

## Still unresolved

Actual owner game memory -> GPR5 input origin, the containing
`0x21EF3C` eligibility decision, `0x359F44` vector dependency,
real retail navrange resource ownership, and game-engine startup/render/update.
The project remains a **nonplayable native engine reconstruction** at this point.
