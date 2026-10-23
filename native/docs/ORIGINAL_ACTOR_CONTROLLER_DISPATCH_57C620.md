# Original Ape Escape 3 USA actor controller — source-verified 0x57C620 + 0x30FFD0

The native code in this pass is recovered directly from the **user-owned
original retail USA `SCUS_975.01`**, extracted from that user's previously
provided ISO parts. No original executable, ISO, game assets or proprietary
instruction bytes are added to GitHub.

Original source ELF SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Verified original source and addresses

- **0x57C620–0x57C7CC**: 107 MIPS/EE instruction words, complete window
  SHA-256 `d15f6ebe4f862da7c50c7dfc08087cf12ab4a90cb38fc3231eb876987a61cc54`.
  Original controller vtable entry at **0x6B1834** points to `0x57C620`.
- **0x30FFD0–0x31009C**: 51 instruction words, complete window SHA-256
  `141b7623d8668917a8e3826bc39372ad9a5501596b03602ad4358edc044861c5`.
  Real original calls `0x310010 -> 0x21B788`,
  `0x31002C -> 0x21D898` and `0x310070 -> 0x21B788`.
- **0x57C6C0 -> 0x30FFD0**: original actor controller invokes polygon
  selection on source parent+0x04 with candidate original polygon+0x50.
- Original `0x708A50..0x708A54` method sentinel is zero/zero. Source
  `0x706304` is IEEE754 0.1f / bit pattern `0x3DCCCCCD`, not 0.0f.
- Original diagnostic calls `0x2478EC` and `0x247D84` are reached
  only when the original candidate+0x50 remains after a source
  dispatch returned zero AND owner runtime flags `0x6354E7 != 0`
  and `0x6354E0 == 0`.

Privately executable-checked verifier scripts:
`native/tools/verify_original_manager_dispatch_57c620.py` and
`native/tools/verify_original_actor_polygon_30ffd0.py`. Both require
the same hash-pinned original USA executable and fail on deviations.

## Original controller virtual method 0x57C620

`native/src/original_manager_dispatch_57c620.cpp` reconstructs the
source branch/side-effect order:

1. A zero controller source method field `+0x08` returns **2** without
   dispatch, even when `+0x0C` is nonzero.
2. An existing child at source `+0x10` forces original method dispatch.
   Otherwise `a1 & 3` bypasses dynamic dispatch and yields original
   return **1**; if both bits are clear, the original method runs.
3. The original `+0x0C` low bit chooses **virtual** or **direct**
   dispatch. Source method `+0x08` is a direct original function VA or
   virtual-method byte offset rounded down to a multiple of four.
   The original signed SRA-one subobject offset is preserved. These
   source method identities must be resolved by the real native actor
   owner, never called as guest addresses.
4. If source `+0x50` has an incoming polygon candidate and the source
   result is nonzero, the original invokes `0x30FFD0` on its parent and
   then clears `+0x50`. **This is not a generic resource free.**
5. If the pending source candidate remains, read original diagnostic
   flags in source order and invoke the original warning/initialization
   behavior only when those runtime flags request it. Missing original
   diagnostics fail closed rather than silently skipping them.

The source branches are translated. The dynamic source method bodies,
game-owned object offset resolution, global diagnostic state, and
actual persistent Player/Monkey owner still require integration.

## Real actor current polygon function 0x30FFD0

`native/src/original_actor_polygon_selector_30ffd0.cpp` reconstructs
the original control flow in source order:

1. When an incoming original polygon is supplied, source
   `0x21B788` tests it with **0.1f tolerance**. If eligible, it
   becomes actor's current polygon at `+0x30`.
2. If the incoming polygon is rejected AND original current is null,
   the function returns null immediately, without attempting nearest.
3. Otherwise a still-current original polygon is tested with source
   `0x21B788` and the same 0.1f margin. Eligible current is retained.
4. When the required earlier source checks fail or current is null
   with no incoming polygon, call genuine `0x21D898` nearest from
   original actor+0x28 registry. Only a nonnull nearest polygon
   replaces actor current+0x30; failure does not clear prior state.

The exact two original ray/query scalar operands and source VU0 actor
query must come from the real game-owned actor state and spatial grid.

`ae3_original_actor_polygon_select_30ffd0` connects this function
to the already source-decoded, session-owned original
`0x21D46C` stage data. It invokes actual recovered
`0x21B788` VU0 geometry and the source `0x4E3C14 -> 0x21D898`
native grid/nearest selection. It returns a source polygon index,
not an unreal Actor or host pointer. Actual actor+0x30 updates and
controller+0x50 lifecycle publication belong to the real owner.

## Verification status and limitations

The original user-owned executable SHA-256, both full source function
windows, original vtable entry, fixed tolerance, source opcode anchors
and original JAL targets were checked locally. Focused native C++20
source-flow smoke tests passed on local **GCC and Clang** with
`-Wall -Wextra -Werror -fsanitize=address,undefined` for both new
functions. These source-flow tests use test owner callbacks, not game
actors. The tests registered in `native/unreal_bridge/CMakeLists.txt`
also cover original dispatch and incoming/current/nearest polygon
branch order.

No complete Windows DLL/Unreal Editor build was executed and no
real gameplay boot or live Player/Monkey native actor update is
claimed. The original Lua bindings, full dynamic actor methods,
graphics/animation calls, and source-complete warning side effects
remain required before calling this a playable original port.
