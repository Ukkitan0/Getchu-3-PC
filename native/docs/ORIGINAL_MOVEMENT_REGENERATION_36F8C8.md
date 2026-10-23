# Ape Escape 3 USA — original 0x36F8C8 movement-vector regeneration

## Authoritative evidence

This is a native C++ translation of the game's genuine
`SCUS_975.01` movement code, not a VU emulator, a substitute
physics controller or an Unreal-generated actor motion.

- Private original USA executable SHA-256:
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
- Original `0x36F8C8..0x36F954` source:
  **35 instruction words**, SHA-256
  `62aef8aedb73f27eb053c1f294af37dbb35f9550fc48f46293b38e2da2eeb7f6`
- All **35 original instruction words** verified individually,
  including **3 original EE MMI PPACW** instructions,
  **8 original VU0 SPECIAL2** instructions, and
  **9 alternate tagged-pointer branch targets**.
- Original source verification script:
  `native/tools/verify_original_movement_regenerate_36f8c8.py`

The original ELF is kept private, not committed to GitHub.
The VU0 instruction encoding was cross-checked against the
PCSX2 R5900/VU0 opcode names because generic llvm-objdump
displays these original hardware instructions as unknown.

## Recovered common untagged source branch

The new
`native/src/original_movement_regenerate_36f8c8.cpp`
reconstructs the ordinary source path through the original
`0x36F8C8` entry, assuming source EE child+0x10
guest address and resource-pointer `+0xA0` have zero
upper 32 bits and do not take the original BBit032/BBit132
tagged-pointer branches.

1. `0x36F8C8 LWC1` reads the original scratch
   progress at `+0xA4`. `0x36F8D4` squares
   that progress; `0x36F8E8` computes its cube.
2. The three original `PPACW` instructions at
   `0x36F8E0`, `0x36F908`, `0x36F90C`
   combine those genuine scalar bits and the literal
   `1.0f` into four raw VU0 lanes:

   `VF1 = VF4 = [progress³, progress², progress, 1.0]`

3. Original `0x36F918 VMULAx` consumes the
   **previous VF1** and the newly packed VF4.x,
   before `0x36F924 QMTC2` overwrites VF1.
   `0x36F92C/930/934` then apply original
   `VMADDAy / VMADDAz / VSUBAw` to produce
   the first source VU0 ACC stage.
4. Original `0x36F938 VMULAx` **replaces**
   the accumulator, then `0x36F93C/940/944`
   execute the second source accumulation using
   genuine VF8, VF7, VF6, VF4 and the polynomial.
   Both first-stage and final ACC lane values are
   preserved, rather than substituting host-vector math.
5. Original `0x36F91C ORI` and `0x36F920 SW`
   set scratch flags `+0xA8` **bit 1**.
   Original `0x36F94C JR` executes
   `0x36F950 DMFC2` in its delay slot; the
   original 128-bit v0 return equals VF1.

The native entrypoint accepts genuine source VF1–VF8
register lanes supplied by the game owner, not invented
character positions. All inputs and arithmetic are
limited to finite normal/zero binary32 values with
exactly representable intermediate operations. Source
special floating-point modes, inexact rounding and
other tagged-address paths remain unrecovered.

## Actual original child-data integration

`originalMovementRegenerateOwned_36f8c8`
connects this result to the previously reconstructed
0xD0-byte original `0x30C57C` child object.
It reads only source-backed byte-initialized fields:

- Original resource pointer at child `+0xB0`
  (scratch `+0xA0`).
- Original progress at child `+0xB4`
  (scratch `+0xA4`).
- Original flags at child `+0xB8`
  (scratch `+0xA8`).
- Original parent guest VA at child `+0x04`.

The real 0x36FC18 callsite regenerates only when
flags bit 1 was **clear**. The typed native implementation
requires that condition, requests original VF1–VF8 from
the same parent, computes the full native source result,
publishes the source VF1/VF4/ACC/v0 outputs, and then
writes the original child flag word. Unknown source
fields, repeated regeneration and a failed owner
publisher do not produce a successful flag update.

The new optional
`OriginalMovementMetricOwner_557f34::read_original_regeneration_binding_36f8c8`
connects this real owner to the already recovered
`0x557F34 -> 0x36FB18` movement path. The native
controller invokes source `0x36F8C8` on the original
bit-1-clear route, sets the source register state,
and resumes the **actual owner callback** for the
remaining movement. Subsequent bit-1-set updates
do not repeat regeneration. If the original source
binding is not available, the existing full original
`call_original_36fb18` callback is used unchanged.

**This does not implement the rest of 0x36FB18.**
The original 0x36FB70 onward positive-movement
calculations still have separate unsupported cases
and the original 0x557F54+ actor animation/collision
continuation is incomplete.

## Regression coverage

Three CTest targets were added to
`native/unreal_bridge/CMakeLists.txt`:

- `OriginalMovementRegeneration36F8C8`:
  original polynomial/PPACW word ordering, old VF1
  read before write, ACC accumulator ordering, raw
  VF return and guest-address tag rejection.
- `OriginalMovementRegenerationOwned36F8C8`:
  genuine child/parent source, flag update after
  VU publication, no second regeneration, missing
  original bytes and owner failures.
- `OriginalMovementRegenMetricIntegration36F8C8`:
  source 557F34 → 36FB18 bit-1-clear →
  36FC18 → 36F8C8 → flag bit1 set →
  original 36FB18 owner; no repeated regeneration
  and failed original publication handling.

Locally equivalent source/tests passed under
**GCC and Clang C++20**, strict warnings and
UndefinedBehaviorSanitizer. The original ELF
window and all original instructions were
independently rechecked locally.

The full repository CMake/CTest build, native
Windows DLL and Unreal Editor, and a playable
Ape Escape 3 stage/game frame are **not verified**.
Tagged-address original 36F8C8 branches, hardware
floating-point special cases, the full original
36FB18 positive movement/iterative collision and
original 557F54+ actor animation remain blockers.
