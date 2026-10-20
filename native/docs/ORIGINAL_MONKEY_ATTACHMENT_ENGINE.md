# Original Ape Escape 3: Monkey attachment engine — source-level recovery

Source: actual USA retail SCUS_975.01, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This pass follows the **actual original Monkey attachment engine**, not a
replacement game subsystem or PS2 hardware emulator. The original executable
stays owner-private.

## Recovered code and object layouts

The new host C++ module is
`native/src/original_attachment_engine.cpp`, with an exact-address
verification script at `native/tools/verify_original_attachment_engine.py`.

| Original method | What the original code does |
|---|---|
| `0x312760` | Allocate **96 bytes**. Copy exactly **four 16-byte quadwords** from parameter 3 into object offsets 0x10–0x4F; store owner at +4, parameter 2 at +0x50, zero +0x54, class vtable 0x6B1220 |
| `0x3127D8` | Allocate **72 bytes**. Set vtable 0x6B1280, owner +4, copy eight bytes from original global 0x707200 to +8, set parameter +0x10; clear known fields +0x14, +0x3C, +0x3D, +0x40, +0x44 |
| `0x312854` | Allocate **64 bytes**. Set vtable 0x6B1260, owner +4, parameter +8, clear fields +0x30 and +0x34 |
| `0x571E44` | Initialize original controller class vtable 0x6B1500; set owner +4, copy eight bytes at 0x707F10 to +8; set parameter +0x18; clear +0x10, +0x14 and +0x20 |
| `0x3161B8` | Apply original optional bit flags 1 and 2; dispatch child virtual slot 5 through original nested-call path; store exact returned word +4; on success call child deleting virtual slot 1 and clear the child pointer |
| `0x315C08, 0x315C40` | Identical cleanup wrappers: if child exists, follow original child+8 pointer, set its +0xA1 byte flag to 1 and free original child |

Four original controller virtual methods at
`0x5725D0` / `0x572800` (return integer 3) and
`0x5725D8` / `0x572808` (return the original third argument)
are implemented. The controller methods
`0x5725E0` / `0x572810` forward to their existing attached
child's virtual slot 3, and `0x572524` conditionally deletes
that child.

Six more entries in original attachment vtables 0x6B1240/0x6B1220
are proven simple zero-return methods or no-op handlers. The exact
original `0x56AF24` **returns zero**: the 8-byte attachment
created by `0x3126E8` therefore does NOT trigger success/deletion
in the `0x3161B8` lifecycle decision. The other attachment type's
virtual slot 5 (`0x56AF44`) is nontrivial and still requires decompilation.

The original base no-op `0x315BAC` is identified separately.

## Audit and native test

The owner-private ELF was read directly and verified with SHA-256.
The audit checks **four original virtual tables**, **58 exact machine-word
sites** and **six original direct-call targets**. One table boundary
at 0x6B1520 was identified only after a failed verification, then
corrected by reading actual table pointers; it is not a guessed layout.

The native C++ implementation passed a local optimized C++20 build test
under `g++ -O2 -Wall -Wextra -Wpedantic -Werror`. Cases test
constructors, copied bytes, native pointer associations, zero-return and
pass-through virtual methods, bit flags, nested invocation, success
deletion and failure retention, and cleanup flag/allocator behavior.

**Limitations:** The native types use host pointers and keep unknown fields
uninterpreted. The original allocator, virtual child implementations,
monkey AI/capture logic, physics, stage boot, rendering and menu code are
not translated by this module. The callback interfaces explicitly expose
those unresolved engine dependencies; they are not substitutes for a
playable game. No Windows CI run was triggered.
