# Genuine Ape Escape 3 nonterminal movement metric and frame continuation

This tracks retail **SCUS_975.01** native movement recovery,
not an emulator, alternative gameplay controller, Unreal-generated
physics or a claim of a playable Windows game. Authoritative
original ELF SHA-256:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

## Three complete source windows verified

| Original machine code | Window | Count | Original full-window SHA-256 |
| --- | --- | ---: | --- |
| Resource float/VU0 `0x317B34` | `0x317B34..0x317B9C` | 26 words | `66e99b9cf7fadebdbb0a4befc31cdfce5aeefa3beba5c08787351c2ea9b09b4c` |
| Normal step `0x36FB18` initial return gates | `0x36FB18..0x36FB5C` | 17 words | `053051e501659048bcc73ce6ad95247f21f761436775fe59382283661eb506e3` |
| Child metric handoff `0x557F34` | `0x557F30..0x557F54` | 9 words | `1d6be51ae1fb85db3ac8f83bec9031bd1c7115621ca00b550263d984d8d41d71` |

The original source verifier
`native/tools/verify_original_movement_metric_557f34.py`
pins the complete windows, original COP2/COP1 opcodes, source
branch and JAL addresses, the `0x7157C0 = 1.0f`
literal, and original `0x557EB4 BNEZL` initialized-child
re-entry into `0x557F34`. The user-private original ELF
is **not** committed to the repository.

## Original `0x317B34` VU0 branch, not just the fast returns

Prior native code already handled two original direct returns:
`parent+0x40 == 1` returns the exact resource float at
`parent+0x08 resource table +0x374`, and source mode zero
returns the exact float at table `+0x3B4`.

The new `native/src/original_manager_resource_vu0_317b34.cpp`
recovers the genuine source nonzero-mode VU0 branch at
`0x317B58..0x317B90` for **exact ordinary source floats**:

1. Source `0x317B58 BBit032` has a separate tagged-parent
   continuation; the native subpath rejects such parent guest
   addresses rather than pretending to process them.
2. `0x317B5C` loads the genuine resource+0x364 COP1 f2.
   Source `0x317B60 VMUL.xyzw` squares the **actual VF1**
   input, `0x317B68 VADDy.xyw` adds the squared Y lane
   while leaving Z unchanged, and `0x317B6C VADDz.xyzw`
   adds the original squared Z lane, yielding VF1.x =
   `x² + y² + z²`.
3. `0x317B64` loads the original resource+0x368 COP1
   clamp into f1. `0x317B70 DMFC2` gets the VF1.x
   register bits, `0x317B84 SQRT.S` takes the square
   root, `0x317B88 ADD.S` adds the original f2,
   and **`0x317B90 MIN.S` in the `JR` delay slot**
   caps the returned COP1 f0 to the original f1.
4. The VU0 calculation also **modifies the original VF1
   register**. The native function returns all four
   resulting VF1 lanes. On this VU0 branch the
   `OriginalMovementMetricOwner_557f34` must publish
   them to the genuine game's live VU0 state **before**
   the caller proceeds. The two direct-return branches
   do not require this VF1 publication.

To avoid silently relying on the host's incompatible PS2
FP rounding, the recovered native implementation accepts
only finite, normal/zero source values with intermediate
operations verified **exactly representable** in binary32,
including the square-root and COP1 sum. Missing provenance,
tagged parent, NaN/Inf, subnormal, inexact operations and
unresolved signed-zero ties refuse with a native exception.
This does not replace the PS2's other floating-point
rounding/status modes with invented results.

## Original `0x36FB18` source early returns

The new
`native/src/original_movement_step_fast_36fb18.cpp`
recovers exactly the first two original return gates:

- Source `0x36FB40 C.OLE.S(1.0f,child+0xB4)`,
  `0x36FB44 BC1T`, and `0x36FB48 MOV.S f0,f12`
  return the **original raw input f12** when the
  original status has reached at least 1.0f.
- Otherwise `0x36FB4C MTC1 zero,f0`,
  `0x36FB50 C.OLT.S(0,f12)`, and
  `0x36FB54 BC1FL` return source **positive zero**
  when original f12 is not strictly positive,
  including when f12 is negative zero.
- For positive f12 below the status threshold, the
  native fast helper returns `std::nullopt` and the
  actual original engine owner **must** implement
  `0x36FB5C..` movement/bitflag/collision work.

The status comes directly from original child+0xB4
(original source `a0+0xA4`); unknown copied child bytes
or unsupported floating-point classes cannot be
interpreted as source zeroes.

## Original `0x557F34` handoff and persistent child re-entry

`native/src/original_movement_metric_step_557f34.cpp`
models the genuine source instructions:

- Original `0x557F30` reads child mode+0x09 and
  `0x557F38 JAL` invokes source `0x317B34`.
- `0x557F40` reads parent `+0x70` COP1 float
  (genuine source bytes), and `0x557F48 MOV.S`
  preserves returned f0 in f20.
- `0x557F4C JAL 0x36FB18` executes the
  `0x557F50 MUL.S` **delay slot**, passing scaled
  `f12 = resourceMetric × parent+0x70`.
- Source values and live VF1 side effects are returned/
  published by actual game-owner interfaces.
- If `0x36FB18` is not covered by its authentic
  fast return, source `call_original_36fb18` is
  **mandatory** and must not fabricate a success value.

One critical corrected lifecycle branch:
`0x557EB4 BNEZL` reads original child+0xC0.
When it is **zero**, the child runs the one-time
`0x557ED8` setup and is marked initialized. When it is
**nonzero (any nonzero byte)**, the original branches **directly to
`0x557F34`**, executing child-mode read in its
delay slot and skipping the initial resource/route
setup. The native `OriginalMovementChildLifecycle_557e30`
now preserves that distinction. On both paths, the
later `0x557F54+` collision, animation and actor frame
return are unrecovered and rejected; the caller does not
receive a fabricated completed actor tick.

## Native tests and blockers

New CTest targets registered in `native/unreal_bridge/CMakeLists.txt`:

- `OriginalManagerResourceVU0Exact317B34`
- `OriginalMovementStepFast36FB18`
- `OriginalMovementMetricPrefix557F34`
- `OriginalMovementNormalMetricLifecycle`

The locally reproduced native source path was compiled
under **GCC and Clang C++20**, using strict
`-Wall -Wextra -Werror -pedantic -fsanitize=undefined`.
Both compilers passed standalone metric, fast-step, and
persistent lifecycle test scenarios, including the
original initialized-child branch that no longer
restarts setup.

The full repository CTest suite, complete Windows
DLL and Unreal Editor/game startup were **not** executed.
Real Player/Monkey lifecycle ownership, `0x36FB18`
positive-movement collision paths, post-`0x557F54`
animation and native full-game startup remain blockers.


### Source parent identity invariant

The original `0x317B34` VU0 branch consumes state from
the **same parent** used by the resource table lookups.
The native `0x557F34` owner now rejects any supplied
`originalParentGuestVA` whose value differs from the
actual child parent pointer at original child `+0x04`,
**before** publishing new VF1 register state. A regression
checks this refusal and verifies no source VF1 publication
occurred. Neither arbitrary guest virtual addresses nor
host pointers are interpreted as valid related game objects.
