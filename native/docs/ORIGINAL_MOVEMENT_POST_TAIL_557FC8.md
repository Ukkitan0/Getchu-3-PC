# Ape Escape 3 USA — original movement post-tail and complete slot-5 statuses

## Source and newly recovered window

This update reconstructs **actual original MIPS control flow**, not a
replacement gameplay controller or an emulator. The owner's private
retail `SCUS_975.01` ELF SHA-256 is:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

The complete `0x557FC8..0x558030` **26-word** window SHA-256 is:
`aecf9edaedeeea8a27a5987bed707c47e9210d57ca411f95bfd1503fec0df6b1`.
The checked original-source verifier is
`native/tools/verify_original_movement_post_tail_557fc8.py`.
Original executable bytes stay private.

This window joins the other three pinned sections reconstructed in
this pass: `0x557F54..0x557FC8` (29 words), the original sibling
`0x36F83C..0x36F8C8` (35 words), and
`0x558030..0x558084` (21 words). All **111 original
instruction words** in those four complete windows have matching
source SHA-256 fingerprints in the supplied USA ELF.

## Exact source operations, preserved ABIs and order

`native/src/original_movement_post_tail_557fc8.cpp` reproduces the
real original control flow after the owner has published the
`0x557F54` VU0 VF1/VF2/Q and complete 128-bit EE s3 state:

1. `0x557FC8` reads original child flags at `+0xB8`.
   If bit 0 is **zero**, the source branch calls genuine
   `0x36F83C` at `0x558084` and returns a complete 128-bit
   VF1 into EE v0. `0x558090` MMI3 `POR` moves all 128 bits
   to EE a1. The source-accurate native
   `originalMovementRegenerateFlag0Owned_36f83c` publishes
   VF1/VF4/ACC and sets only source bit 0.
2. If bit 0 was already **set**, `0x557FDC LQ` loads all
   four original words at child `+0x90`; `0x557FE0`
   `POR` passes this exact 128-bit vector as EE a1.
   The native helper never substitutes a new scalar-derived vector.
3. Source `0x557FE4 JAL 0x30C0A0` passes the genuine
   original parent and packed source EE a1. Its original
   32-bit v0 is kept as **s5**, rather than fabricated.
4. Source `0x557FF4 JAL 0x276538` passes actual parent
   `+0x14`. Its JAL delay slot at `0x557FF8`
   uses `POR` to pass **all 128 bits** of EE s3
   (the exact `0x557F8C DMFC2 VF2` output) into EE a1.
   Previous scalar-only native s3 representation was corrected
   to carry all four source words in
   `OriginalMovementPostVector_557f54::originalGprS3VF2`.
5. The original diagnostic gate checks BSS `0x6354E7`.
   Only when nonzero does it read BSS `0x6354E0`.
   If that second byte is zero, source
   `0x558024 JAL 0x36FC28` receives the genuine guest
   child+0x10 pointer. All side effects belong to the
   actual original game-owner method, not invented native collision.
6. The next source `0x558030..0x558084` control block
   is already natively recovered. For child progress
   `+0xB4 >= 1.0f`, it divides source **f21/f20**,
   stores the result in actual parent `+0x70` and
   returns original controller status **2**. Otherwise it
   calls original `0x314C44(parent+0x1C,s5,child+0x09)`
   and returns controller status **0**.

The new
`OriginalMovementPostTailOwner_557fc8` demands genuine original
`0x30C0A0`, `0x276538`, `0x36FC28`, and
`0x314C44` game-owner callbacks. The owner must also
provide an authoritative guest child+0x10 VA; guest virtual
addresses must not be reinterpreted as Windows host pointers.

This is a complete original-source **caller/control-flow**
translation for the selected, source-backed ordinary path; the
external manager callee implementations are still required for real
gameplay.

## Existing lifecycle integration, preserved concurrent changes

`OriginalMovementChildLifecycle_557e30` can opt into the new
`OriginalMovementPostTailOwner_557fc8` as a final
constructor argument (after the existing optional original
`OriginalMovementPostStepContinuation_558030`).
The two interfaces are preserved independently:

- If a real post-tail owner exists and the preceding
  original `0x557F54` VF state was published, the native
  lifecycle executes **all recovered** `0x557FC8`
  operations and returns original slot5 status **0** or **2**.
- If no post-tail owner is attached, the pre-existing explicit
  `0x558030` continuation can still operate, but must
  certify its own genuine upstream source callbacks. This
  interface is not removed or silently bypassed.
- Earlier `0x557F54` normalized-vector status **1**
  returns immediately, skipping the source post-tail.
- The existing original `0x3161B8` update wrapper
  keeps children alive for status 0, and deletes the child
  on original nonzero status 1 or 2.

The new source code preserves output state from genuine VF registers,
source child-byte initialization/provenance and the original branch
delay-slot callback ordering. It never reports an actor update when
the game owner's original methods are missing or refuse.

## Tests and remaining blockers

CTest `OriginalMovementPostTail557FC8` covers both
flag-bit-0 branches, genuine 128-bit argument values to
`0x30C0A0/0x276538`, conditional diagnostic call, original
status 0/2, and rejected early status1 or failed real VU
publication. It passed local GCC and Clang C++20 UBSan smoke tests.

CTest `OriginalMovementPostTailLifecycle557FC8` additionally
exercises the true `0x557E30` slot5 → `0x3161B8`
controller path over **two updates**, verifying that first-time
resource/route setup and flag0/bit1 VU0 regeneration do not repeat.
It is registered in CMake, but a complete repository-wide
Windows/Unreal build has **not** been executed.

This pass still does not recover the original game-owner
callee **bodies** `0x30C0A0`, `0x276538`,
`0x314C44` and optional diagnostic `0x36FC28`.
Those depend on genuine stage, actor collision, animation and
navigation state. Completing them and the full original game
startup remains essential. This is not yet a playable native
Windows Ape Escape 3 port.
