# Ape Escape 3 USA — Original navrange callback installation and teardown

Retail game evidence: owner-private `SCUS_975.01` executable,
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No retail executable, asset data, or source-byte copies are committed.

## Why the 0x4E4060 handler has no direct callers

The original game's navigation registry uses a table of callback function
pointers at **0x6AE248**. Its six 32-bit entries are:

| Offset | Original entry |
|---|---|
| +0x00 | 0x4E4060 — `navrange` resource group registration |
| +0x04 | 0x4E4050 — callback object initializer |
| +0x08 | 0x4E4030 — callback cleanup and release |
| +0x0C, +0x10, +0x14 | null |

A complete static scan of original PT_LOAD instruction words found zero
direct JAL edges to 0x4E4060: this is expected for an indirect callback.
The source registration function at **0x21F8D0** sets the original
navigation map pointer in the 32-bit global **0x647B90**; at 0x21F920
it passes the 0x6AE248 descriptor through the outgoing argument area
to original generic registration function **0x27242C**.

The stage bootstrap calls `0x21F8D0` at **0x22D8D8**, and the
stage shutdown calls `0x21F940` at **0x22E064**.

During shutdown, original 0x21F940:
1. Uses callback descriptor **0x6AE260** with 0x36D844.
2. Checks the original 0x647B90 global and, when non-null,
   invokes 0x36D5A8 to destroy the map and 0x41A3B0 to free it.
3. Clears 0x647B90.
4. Invokes 0x4E3D9C with final descriptor **0x6AE278**. This function **only resets the descriptor object's vtable to 0x6AE290**; it does not free that descriptor. The map is destroyed separately.

The shutdown descriptors also contain original function addresses.
They are identifiers/data, not valid pointers to call in the native host.

## Native implementation

`native/include/ae3/original_navrange_registration.hpp`
adds `OriginalNavrangeRegisteredOwner`. This native owner retains
the already implemented keyed navigation registry, invokes the
existing source-derived original `F20` polygon eligibility when a
resource is delivered, and clears the group owners during teardown.
It keeps the source ordering: owner created before callback installation,
dispatcher teardown request before map deletion, and descriptor vtable reset
after the owner global is logically cleared.

The two as-yet-unrecovered generic registration operations and descriptor
vtable reset are **required event hooks**, supplied by a future
original stage/asset manager implementation. The class does not
pretend that the game resource loader is implemented or invent a
replacement. The source method descriptors are constants and
**are never executed as PlayStation guest code**.

The old `OriginalNavGroupRegistry` and optional eligibility callback
API remain unchanged; this adapter is opt-in rather than a silent
substitution during game startup.

## Verified evidence

`native/tools/verify_original_navrange_registration_lifecycle.py`
validates the SHA-pinned private original ELF, all **three exact
function regions totaling 75 instructions**, the three original
callback descriptors, **26 exact opcode anchors**, **11 direct call
edges**, original map global and descriptor construction, and the
absence of a direct JAL to the registered resource handler.

`Getchu3OriginalNavrangeRegistrationTests` exercises attempted
shutdown before init, registration errors, double initialization,
original-source registration with two accepted / one excluded polygon,
repeat resource dispatch, teardown cleanup, blocked dispatch after
shutdown, and clean reinitialization. The test also checks the final
cleanup descriptor and callback order.

Local **optimized C++20, UBSan and ASan+UBSan tests passed** against
the actual new header and a minimal mock of the unavailable existing
engine boundary, not the full game-library build. Original-ELF source
verification passed. The complete Windows build and live callback
invocation from a real retail stage are **not yet verified**.

## Remaining limitations

This pass reveals original function-pointer registration and bridges
the recovered native polygon eligibility into a stage-lifecycle owner,
but it does not reconstruct the generic dispatcher at 0x27242C,
actual resource allocation from the ISO, bit-exact VU0 arithmetic,
live stage startup, or playable rendering/input. The archive in this
workspace contains the retail executable and extracted Lua scripts,
but not the full 3.1 GB ISO resource inventory. Do not claim that
the stage resource callback fires in a real scene yet.

No emulation, artificial game-level logic, or manually dispatched
GitHub Actions were added.
