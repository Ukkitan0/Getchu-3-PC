# Original Ape Escape 3 positive flagged movement, 0x36FB5C

## Retail USA source and scope

This work recovers another **original gameplay movement** path
rather than a new demonstration or replacement-engine mechanic.

Private genuine retail `SCUS_975.01` SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

Original `0x36FB5C..0x36FC28` source window:

- **51 genuine MIPS/EE instruction words**
- SHA-256 `eba23d30f1f7b8186abebf22a802722e6835be500a5286906420fb02c943c457`
- Genuine movement chunk constant `0x7157C4`:
  IEEE binary32 `0x3E000000` = **0.125f**
- Original source verifier:
  `native/tools/verify_original_movement_flagged_36fb5c.py`
  pins the window, selected individual opcodes, and source
  branch/JAL destinations

This particular native path is selected when original
movement `f12 > 0`, original child `+0xB4 < 1`,
and source word `child+0xB8` (scratch `a0+0xA8`)
has **bit 1** set. Otherwise the original `0x36FC18`
call to `0x36F8C8` needs to regenerate VU0 VF1.
The alternative source child-guest-address tag branch at
`0x36FB6C` is also not represented by this subpath.

## Native original arithmetic and state machine

`native/src/original_movement_flagged_36fb5c.cpp`
now executes a single original source pass using genuine
original VU0 VF1 lane words. It does **not** invent
movement coordinates or use a host-engine steering
component:

1. `0x36FB70 VMUL.xyzw` squares original VF1.
   `0x36FB80 VADDy.xyw` and
   `0x36FB84 VADDz.xyzw` accumulate the four
   final VF1 lanes, including the X norm
   `x²+y²+z²`.
   The resulting **modified VF1 register** is returned
   for required publication in the genuine original
   VU0 owner.
2. `0x36FB9C SQRT.S` computes length, and
   `0x36FBA0 MUL.S` multiplies by the original
   exact `0.125f` movement-chunk constant.
3. If current movement is within the chunk,
   `0x36FBB8 DIV.S` advances original child
   status by `movement / VF1_length`, and
   `0x36FBC0` clears original remaining f20.
   Source `0x36FB4C` then returns zero if status
   stays under 1.0.
4. If movement exceeds the chunk, source
   `0x36FC0C` subtracts the chunk and
   `0x36FC14` increments status by exactly
   0.125f. If status is still under 1.0, source
   `0x36FBE0` loops with positive remaining
   movement, clears the original flag and
   **requires original `0x36F8C8`** to regenerate
   VF1. The native partial pass returns a distinct
   continuation requirement, **not** a fake COP1 f0.
5. If status reaches/exceeds 1.0, source
   `0x36FBD4 BC1TL` executes the original
   branch-likely subtraction, `0x36FBEC`
   stores clamped 1.0 in original child+0xB4,
   `0x36FBE8` clears child+0xB8 flags,
   and `0x36FBF0/0x36FBF4` returns source
   overshoot movement from VF1 length and any
   remaining f20.

To avoid host/PS2 numerical divergence, the native
source subset accepts only finite ordinary source
binary32 values with **exactly representable** VU0
and COP1 intermediates, including SQRT/DIV.
It refuses inexact rounding, denormals, NaN, INF,
tagged guest pointers and the original alternate
source paths. This is a deliberately bounded original
function translation, not a general VU emulator.

## Integration with the real native child and owner

`OriginalMovementMetricOwner_557f34` now provides an
**optional** original VF1 source input reader and a
required VF1 register-publishing callback on this
opt-in source path. After the existing recovered
`0x557F34` metric/scale handoff:

- If original child flags+0xB8 bit1 are **zero**,
  the native function does NOT request a stale VF1:
  it delegates genuine `0x36F8C8`/movement through
  the existing original `call_original_36fb18` owner.
- If flags bit1 are set and an actual owner supplies
  all original child/status/movement registers, the
  native pass calculates the source transition.
- Only for a pass with a real original return
  does the owner first publish VF1, then commit
  child+0xB4 / child+0xB8 via
  `originalMovementCommitFlaggedReturn_36fb5c`.
  That typed commit **recomputes the original pass**
  and rejects arbitrary caller-fabricated, stale,
  or unfinished source results.
- If another `0x36F8C8` iteration is required,
  native source state stays unchanged and the full
  method is delegated to its genuine game owner.
  No original frame is marked completed here.

The existing `0x557F54+` animation/collision
continuation remains unresolved. A successful
`0x36FB18` movement result alone is **not**
a successful full original actor frame.

## Native regression and testing

New CTest targets are registered under
`native/unreal_bridge/CMakeLists.txt`:

- `OriginalMovementFlaggedPass36FB5C`: original
  one-eighth chunk, short movement, exact threshold,
  overshoot, long-step flag clearing, mandatory
  `0x36F8C8` continuation and invalid/inexact inputs.
- `OriginalMovementFlaggedChildCommit36FB5C`:
  typed source child writes, pass recomputation,
  stale state rejection and fail-closed source
  regeneration case.

Local **GCC and Clang C++20** builds with
`-O2 -Wall -Wextra -Werror -pedantic` and
UndefinedBehaviorSanitizer passed for the
source-equivalent isolated arithmetic regression,
typed child commit, and existing native metric
integration smoke tests. The original ELF 51-word
window and coefficient were rechecked locally.
The **complete CMake CTest suite and Windows/Unreal
editor build were not run**. The Windows game is
not yet playable.

## Blocking native original work remaining

Recover real original `0x36F8C8` input geometry,
VU0 vector generation, flag/tag branches and next-step
iteration. Then complete `0x36FB18` for nonexact
floating-point cases, the `0x557F54+` collision/
animation update, and authentic persistent Player/Monkey
game-object ownership.
