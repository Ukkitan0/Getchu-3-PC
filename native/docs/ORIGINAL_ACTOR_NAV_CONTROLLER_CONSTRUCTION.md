# Ape Escape 3 USA — original actor NavGraph controller construction and dispatch

Source: the owner's original USA `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No original retail executable, copyrighted models, code bytes or game resources
are published here. The C++ represents source-observed semantics rather than
an emulated EE memory/vtable ABI or a replacement movement/gameplay engine.

## Actor movement-controller factory chain

Both original factories `0x317538` and `0x31759C` call original allocator
`0x41B800` with **112 bytes (0x70)**. They pass original actor, route and
mode flags to these constructors:

| Constructor | Original engine fields written before 0x57BCB8 |
| --- | --- |
| `0x57BA98` | `+0x04` actor pointer, `+0x08/+0x0C` zero, `+0x10` child null, `+0x14` zero, `+0x18` original route pointer, `+0x1C` `(int32_t)route[+4] >= 2`, `+0x1D` low byte of `$a3`, `+0x30` zero, source vtable at `+0x00` |
| `0x57BAF8` | Same common fields, `+0x20..+0x2F` full 16-byte source `$a3` EE `SQ`, `+0x1D` low byte of `$t0`, `+0x30` one, source vtable at `+0x00` |

The first constructor does **not** initialize +0x20..+0x2F; it must not
be replaced with a fake default zero vector. Both constructors call
`0x57BCB8` before returning. The C++ functions named `Prepare` represent
**only the verified pre-call writes** and expose
`requiresOriginalPostConstructor_57bcb8=true` to prevent claims that a
complete actor/controller has been reconstructed.

## Original controller child dispatch

The 0x70-byte source object stores an optional polymorphic child at +0x10:

- `0x57BC0C`: if original actor field +0x40 is nonzero, return 3;
  otherwise dispatch child vtable +0x08, or return zero with no child.
- `0x57BC54`: dispatch child vtable +0x10 when present, passing through
  original arguments; if absent return the original incoming `$a2`.
- `0x57BC88`: notify child using vtable +0x0C if present.
- `0x57BB60`/`0x57BBB4`: call child vtable +0x04 destructor, then rewrite
  original vtable pointer to the base destructor table. The latter is
  the *deleting destructor* and tailcalls source allocator `0x41A3B0`.

The current C++ uses explicit required virtual-call interfaces, so
there is **no** invented source actor or child method implementation.
The destructor helpers preserve the callback sequence; they do **not**
write original PS2 vtable words, clear the original +0x10 field, or
model the original C++ object allocator. They do not turn an incomplete
controller into a playable engine actor.

## Source verification

`native/tools/verify_original_actor_nav_controller.py` audits four
SHA-pinned original instruction windows, totaling **223 original MIPS
instruction words**, including both factory functions, both controller
constructors, three child dispatchers, both destructor variants, and
part of the unresolved `0x57BCB8` entry. It pins 39 independent opcodes,
6 direct `JAL` calls, one direct `J` tailcall, and two branch targets.

`original_actor_nav_controller_test.cpp` checks signed route-length
boundary values, the byte-masked flags, 16-byte variant-one source copy,
zero/unchanged pre-initialization semantics, actor state priority,
child vtable call/argument forwarding and return values, missing-child
behavior, and child-before-allocator teardown ordering. Locally tested
under GCC and Clang strict optimized C++20, standalone UndefinedBehaviorSanitizer,
and AddressSanitizer + UndefinedBehaviorSanitizer. This is an isolated
host-native test, **not a full Windows game build**.

## Next critical original engine translation

The `0x57BCB8` movement-controller method contains actual VU0/COP1
source math and switch paths from `0x57BCE8` through `0x57C0..`.
Its source branch at `0x57BD90` consumes `0x221104` route edges and
`0x21B444` action kinds (previous recovery). The controller's actual
vector state update, route stepping, object/animation handoff and stage
lifecycle must be translated and connected before actor movement works.
The full Windows game has not been booted by this work.

No GitHub Actions were triggered.