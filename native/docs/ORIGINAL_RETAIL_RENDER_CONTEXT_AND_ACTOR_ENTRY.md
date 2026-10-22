# Original USA renderer — context stores, actor resource, entry and effect chain

Source: owner-private **USA SCUS_975.01**, exact SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Retail ELF32 little-endian, load mapping: file `0x1000` to VA `0x100000`.
No game binary, original models, scripts or game bytes are committed.

## Critical corrected visibility branch

Original `0x280874` calls `0x26C9B8`, and `0x28087C` is
`beqzl $v0, 0x280958`: **a ZERO result SKIPS rendering**. Secondary
`0x280E64` calls the same routine and `0x280E6C` likewise branches on
ZERO to `0x280EAC` (skip). This is the opposite of the previous
`rejectByOriginalBounds` callback interpretation. All graphics-owned
native visibility/pass implementations now name the provider
`originalBoundsVisible` and **advance when nonzero**.
The primary pass invokes it only when actor byte +0x32 is set; the
secondary pass invokes it unconditionally once +0x30 and +0x35 pass.
The VU0-dependent body `0x26C4B0`, entered by wrapper `0x26C9B8`,
still belongs to the VU0/Physics workstream. The source branch result is
verified even though that body has not been ported in graphics.

## Retail context fields recovered as native typed data

The original functions below do **not** draw anything; they set/get
slots of an original graphics context, presumably for later renderer
execution. Conflating their presence with a completed GPU bind/draw
would be incorrect.

| Original VA | Original source operation | Native function |
| --- | --- | --- |
| `0x3B76D0` | write A1 into context **+0x828**, copy U32 at `*(A1+0xC)` into context **+0x818** | `originalSetRenderSource828` |
| `0x3B76E8` | write A1 into **+0x82C** | `originalSetRenderSource82c` |
| `0x3B76F8` | write A1 into **+0x830** | `originalSetRenderSource830` |
| `0x3B7708` | write A1 into **+0x834** | `originalSetRenderSource834` |
| `0x3B7718/20/28/30/38` | return stored source pointer at **+0x824/28/2C/30/34** | `originalGetRenderSource82[4-8]`, `originalGetRenderSource82c/830/834` |

`original_render_context.hpp/.cpp` maintains the fields as
`uintptr_t` host handles and U32 `source818`, rather than emulating
32-bit guest memory. Its native event executor consumes the existing
source-verified `OriginalRetailPassTrace` and applies these real writes
to the correct selected resource object in original call order.
An explicit original-call callback is **mandatory** for still-unported
`0x2803B4`, `0x26831C` or `0x2804F4` operations. This prevents
mistaking mere state writes for a fully rendered game scene.

## Original actor render-resource lifecycle

Original `0x26A854`: if actor+0x110 is null, allocate 4-byte outer
handle, call `0x26A510` (allocate 0x60-byte inner resource), store
the handle at actor+0x110. If handle exists, return it unchanged.
Original `0x26A510` explicitly clears **inner bytes +0x00, +0x08,
+0x10** and U32 entries **+0x40,+0x44,+0x48,+0x4C,+0x50**.
It does **not** explicitly clear scalar +0x04, +0x0C or selected index
+0x14; native `OriginalRenderResourceState` marks these unspecified
fields as optional/uninitialized rather than inventing game values.

- `0x26AFAC` optional actor +0x110 returns resource byte +0x00
  (otherwise zero); `0x26B3D0` returns byte +0x08, zero if missing;
  `0x26B550` likewise returns byte +0x10.
- `0x26A8DC` stores byte +0x00; `0x26A968` stores byte +0x08;
  `0x26A8AC` sets +0x00 to 1 and writes scalar +0x04;
  `0x26A938` sets +0x08 to 1 and writes scalar +0x0C.
  These scalar writes are recovered; any intervening original SIMD/VU
  effects are **not** claimed ported.
- `0x26AEB4` enables resource selector byte +0x10 and writes index
  +0x14. `0x26AEE0` clears +0x10 but retains +0x14. This module uses
  the graphics-owned `OriginalRenderSelector` rather than duplicating
  selected-state semantics.

Source code: `original_render_resource.hpp/.cpp`. It accepts
native actor-owned state, not a fabricated executable-memory layout.

## Source-backed actor renderer entry at 0x26831C

In source order:

1. Call `0x23C9CC(owner+0xFC)` and `0x23CAA4(owner+0xFC, 0)`.
2. When result of `0x23CAA4` is nonnegative, call `0x23CAE8(owner+0xFC,0)`.
3. If actor U32 +0xF8 is **exactly one**, update float
   `actor+0xF0 += globalFloat[0x635488] * actorFloat[0xF4]`,
   with a separate single-precision multiply and addition.
4. If actor byte+0x30 is zero OR byte at global VA `0x6496F0` is
   nonzero, return zero. Otherwise proceed into still-unported
   downstream drawing at `0x26841C`.

New `original_render_actor_entry.hpp/.cpp` implements these native
steps with mandatory callbacks for the three original `0x23Cxxx`
methods and runtime-owned values for the global delta/flag. The
original game's `0x635488` float is **zero in the retail ELF file**,
so treating that file-time zero as permanent game delta would be a bug.

## Original effect wrapper 0x2804F4

Original source calls:
`0x26A854(actor)` -> `0x26A4AC(handle, descriptorOut)` ->
`0x269F7C(inner,descriptorOut)` ->
`0x3AE900(globalGfx,descriptorOut)` ->
`0x3B76F8(context,returnedPacket)`.

`original_render_effect.hpp/.cpp` reproduces the resource lifecycle
and packet-slot mutation in this order. Converters `0x269F7C` and
`0x3AE900` have **not** been natively decompiled: callbacks are
required and absence throws. The host supplies only opaque handles,
not a fabricated packet format/effect. This is original effect
**preparation**, not yet a D3D11 shader or particle renderer.

## Private executable verification fingerprints (ranges end-exclusive)

Additional 10 byte-region SHA-256 checks are in
`native/tools/audit_original_graphics.py`, verified against the
owner-private original executable:

| Range | SHA-256 |
| --- | --- |
| 3B76D0–3B7740 | `ac883e9b102c531ce8061c48a8161f27e2bf8a8909e408a55b3771ff4aa34a93` |
| 26A4C4–26A55C | `7a15cd6dc48d8dac29d2e55b34ca37d9fc522de7c5f60726c315aa3958856e33` |
| 26A854–26A8A4 | `6da75758cde672462a2460741043bba9ebd7fa63c353fc7ff8dd14018eeea262` |
| 26A8AC–26A8F4 | `f6abab67768fdc1431cd7fc1c0e6ace3f72d08d0fc083c406bf10d63ee840717` |
| 26A938–26A980 | `6a5231aba8fe0365f8456d8e194fe574ad9bb7e75370bcb8d711d70b55972011` |
| 26831C–268424 | `4ed314ac49d5c0a81d11cfa20d14de81e50a5d4738a13ab9152ce2c4b1bf5c69` |
| 2802D4–280328 | `e55bff41b6a4c185107783132caf36163e8290aa0bd70ce146fbe242bea4a7b7` |
| 280328–280364 | `908308ffee03d34f74f18d64639dd39b25fdd97d2879a47c1ea9db0781d3d121` |
| 2803B4–2804F4 | `45115d2c0466c8299d83b2a716785192c90cc7a7111c4e5c64ed442182f372a2` |
| 2804F4–280544 | `056a50d5e7eacea38c602f34fa2933391b4a3dd185ef0e838cf9f41815bc3a37` |

## Actual local tests (not a Windows game boot)

Focused C++20 sources under `/mnt/data/graphics_work/native`;
GCC release `-O2 -Wall -Wextra -Wpedantic -Werror`, and Clang
`-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Werror`.
Ten suites, **20/20 compiler-and-test executions passed** (one original
pipeline sanitizer run retried separately after a global command timeout).
New test files:
`original_render_context_test.cpp`,
`original_render_resource_test.cpp`,
`original_render_actor_entry_test.cpp`,
`original_render_effect_test.cpp`;
existing original visibility, render pass and pipeline suites now check
the *correct* source culling polarity. No real stage model rendering,
Windows native game bootstrap or full CMake/CTest was run.

## Next native recovery blockers

Actual `0x26841C–0x2689xx` model render body and its VU operations,
`0x2803B4` graphics transforms, original
`0x269F7C` resource descriptor conversion, `0x3AE900` packet
generation and subsequent hardware-independent shader-state translation
are not yet implemented. The VU0 team owns `0x26C4B0` spatial
calculations. The Assets team must supply authentic original mesh and
material data; the integration team must wire stage actor owners into
the real Windows native loop. No default scene/rendering may be
substituted.
