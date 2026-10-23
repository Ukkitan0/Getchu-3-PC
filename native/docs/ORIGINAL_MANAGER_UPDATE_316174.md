# Authentic Ape Escape 3 USA controller update and child lifecycle

This document records source-recovered native implementations of the
original `SCUS_975.01` MIPS routines `0x316174` and `0x3161B8`.
Verified USA executable SHA-256:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

## Complete original machine-code windows

| Original routine | Source window | MIPS words | Full-window SHA-256 |
| --- | --- | ---: | --- |
| `0x316174` guarded virtual update | `0x316174..0x3161B8` | 17 | `4f5bf06a0806755a5ab50762e1af2357af0a401f63e1a8b4e7d8bea432584103` |
| `0x3161B8` owning child update | `0x3161B8..0x31622C` | 29 | `bc602d09c1633a68857d2ca1c7b1bb2133a3c8e75bd125b876230bc7f0e25cab` |

`native/tools/verify_original_manager_update_316174.py` checks
both full-window hashes and 23 exact instruction/opcode sites including
original `0x3161E0` JAL to `0x316174`, original virtual call
offsets, source game global `0x696C98` and the exact controller fields.

## Actual native source behavior

`native/src/original_manager_update_316174.cpp` implements:

- `originalManagerGuardedUpdate_316174`: Source virtual method
  slot **5** (vtable byte offset `+0x14`), called while the shared
  source update nesting global at `0x696C98` is incremented.
  The global is reloaded and decremented after source return. Native
  exception handling also restores it on missing engine methods.
  It returns the genuine source virtual method's full `v0`.
- `originalManagerChildUpdate_3161b8`: Original `a2`/`a3`
  low-byte gates bit 0 / bit 1 in original `a1`, respectively.
  It invokes genuine original `0x316174` on the active source
  child pointer at controller `+0x10` and writes its **full**
  return status to controller `+0x14`. Zero status returns zero
  and preserves the child. Any nonzero status re-reads the child
  pointer, executes original child vtable slot **1** (deleting
  destructor) when nonnull, clears `+0x10`, and returns one.

The source object pointer passed to a real virtual method must be
an actual native game owner. Guest 32-bit vtable pointers are not
callable Windows addresses. Unavailable original child methods
reject; they are NOT acknowledged with fake callback success.

## Connected native test coverage

`native/tests/original_manager_update_316174_test.cpp`,
CTest `OriginalControllerUpdate3161743161B8`, checks the original
flags, update result 0 and nonzero including 2, status publication,
slot-1 destruction, rereading the child after a synchronous callback,
nested shared depth, bad/missing owner, and no false cleanup on a
native owner exception.

The existing CTest
`OriginalRecoveredControllerMethodDispatchIntegration` additionally
calls `0x3161B8 -> 0x316174 -> 0x57C620` with an actual typed
native controller fixture, requiring source status 2, preserved
shared update depth and original child deletion.

Both isolated C++20 implementations and the connected native
dispatcher smoke test passed locally with GCC and Clang
`-O2 -Wall -Wextra -Werror -pedantic -fsanitize=undefined`.
The owner-private ELF matched both full function hashes and all
23 independent opcode sites.

## Boundaries

These are **complete source control-flow recoveries** for the two
short game functions but do NOT themselves implement the vtable[5]
body for every original Player/Monkey type or the underlying real
model/animation/physics state. Their owner callbacks MUST be wired
to true original engine objects. The complete retail stage runtime
and a playable Windows/Unreal build are not yet verified.
No original copyrighted ELF or ISO data is committed.
