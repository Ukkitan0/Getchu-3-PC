# Ape Escape 3 USA — decoded VU0 fallback source and recovered R-register state

The original United States `SCUS_975.01` code at
`0x57CA68..0x57CD18` (173 instructions; owner-private SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`)
contains two different VU0 candidate blocks, **not** a single repeated
unrolled barycentric routine.

## Original candidate machine code

`native/tools/audit_original_fallback_vu0_57ca68.py` authenticates the
original 173-instruction ELF region with its SHA-256, decodes all
native VU0 COP2 macro suboperations and register/mask operands, and
checks the 6 source RNG/XYZ-dot calls.

| Source block | Original VU0 macro operations | Important difference |
|---|---:|---|
| Initial candidate `0x57CB4C..0x57CBB8` | 20 | Has a unique `VMOVE` at `0x57CB74` |
| Repeated candidate `0x57CC48..0x57CCB0` | 19 | Reuses source state, no `VMOVE` |

Both use source VMULx, VADD, VSUB, VMUL, VADDy, VADDz and
two **VMULq** operations apiece. The VU0 Q register is *not*
initialized in this function. Source `VMULq` therefore
requires genuine caller-owned Q state; the original candidate
transform cannot be correctly implemented using only the
three polygon vertices and game RNG weights.

### Special VU0 random register operations

The former undecoded instructions at
`0x57CB90/94`, `0x57CBB0/B4`, `0x57CC88/8C`, and
`0x57CCA8/AC` are **VRINIT and VRXOR**, via the
original COP2 special2 codes `0x3A` and `0x3B`.

Each VRINIT instruction has source register `VF0`
and scalar selector **w**; `VF0.w` is the hardwired
float32 constant `1.0`, raw bits `0x3F800000`.
Each following VRXOR reads `VF0.x`, hardwired
`0.0`; XOR therefore leaves R's lower 23-bit mantissa
unchanged. Accordingly each complete source pair
writes `VU0 VI[R] = 0x3F800000`.

This is separate from the original EE game RNG
`OriginalGameRandom_36f3c0` / MT19937, which
generates **two scalar draws per candidate**.
For one six-candidate fallback selection, retail code
executes **twelve VRINIT and twelve VRXOR instructions**,
plus 12 original EE game RNG calls.

The source operations are now native C++ in
`original_actor_fallback_vu0_random.cpp`. The optional
original `OriginalActorFallbackVu0R_57ca68` state may be
bound to `originalActorFallbackSelect_57ca68`.
The selector preserves the final VI[R] effect after
each candidate without confusing it with MT19937 state.

The original VMOVE / VMULq / VU0 Q-driven candidate geometry
remains an explicit **unimplemented source transform**
(`OriginalActorFallbackVU0Transform_57ca68`).
No synthetic steering mathematics or VU interpreter has
been substituted. The current API allows a source-backed
transform to receive the caller's VU0 Q later, but its
provenance must be recovered and tested before that occurs.

## Validation

The source auditor was run directly on the owner's private
original ELF and verified all 173 code words plus the two
candidate operation sequences. The C++ regression checks
R-init and XOR semantics, observable R outcome and counts.
The integrated original fallback test verifies both
random-number systems independently and checks six
candidates against all **59 original woods_c polygons**.

Local GCC Release, Clang, ASan and UBSan tests passed,
including original private `woods_c/navigation.pck`
(SHA-256:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`).
The complete CMake/Windows program has NOT been built or
booted; original actor update logic and graphics remain
significant native-port blockers. No game-owned assets
were checked into public GitHub. No GitHub Actions
were manually invoked.

ISA cross-check: PCSX2 `R5900OpcodeTables.cpp` and
`VUops.cpp`, plus the PS2 VU instruction manual.
