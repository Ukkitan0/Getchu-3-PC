# Ape Escape 3 USA original post-movement controller: 0x557F54 and 0x558030

## Source provenance and verification

All of this code comes from the genuine, user-private **USA
`SCUS_975.01`** executable, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The original game executable is not committed to GitHub.

| Original source | Original window hash | Words verified |
| --- | --- | ---: |
| `0x557F54..0x557FC8` | `9aaaa5e8ebb1c3ddb3612f6271fc20dd137f49da76eabdeaaf70918ba9df0020` | 29 |
| `0x558030..0x558084` | `e4d59bf362eda8e908c4ffc4167674760b73b59f6d78710be2e9efddc9cfc3e4` | 21 |

All **50 source instruction words** are individually pinned by
`native/tools/verify_original_movement_post_vector_557f54.py`
and `native/tools/verify_original_movement_post_step_558030.py`.
Both verifiers check original branch targets and the original
constants: `0x704B08 = 0x3F733333` (approximately 0.95)
and `0x704B0C = 0x3F800000` (exactly 1.0).

## Original VU0 normalized vector and status 1: 0x557F54

The new `native/src/original_movement_post_vector_557f54.cpp`
reads actual copied game-vector data from the original
`0x30C57C` typed child **`+0xA0..+0xAF`** and confirms
that every consumed source byte has real-game provenance.
It does **not** fabricate character coordinates.

1. Source `0x557F54 LW child+0xB8`, then checks bit 1.
   If the bit is zero, the retail branch runs
   `0x558094 JAL 0x36F8C8` and returns to the original
   VU0 calculation. That regeneration is independently
   recovered in `originalMovementRegenerateOwned_36f8c8`;
   this helper only accepts the already-regenerated case.
2. Source `0x557F64 LQ child+0xA0` loads four real
   source float lanes, then `0x557F68 DMTC2` installs
   them as VF1. The source `VMUL.xyzw`, `VADDy.xyw`
   and `VADDz.xyzw` instructions at
   `0x557F6C/70/74` accumulate the squared XYZ norm
   in VF2.x.
3. Original `0x557F78 VRSQRT` and `0x557F7C VWAITQ`
   prepare Q, and source `0x557F80 VMULq` writes
   **VF2 = VF1 × Q**. For supported exact finite
   operands, this is a normalized original vector.
4. Source `0x557F84 LBU child+0x08` determines
   whether to run the second dot-product. The
   branch-delay instruction `0x557F8C DMFC2`
   unconditionally publishes VF2.x to original
   GPR s3. When the original mode byte is zero, the
   second comparison is skipped.
5. For nonzero mode, source `0x557F90 BBit032`
   chooses a separate tagged-pointer branch if
   applicable. The ordinary untagged branch executes
   `0x557F98 VMUL.xyzw VF1,VF1,VF2`,
   `0x557F9C VADDy` and `0x557FA0 VADDz`.
6. `0x557FBC C.OLT.S` tests the resulting original
   VF1.x against `0x3F733333`. On true,
   `0x557FC0 BC1TL` returns to the original
   `0x557E88` slot5 epilogue, with source status
   **1** from `0x557F94`. Otherwise genuine source
   gameplay continues at `0x557FC8`.

The native `OriginalMovementPostVectorOwner_557f54`
is an explicit game-owner interface that must publish
the original VF1, VF2, Q and GPR s3 outputs **before**
the status is returned. With this owner provided, the
existing `OriginalMovementChildLifecycle_557e30`
can return source status 1; the already-recovered
`0x3161B8` update then stores source status 1 and
deletes the child through the original slot1 path.
Without it, existing callers retain their previous
fail-closed gameplay behavior.

The normal vector arithmetic is supported only for
finite original binary32 inputs whose VU0 SQRT,
reciprocal and stage operations are provably exact.
Actual PS2 rounding/flag modes, zero-vector VRSQRT
and tagged-parent branches are not approximated.

## Original post-movement terminal and nonterminal status: 0x558030

The new `native/src/original_movement_post_step_558030.cpp`
reproduces the complete source **21-word control-flow block**
after preceding original `0x30C0A0` and
`0x276538` methods have executed:

- `0x558030 LWC1` reads genuine child progress
  `+0xB4`, and `0x55803C C.OLE.S` compares
  the original **1.0** threshold to that value.
- If **progress ≥ 1.0**, original `0x558060 DIV.S`
  computes `f0=f21/f20`, `0x558064` sets slot5
  return status **2**, and source
  `0x558068` returns via the original epilogue.
  The branch-delay instruction `0x55806C SWC1`
  stores exact source COP1 result to actual
  **parent `+0x70`**.
  The typed owner implements that write; the
  original guest pointer is never cast to a host
  pointer. The native subpath rejects unsupported
  PS2 divide-zero, special values and inexact
  rounding instead of silently inventing behavior.
- If **progress < 1.0**, original
  `0x558050 BEQZL` takes its branch and
  **only then** runs `0x558054` delay-slot
  `LW a0,parent+0x1C`. Genuine source
  `a1=s5` must come from the earlier
  `0x30C0A0` call, and source
  `0x558074 JAL 0x314C44` passes actual
  child mode `+0x09` in its delay slot.
  Original `0x558080` returns slot5 status
  **0** after that genuine callback.
- Missing `s5` provenance, unknown original
  child data, unsupported floating-point inputs,
  and actual callbacks that fail are refused,
  with no fabricated successful actor update.

Crucially, **this is not a claim that all preceding actor
work is complete**. The source path
`0x557FC8..0x558030` still invokes actual
`0x36F83C`, `0x30C0A0`,
`0x276538` and optional `0x36FC28`;
those effects still need source-backed integration,
as does nonterminal collision/animation beyond
`0x557FC8`.

## Testing and remaining blockers

Three native CTest registrations cover this pass:

- `OriginalMovementPostVector557F54`: original
  normalized VF1/VF2, Q, the 0.95 status1
  boundary, byte+8 selection, source provenance
  and tagged-pointer/zero-vector refusals.
- `OriginalMovementPostVectorLifecycle557F54`:
  real 557E30→557ED8→557F34→36F8C8→557F54
  status1→3161B8 deletion; nonterminal
  reentry without repeated initialization and
  failed original VU0 publication.
- `OriginalMovementPostStep558030`:
  original terminal status2 and parent+70
  ratio, ongoing status0/314C44 owner call,
  source delay-slot field ordering and
  unsupported source-value refusals.

Local native regression implementations compiled and passed
under **GCC and Clang C++20**, strict warnings and
UndefinedBehaviorSanitizer. The original source instruction
windows and constants match the real private USA ELF.
The full repository test suite, Windows DLL build,
Unreal Editor build and playable Ape Escape 3 game
remain **unverified**.
