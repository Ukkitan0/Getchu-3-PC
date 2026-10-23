# Original USA 0x36F5F4 movement scratch: verified source recovery

This file documents a **retail executable-verified** part of Ape
Escape 3's native movement controller. This is not emulation, a
replacement gameplay engine or a complete Player/Monkey update.

## Original source and provenance

- Owner-provided source: original USA `SCUS_975.01` retrieved from
  the project's private handoff archive, kept off GitHub.
- ELF SHA-256:
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
- Original `0x36F5F4..0x36F6DC` initial entry-to-return window:
  **58 instructions**, SHA-256
  `1a3f90814dc34de36195e7d7718d1b0d906e365020117633f9f8967218df3de8`
- Original caller `0x57C120..0x57C164`: 17 instruction
  words, SHA-256
  `6ce5da0f7c29c442276734d4ef87501b483e7c3f242c3cbf61fe721022b81112`

Original caller `0x57C120` establishes `s0=sp+0x20`;
`0x57C138` passes `a0=s0`, `0x57C13C` explicitly
sets `t0=0`, and `0x57C148` invokes `0x36F5F4`
with PS2 COP2 vector operands packed in the register
sequence ending at the call delay slot `0x57C14C`.
This original stack VA is a 32-bit positive, 16-byte-aligned
EE address, not a host C++ stack address.

## The actual 0x36F5F4 direct-return path

The original `0x36F5F4` entry transfers the genuine
`a1/a2/a3` vector data to VU0/COP2. It performs multiple
vector subtractions, multiplications, accumulations and source
COP1 comparisons. At `0x36F678` it sets `t3=2`,
and `0x36F680 MOVZ t3,zero,t0` changes `t3`
to **0**, because the verified original `0x57C148` caller
sets `t0=0`.

The five source `BBit132` branches at
`0x36F6A0`, `0x36F6A4`, `0x36F6B4`,
`0x36F6B8`, `0x36F6BC` test source
`a0`'s **upper 32 bits**. For the genuine positive
32-bit EE stack VA from this specific caller, all five
are untaken. Their other targets remain separately
identified in the source verifier; this is **not**
a claim that those branches can be removed from
`0x36F5F4` globally.

For the verified `t0=0` + positive source-stack-VA
path, the source's exact output tail is:

| Original opcode site | Original effect |
| --- | --- |
| `0x36F6A8` | `SW zero, a0+0xA8` |
| `0x36F6AC` | `SW zero, a0+0xA4` (source `t5=0`) |
| `0x36F6B0` | `SW 0x00744DD0, a0+0xA0` |
| `0x36F6C4` | `SW zero, a0+0xAC` (original `t3=0`) |
| `0x36F6C8` | `BC1F` using `C.OLT.S f1,1.0f` |
| `0x36F6D0` | If ordered `f1 < 1.0f`, `SWC1 1.0f, a0+0xA4` |

The source `0x7157AC` constant is actual
`0x3F800000` (1.0f). The `0x744DD0` pointer
is in original **.bss**: it is *not* a file-backed
static table that can be dereferenced from the ELF.
`0x36F690` multiplies the source selector by
**128 bytes**; `0x36F694` forms this original
guest runtime pointer. The native code deliberately
stores the original 32-bit guest VA, not a Windows
host pointer.

## New native code and tests

`native/src/original_movement_scratch_tail_36f6a8.cpp`
executes exactly that source-bounded scalar/store
sequence over the **full existing** 0x110-byte
`OriginalMovementScratch_57bf5c` frame. Its
caller supplies the original 64-bit guest `a0`
stack address, source `t0` and the raw original
VU0/COP1 `f1` bits.

It **rejects** missing source callsite invariants
(`t0 != 0`, high32 guest-address bits nonzero,
zero/unaligned source stack address) without
changing the frame. It uses raw IEEE-754 bits
for the original ordered C.OLT.S result: NaNs
cause the optional `+0xA4` store to be
skipped, and equality to 1.0f is also skipped.

`OriginalMovementScratchTail36F6A8` is the new
CTest registered in `native/unreal_bridge/CMakeLists.txt`.
A local smoke test compiled and passed under GCC
and Clang C++20 with `-O2 -Wall -Wextra -Werror
-pedantic -fsanitize=undefined` and checked
positive, equality, NaN, infinity, source guest
pointer constraints, and unchanged surrounding memory.
The new **whole repository CTest/Windows DLL/Unreal
Editor executable** has not been verified here.

`native/tools/verify_original_movement_scratch_36f5f4.py`
re-checks original ELF SHA, exact callsite/window hashes,
source instructions, original runtime-BSS location,
cross-entry branch targets and 1.0f constant.

## Still required to complete the real original 0x36F5F4

The original VU0 vector calculations that produce the source
`f1` and runtime vector register state are **NOT**
fully translated. The routine also has branch targets
at `0x36F6E4`, `0x36F728`, `0x36F7B8`,
`0x36F7FC` and `0x36F840` on other
source address/tag cases; their semantics cannot be
inferred from just the 58-word first window.

The existing `OriginalManagerMovementCalls_57bf5c::call_original_36f5f4`
still MUST be provided by an actual source game owner until
the full VU0 pipeline and all necessary live operand values
are natively recovered. The new scalar-tail function is a
verified building block for that source implementation,
not a fake replacement for the unresolved VU0 portion.

The original `0x30C57C` child creation, actor movement
and collision, persistent Player/Monkey game ownership,
animation and native Windows startup remain additional
blocking dependencies.


## Source-recovered original VU0 VF1/VF2 scalar arithmetic

The source `0x36F5F4..0x36F664` was examined directly in the
hash-pinned original USA executable. The actual original COP2 macro
instructions are now translated in
`native/src/original_movement_vu0_scalar_36f5f4.cpp` for finite
ordinary values (including zero). No general-purpose VU emulator
or substitute gameplay distance query is involved.

The input is the **original 128-bit EE source a1/a2/a3 vectors**,
not reconstructed host physics coordinates. These are packed at
`0x57C140`, `0x57C144`, and `0x57C14C` by the real controller.
Original `0x36F5F4`, `0x36F5FC`, `0x36F604` transfer them
to VU0 VF3, VF4, VF6 respectively.

| Original PC | Original COP2 opcode | Recovered operation |
| --- | --- | --- |
| `0x36F608` | `0x4BE3206C` | `VSUB.xyzw VF1,VF4,VF3` |
| `0x36F610` | `0x4BE330AC` | `VSUB.xyzw VF2,VF6,VF3` |
| `0x36F618` | `0x4BE1086A` | `VMUL.xyzw VF1,VF1,VF1` |
| `0x36F61C` | `0x4BE210AA` | `VMUL.xyzw VF2,VF2,VF2` |
| `0x36F620` | `0x4BA10841` | `VADDy.xyw VF1,VF1,VF1` |
| `0x36F624` | `0x4BA21081` | `VADDy.xyw VF2,VF2,VF2` |
| `0x36F628` | `0x4BE10842` | `VADDz.xyzw VF1,VF1,VF1` |
| `0x36F62C` | `0x4BE21082` | `VADDz.xyzw VF2,VF2,VF2` |

For the X result consumed by the source, these instructions compute
the ordered single-precision sums of squared X/Y/Z differences.
`0x36F630` copies the VF1.x bits for the *first* source
`C.OLT.S` at `0x36F640`. `0x36F63C` copies VF2.x;
`0x36F664` installs those bits into COP1 f1, which is
compared to 1.0f at `0x36F698`. **VF2 comes from source
A3 minus A1**; mistakenly forwarding the first A2/A1
distance would change the original movement status.

The implementation preserves the VU source ordering:
VSUB, VMUL, VADDy, then VADDz, never a fused vector dot
product with unspecified reassociation. It does not include
W in the X reduction. Unsupported IEEE/PS2 edge conditions,
such as subnormal, nonfinite and overflowing XYZ lane operations,
are refused instead of being guessed to match the PS2's
nonstandard floating-point behavior.

The native helper
`originalMovementScratchFromVU0Scalar_36f5f4` connects
the recovered VU0 VF2.x bits directly to the existing
`originalMovementScratchScalarTail_36f6a8` for the genuine
`0x57C148` callsite, including its original `t0=0`,
guest stack-address restrictions and `+0xA0/+0xA4/+0xA8/+0xAC`
source writes. The native helper does **not** invent any of the
other 0x110-byte original scratch data.

Two more native CTest targets are registered:
`OriginalMovementVU0SourceScalar36F5F4` validates
both source vector pairings, W independence, comparison
boundary and unsupported floating-point inputs;
`OriginalMovementVectorScratchChildChain` executes the
source sequence **original EE vectors → VU0 VF2.x →
0x36F6A8 scratch → original 0x30C57C child copy**, including
unchanged output on invalid inputs. Both tests passed
locally under GCC and Clang C++20 with
UndefinedBehaviorSanitizer; the full Windows DLL and
Unreal runtime have not been tested.

The exact eight original VU0 arithmetic opcodes, their
operand registers, masks, and low-six-bit opcode IDs
are now additionally pinned by
`native/tools/verify_original_movement_scratch_36f5f4.py`.
Its source-opcode fingerprint
`b24dbd9ee6c08d78397e803f4e25b1591b95f2b735215ae7f115e66dffed1840`
was checked against the user's genuine USA ELF.

### Follow-up source VU0 register outputs

The separate
`originalMovementVU0VectorFrame_36f5f4`
native path now recovers the ordinary-float branch and
register outputs **after** source `0x36F640`.

The exact first `C.OLT.S` test compares VF1.x against
1.0f. When true, original `0x36F64C` places 0.5f in
VF1.x, the source `VMULx` pair scales original VF3
(A1) and VF6 (A3), and the source `VADD.xyzw` places
their midpoint in VF4. When false, VF4 retains genuine
A2. These results are returned only for ordinary finite
original vector lanes.

Original `0x36F660` zeroes t5; the two actual
`PPACW` operations at `0x36F668/0x36F66C`
produce zero t6 for `0x36F674` QMTC2 VF7.
Original `0x36F67C/0x36F684/0x36F68C`
then execute the source operations:

- VF3 = VF4 - original A1
- VF1 = original A3 - VF4
- VF2 = VF7(0) - original A3

This is the recovered 128-bit **source register output**,
not a guessed Unreal transform or movement animation.
All four XYZ/W lanes are checked on this path, unlike the
scalar-only norm helper that consumes XYZ.

The new native CTest
`OriginalMovementVU0SourceVectors36F5F4`
covers both exact source VF4 branches, VF1/VF2/VF3
outputs and invalid original W-lane refusal. Local
GCC and Clang C++20 UndefinedBehaviorSanitizer tests
passed. The source verifier now pins these additional
MMI, COP2 transfer and vector opcodes against the
actual USA ELF.

### Remaining original VU0 work

The **first-return path** has substantially more of its
VU0 scalar and vector arithmetic recovered, but the
original function is still not fully integrated into
a persistent actor game owner. Alternative cross-entry
paths, the original branch-specific hardware rounding,
overflow/underflow and VU0 status/MAC flags, and authentic
source input register ownership need further verification.
The native implementation refuses unsupported source
floating-point classes rather than assuming that host
IEEE-754 handling matches hardware.

The real original `0x36F5F4` engine callback is
therefore **not automatically replaced** in the live
movement-owner path. It would be incorrect to claim a
complete actor update while those remaining original
side effects and game-owner dependencies are absent.


## Source first-return integration and initialized-byte provenance

The now-recovered ordinary-operand `0x57C148 -> 0x36F5F4`
first-return path has a single native entrypoint:
`originalMovementVU0FirstReturn_36f5f4`. It computes and validates
**all four returned VU0 register vectors** before performing the
already verified original `0x36F6A8` scalar/store tail. This avoids
the previous scalar-only shortcut silently omitting the original
VF1/VF2/VF3/VF4 updates.

The source `0x36F5F4` first-return writes only four 32-bit scratch
words at relative offsets `0xA0`, `0xA4`, `0xA8` and `0xAC`
for the actual `0x57C148` callsite. It does **not** initialize
the remaining `0xA0` source bytes. The first-return native API
therefore accepts a `std::bitset<0x110>` known-byte mask and
marks **only** `0xA0..0xAF` as newly initialized after all
input validation and successful writes. Any previous independently
proven source-byte ownership is preserved. An invalid original
VU input or guest-stack address raises an error before mutating
either scratch bytes or the known-byte mask.

The original `0x30C57C` constructor indeed copies all `0xB0`
scratch bytes to original child `+0x10..+0xBF`, but a physical
copy of an *uninitialized* original source byte does not make
its value known. The new
`originalMovementChildCreate_30c57c(..., sourceKnown, ...)`
overload copies the retail fields while propagating the source
initialized-byte mask to the native child view. Attempting to
read unknown copied game bytes returns `std::nullopt` or
throws for a full unknown source word, instead of reporting
invented zero values. The existing 4-argument constructor
overload remains for genuinely **fully initialized** source
scratch supplied by a true game owner.

CTest `OriginalMovementFirstReturnScratchProvenance` exercises
the entire ordinary-float source chain, output vectors, scalar
word writes, preexisting known bytes, actual 0xD0 constructor
fields, unknown stack data, and atomic rejection of bad
source VU0 inputs/guest addresses.

An isolated local equivalent of the new path compiled and
passed using GCC and Clang C++20 with strict warnings and
UndefinedBehaviorSanitizer. The full new repository CTest,
Windows DLL and Unreal Editor have not been run here.
No original proprietary ELF/ISO bytes are included.
