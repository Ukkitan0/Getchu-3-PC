# Complete source-field reconstruction of original 0x30C57C movement-child constructor

## Verified original source

Genuine **Ape Escape 3 USA** executable `SCUS_975.01`,
SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
This executable was obtained from the user's previous private
project-handoff archive and has NOT been committed to GitHub.

Source method `0x30C57C..0x30C670` has **61 MIPS
instruction words**, full window SHA-256:
`9c9bd89de5dc50b3780c081f91aa5e826d89383b0bea95fb39b78bf35943d4a5`.

Original `0x57C150..0x57C164` callsite has original SHA-256
`6aacae3b2047d5a00fbbdefca10af0a64580551e9f700f2500dcb931bc6a8b13`.
Caller `0x57C15C` calls original constructor with:

- `a0`: genuine original game-owned parent (not a host pointer
  invented from `0x57BF5C`)
- `a1`: source 0x110-byte movement scratch from
  `0x36F5F4`, at original stack+0x20
- `a2`: **1**, set in the original JAL delay slot
- `a3`: original controller mode byte at `+0x1D`

The original constructor performs an original game allocation
`0x41B800(0xD0)`. `0x30C5D0..0x30C618` copies
two 0x40-byte scratch blocks, and `0x30C61C..0x30C650`
copies six remaining 8-byte words, for a total original
**0xB0 bytes** from scratch to the new object at `+0x10`.

## Exact original initialized fields

| Guest object offset | Verified original source write |
| --- | --- |
| `+0x00..+0x03` | Original vtable guest VA `0x006B0B20` |
| `+0x04..+0x07` | Genuine original parent guest VA |
| `+0x08` | `a2 & 0xFF`, source caller passes 1 |
| `+0x09` | `a3 & 0xFF`, original controller mode |
| `+0x0A..+0x0F` | **NOT initialized by this constructor** |
| `+0x10..+0xBF` | Exact 0xB0-byte copy of source movement scratch |
| `+0xC0` | Byte cleared to zero at original `0x30C658` |
| `+0xC1..+0xCF` | **NOT initialized by this constructor** |

`native/src/original_movement_child_30c57c.cpp` implements
all these original source operations, using an ordinary native
C++ heap allocation and a typed
`OriginalMovementChildD0_30c57c` source-data view.
The guest vtable and parent addresses remain **32-bit game
identity values**; they are NEVER dereferenced as Windows
function pointers.

The view tracks exactly which retail bytes were written,
refusing reads from bytes that the original constructor left
uninitialized. It does **not** zero-fill unknown source actor
state and pretend those values were present in the retail game.
The object retains its own 0xB0-byte data copy, so the
owner may release the caller's original stack scratch afterward.

`native/tools/verify_original_movement_child_30c57c.py`
pins all 61 original source instructions by their full-window hash
and checks 28 important individual opcode/caller sites.

## Original 0x36F5F4 -> 0x30C57C integration

This constructor is called directly after original `0x36F5F4`
fills the source 0x110-byte scratch. The native
`OriginalMovementScratchTail36F6A8` helper already recovers
the verified original scalar status, BSS pointer and selector
tail **for the real 0x57C148 callsite**.

The new CTest target
`OriginalMovementChild30C57CSourceCopy` exercises this
source-specific chain together: original caller conditions,
native `0x36F6A8` scalar output, the true 0xB0-byte copy,
retail vtable and parent guest address, byte-mode low masks,
original +0xC0 reset, uninitialized-byte tracking,
and scratch input lifetime. The isolated source translation
passed GCC and Clang C++20 `-O2 -Wall -Wextra -Werror
-pedantic -fsanitize=undefined` tests locally.

**Important distinction:** This completes original
`0x30C57C` **allocation/initialization data semantics**;
it does not decompile its vtable methods, give the
new object a live actor update or make a playable native
game. The real original `0x36F5F4` vector calculations,
original heap/actor lifecycle, child destructor methods,
route/collision, game animation, and native runtime
still require source-backed recovery.

No proprietary ELF, ISO, or original game resources
have been committed in this change.


## Data provenance and source child update recovery

The original first-return `0x36F5F4` routine actually writes
only source scratch **`+0xA0..+0xAF`**. It does not magically
initialize the entire `0xB0` bytes later copied by
`0x30C57C`. The source-exact native API now carries a
`std::bitset<0x110>` initialized-byte mask from
`originalMovementVU0FirstReturn_36f5f4` to the overloaded
`originalMovementChildCreate_30c57c`.
The constructor retains genuine source bytes and explicitly
withholds unknown copied bytes from callers. An unknown
source word cannot be read as a fake zero.

A separate original machine-code audit identified actual
new child vtable at **0x6B0B20**: its deleting
destructor slot1 is **0x556A0C** and child-update
slot5 is **0x557E30**. A new native owner recovers the
flag-driven status-one completion and genuine deletion chain
through `0x316174` and `0x3161B8`. It preserves
the source parent `+0x14` delay-slot load. The regular
per-frame `0x557EB4+` gameplay path remains unrecovered
and explicitly fails rather than claiming an update.

See `native/docs/ORIGINAL_MOVEMENT_CHILD_557E30_LIFECYCLE.md`
and CTests `OriginalMovementFirstReturnScratchProvenance`
and `OriginalMovementChildLifecycle557E30`.
