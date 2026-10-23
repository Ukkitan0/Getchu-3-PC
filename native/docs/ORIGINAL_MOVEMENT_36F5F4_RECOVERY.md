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
