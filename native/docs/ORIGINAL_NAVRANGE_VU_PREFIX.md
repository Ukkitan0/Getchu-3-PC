# Original Ape Escape 3 USA navrange VU0 dependency recovery

Source: original owner-private retail `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Neither the game ELF nor assets are committed.

## Original no-op dependency: 0x441F40

The two calls at 0x21EF94 and 0x21EFD0 target 0x441F40.
The entire 0x441F40..0x441F6C contains scratch integer register
setup, a finite branch-likely loop whose body is all NOPs, and a
JR ra/NOP return. It changes no game memory, FPU, VU registers
or externally observable state. The new native helper
`originalNavrangeBarrier_441f40` therefore does no work.

## Native arithmetic translation

The source sequence 0x359EA4..0x359EC8 has been decoded and translated
into `originalNavrangePrefix_359ea4`:
- VF1 = VF1 + VF1 (all four lanes).
- VF2.x and VF2.z = 0 - their previous values.
- VF6 = VF0 + VF0.w broadcast; immutable VF0=(0,0,0,1).
- VF3.y and VF3.z = 0 - their previous values.
- ACC = VF2 * VF1.y (broadcast).
- ACC.x += VF6.x * VF0.w.
- VF5 = ACC - VF3 * VF1.z (broadcast).
- VF4.x and VF4.y = 0 - their previous values.

The real machine operands, mask bits and source order were
verified against the original ELF. The native code acts on already
prepared VF1..VF4 values. **The source input's MMI unpacking and
initial DMTC2 register provenance are not yet translated.**
Consequently, the function is NOT connected to the original
resource-key polygon eligibility callback.

## Independent original-source verification

`native/tools/decode_original_navrange_vu.py` reads the private
ELF through its PT_LOAD segments, requires the exact SHA-256 and
decodes VU0 macro-mode arithmetic/register transfer instructions.
Four original-code ranges contain 612 original instruction words;
201 COP2 instructions were classified. Option `--json` produces
a local machine-readable atlas, without publishing the ELF.

`native/tools/verify_original_navrange_vector_prefix.py` checks
26 exact instruction anchors, two original JAL call targets and the
no-op loop branch target. `test_decode_original_navrange_vu.py`
checks decoded masks, source registers and instruction names.

The C++ regression exercises all result lanes, ACC order,
masked lanes, negative zero and NaN payload preservation.
Local strict C++20, UBSan and ASan/UBSan tests passed (isolated
header test harness with the existing OriginalVU0Vec4 layout).
The complete native game build was not tested in this pass.

## Remaining missing pieces

The full MMI/VU setup at 0x359E74, remaining 0x359F44 vector
transforms, 0x21EF3C final calculation, genuine navrange
resource allocation/lifetime and native gameplay/Windows boot.
No PS2 emulation and no invented gameplay logic were added.
No GitHub Actions were dispatched.
