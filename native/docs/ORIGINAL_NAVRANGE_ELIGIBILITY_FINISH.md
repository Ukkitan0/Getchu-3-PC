# Ape Escape 3 USA — original 0x21EF3C eligibility finishing block

Original commercial executable: owner-private USA SCUS_975.01, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The original ELF and game data are not committed. This is source-level C++ recovery,
not PS2 emulation or replacement navigation.

## Recovered original machine region

Original 0x21F3C8..0x21F3FC is 14 instructions:
- 0x21F3C8 BEQ s8,zero,0x21F3FC skips the optional vector store. Its
  0x21F3CC LQC2 delay-slot load still executes.
- Source matrix columns are loaded from stack +0x00/+0x10/+0x20/+0x30.
  VMULAx/VMADDAy/VMADDAz/VMADDw combine those columns with the
  previously prepared vector at stack +0x360.
- 0x21F3F0 QMTC2 s7,VF2 then 0x21F3F4 VADD.xyzw adds the resource
  header +0x20 vector that the callee preserved in s7.
- 0x21F3F8 SQC2 writes the resulting 16 bytes to the optional pointer.
- 0x21F3FC MOV.S F0,F20 returns the scalar accumulator whether or not
  output was requested.

The real 0x21F6D4 navrange group-list caller sets t0=0 in the delay
slot of JAL 0x21F750; 0x21EF6C copies t0 into s8. Therefore
**navrange group creation skips the optional vector-output path**.
Its polygon inclusion still depends on the scalar F20 result.

## Native source recovery

`native/include/ae3/original_navrange_eligibility_finish.hpp` implements
`originalNavrangeFinish_21f3c8` as a source-ordered four-column
vector transform and a preserved original scalar return. The optional
native result is `std::optional<OriginalVU0Vec4>`, rather than a
pretend PS2 guest-memory output pointer.

Inputs are the original inverse matrix columns, previously prepared
stack+0x360 vector, source resource +0x20 vector, and already-computed
F20. **The prepared vector is not assumed to equal a raw polygon
centroid.** The incomplete scalar arithmetic is not replaced with an
invented predicate. Native arithmetic ordering is retained, but
bit-exact VU0 rounding/flags are not yet established.

## Verification

Run `python native/tools/verify_original_navrange_eligibility_finish.py
PATH/TO/PRIVATE/SCUS_975.01` to check 17 retail opcodes, the precise
BEQ destination, five source stack vector loads, VU arithmetic and
the zero-output retail call site. The verifier passed against the
private original ELF.

`Getchu3OriginalNavrangeEligibilityFinishTests` covers the skipped
output path, source matrix multiplication, additive vector, identity
case, scalar preservation including negative zero and a NaN payload,
and 500 deterministic calls. Locally tested with optimized C++20,
UBSan and ASan+UBSan using a minimal test-only replacement header
with the same OriginalVU0Vec4 layout. The entire Windows game build
has not been run here.

**Still unfinished:** deriving the original prepared vector at stack
+0x360 and all paths computing F20 inside 0x21EF3C. Real retail
navrange allocation and playable native Windows startup also remain
unfinished. No GitHub Actions workflows were dispatched.
