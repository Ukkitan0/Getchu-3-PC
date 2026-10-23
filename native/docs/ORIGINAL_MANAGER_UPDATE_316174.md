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


## Source-owner integration without duplicate child lifecycle code

Two components now make the original manager controller's repeated
`0x3161B8` and `0x316174` calls consistent:

1. `native/include/ae3/original_manager_tick_bridge_3161b8.hpp`
   binds a **specific** `OriginalManagerChild70` to the game's
   shared `OriginalUpdateDepth_696c98` and actual
   `OriginalManagerUpdateOwner_316174`. It checks that the
   pointer-to-child+0x10 passed by a translated controller method
   belongs to exactly this original controller. It delegates to
   translated `originalManagerChildUpdate_3161b8` with original
   `a2=a3=0`. The original `0x316174` controller continuation
   invokes the same real virtual slot5 owner, with the same global
   depth and a true controller address. No host guest-VA casts.
2. `native/include/ae3/original_manager_controller_owner_base.hpp`
   shares that adapter across **all five** original controller
   actions (`0x57BF5C`, `0x57C27C`, `0x57C388`,
   `0x57C48C`, `0x57C554`). The common original child update,
   controller continuation and deleting destructor overrides are
   `final`; a game owner cannot accidentally implement a second
   inconsistent copy for another controller action. Its
   `nativeMethodOwner` method connects the same object to the
   recovered `0x57C620` dispatcher.

Actual per-game construction methods, actor VU0 data, original
`0x36F5F4`, collision, Lua event binding, and animation remain
unimplemented pure virtual/game-owner responsibilities; this
base cannot be instantiated without concrete genuine source
implementations. It does **not** create replacement game objects.

Native regression targets:

- `OriginalControllerTickBridge3161B8` checks status, child
  deletion, controller continuation, global nesting, source
  pointer identity and real owner exceptions through the recovered
  original `0x57C554` update.
- `OriginalSharedControllerOwner3161B8` composes recovered
  `0x57C620 -> 0x57C27C -> 0x57C388 -> 0x57C48C`
  including nested `0x316174` calls and then a `0x57C554`
  update, all using a SINGLE original `+0x10/+0x14` lifecycle
  and source depth counter. Test callbacks are fixtures, not
  an implementation of retail Player/Monkey behavior.

In this pass, the isolated common tick adapter behavior passed
local GCC and Clang C++20 UndefinedBehaviorSanitizer smoke runs;
the five-inheritance native owner interface compiled in
isolated GCC and Clang checks. The **repository's full new
integration CTest targets and Windows/Unreal DLL have not been
executed here**, and the original retail `SCUS_975.01` executable
was not available locally for new source-disassembly claims.
