# Original Ape Escape 3 USA — model draw packet recovery (graphics pass 4)

## Source and evidence

Authoritative executable: owner's private **USA SCUS_975.01**, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The original ELF is never committed. VAs below are retail MIPS instruction addresses,
verified with a private local `llvm-objdump` pass and SHA-256 byte-range comparison.

This pass recovers **native model-to-render-descriptor flow and the five-group packet
assembly dispatch**, plus **two genuine binary copy operations**. It is NOT a PS2
emulator or a substitute rendering engine. The original 0x27C614 / 0x27C520
model converters, 0x51C9B0 scratch initializer, VU-only actor block, allocation
and GPU segment transform bodies are still dependencies, never fabricated.

## Original draw-packet chain

`native/include/ae3/original_render_draw_packet.hpp`,
`native/src/original_render_draw_packet.cpp`:

| Original VA | Source-derived operation | Native implementation |
| --- | --- | --- |
| `0x280328–0x280360` | Shift scratch descriptor by **+0x330**, call `0x3AF8F0(globalGraphics, descriptor)`, tail-call `0x3B76E8(context, packet)` | `originalBuildDrawPacket280328` invokes original packet builder, sets native context `source82c` |
| `0x280364–0x2803B0` | Create 0x390-byte scratch, call `0x51C9B0`, `0x2802F8` / `0x27C614` with model root `+0x28`, then call `0x280328` | `originalBuildStaticDrawPacket280364` preserves exact call sequence |
| `0x2803B4–0x2804F0` | Initialize scratch, save actor `+0x118` word at scratch `+0` on delay slot, run `0x280310` / `0x27C520`; check `0x26AFAC` original actor resource byte `+0`; optional `0x26A854` / `0x26A8F4` and VU-assisted scratch population, then `0x280328` | `originalBuildActorDrawPacket2803b4` preserves source **outer** call routing and packet output |

`OriginalDrawScratch` is a host-native 0x390-byte descriptor owned on the C++
stack. It is **not** a simulated guest address space. It exposes source
`+0x330` as a 0x60-byte `std::span` to the packet assembler. Original
model conversion and PS2-specific scratch stores are passed to **mandatory**
source-bound callbacks, making missing original functionality explicit.
`OriginalRenderContextSlots::source82c` is updated through the
previously recovered `originalSetRenderSource82c` on successful packet assembly.

**Important accuracy boundary:** Do not interpret the actor wrapper as a
native implementation of `0x27C520`, `0x26A8F4`, the masked SIMD/VU
instructions at `0x28040C–0x2804C8`, or the final Direct3D draw.
The caller must recover those genuine instructions and supply their behavior.

## Original packet material-group assembler `0x3AF8F0–0x3AF9C0`

The original packet constructor always:

1. Calls `0x3B3CE0(globalGraphics, 4)` to allocate an original packet owner.
2. Calls `0x3AF8B8(packet)` to reset packet fields.
3. Installs graphics owner at packet `+0x38`, then calls `0x3AF8A8(packet, descriptor)`.
4. Reads **five** descriptor `(u32 token, u32 count)` pairs in order and
   invokes a different original handler when the corresponding count is nonzero.
5. Returns the original packet handle.

| Material group | Token offset | Count offset | Original handler | Inner processing |
| --- | --- | --- | --- | --- |
| 0 | `+0x10` | `+0x14` | `0x3AF478` | Allocates and transforms 32-byte records via VU/packed copy |
| 1 | `+0x18` | `+0x1C` | `0x3AF680` | **Native recovered:** copy `count * 0x20` bytes |
| 2 | `+0x20` | `+0x24` | `0x3AF6F8` | Per-record VU/copy and original floating-point transform |
| 3 | `+0x28` | `+0x2C` | `0x3AF530` | Allocates/transforms 48-byte records |
| 4 | `+0x30` | `+0x34` | `0x3AF600` | **Native recovered:** copy `count * 0x30` bytes |

Source loop is unrolled. Zero counts are skipped independently without
reordering the other groups. Original integer words are read explicitly as
little-endian; they are not cast to host pointers, eliminating guessed mapping
of original 32-bit data tokens to host assets.

`native/include/ae3/original_render_packet_assembly.hpp` and
`native/src/original_render_packet_assembly.cpp` implement this original
constructor dispatch and return the source packet handle. The callbacks
for original packet allocation/setup and the five group handlers are
mandatory until source-equivalent native versions are integrated.
No fake graphics packets or shaders are created.

## Two original packet-copy functions recovered to native

In `native/include/ae3/original_render_packet_copies.hpp`,
`native/src/original_render_packet_copies.cpp`:

- `0x3AF680–0x3AF6F4`: store 32-bit record count at packet `+0x18`,
  allocate **2*count 16-byte units** via `0x3B3CE0`, copy exactly
  **32*count bytes** via retail `0x42B16C`, store pointer at packet `+0x1C`.
  Native equivalent produces an owned `OriginalCopiedPacketSegment`
  preserving original data verbatim and metadata (group 1, count, allocation units).
- `0x3AF600–0x3AF67C`: store count at packet `+0x30`, allocate **3*count
  16-byte units**, copy exactly **48*count bytes** via `0x42B16C`, store
  pointer at packet `+0x34`. Native equivalent preserves original source
  bytes verbatim as group 4.

Each copy requires actual original payload bytes supplied by Assets. It
rejects insufficient source data, oversized count/byte multiplications and
all three other segment types, which require original transformations
and **must not be treated as memcpy**. C++ vectors own their bytes and
are independent of the user-supplied source memory.

Original `0x3AF8B8`, `0x3AF8A8`, packet allocator `0x3B3CE0`,
transformed packet groups `0x3AF478/0x3AF6F8/0x3AF530` and
full scene GPU submission are **NOT** marked recovered. This is a valid
incremental original engine port, not a playable-game claim.

## Private original-instruction evidence

The retail audit tool `native/tools/audit_original_graphics.py` checks
these exact byte spans, all exclusive-end VAs, against the private owner ELF:

| VA interval | SHA-256 |
| --- | --- |
| `0x280364–0x2803B4` | `e59a2f65148e03ce001d7efd4418e067b8554e7e06237db10450a1f327fefbb9` |
| `0x3AF8F0–0x3AF9C4` | `203f02462c8b8e43621242b6d67147c0beeaa91aedb52feb990f4a92b08fb7ea` |
| `0x3AF8B8–0x3AF8F0` | `6fc509515ce05af8fc878d9cc797a9f5028a4abbbf1445700a3fb086d6927b0c` |
| `0x3AF478–0x3AF530` | `7dfd9036d8b7b34767af38b93d11aa646250b1459e981b5326983920ff9ae40e` |
| `0x3AF600–0x3AF680` | `d81f350f97a6c04771884fcb544166391ceaac7604825fa11564234e4edf17a4` |
| `0x3AF680–0x3AF6F8` | `4953e4927eeaa9b066fbb1a72f30706bcfb3d644acd9f230b2da1ab96a2e416b` |
| `0x42B16C–0x42B224` | `a7f576b7fc50f78255778c0773db38be4ae78a38dbbf0758d7eb52dfbb35d2ad` |

The earlier pass already checks original `0x280328–0x280364` and
`0x2803B4–0x2804F4`, avoiding duplicate entries.

## Real tests and integration

Three new focused native C++20 tests:
`original_render_draw_packet_test.cpp` (static/actor path and failure
guards), `original_render_packet_assembly_test.cpp` (all five group
routing, conditional gating and native context `+0x82C` integration),
and `original_render_packet_copies_test.cpp` (32/48-byte copies, budget
and truncation safeguards).

Local GCC 14.2.0 `-O2 -Werror` and Clang 17.0.0
`-fsanitize=address,undefined -Werror`: **six of six test executions
passed** with synthetic structural bytes. Native Windows Direct3D and
retail stage mesh validation remain unrun; the original-game payload
needed for that is not supplied here.

### Precise Windows integrator request

**Only the Windows-runtime owner** should update `native/CMakeLists.txt`
and `app_win32.cpp`. Add six new source/test files:

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_render_draw_packet.cpp
  src/original_render_packet_assembly.cpp
  src/original_render_packet_copies.cpp)
foreach(name IN ITEMS original_render_draw_packet
                      original_render_packet_assembly
                      original_render_packet_copies)
  add_executable(${name}_test tests/${name}_test.cpp)
  target_link_libraries(${name}_test PRIVATE Getchu3Assets)
  add_test(NAME ${name}_test COMMAND ${name}_test)
endforeach()
```

No new dependencies beyond the graphics modules already handed off.
The integrator must supply real owner-model lookups, the original
`0x51C9B0` descriptor and `0x27C614/0x27C520` converter APIs,
recovered `0x3B3CE0` / `0x3AF8A8` packet ownership, native source
data from Assets and the VU-owned geometry transforms before connecting
to a final Direct3D draw. Reference data token `u32` must be resolved
to host buffers by original Assets lookups, never blindly dereferenced.

The next connected original rendering blocker is material-transform
groups 0/2/3 and original actor VU scratch construction, which feed
actual mesh/texture/animation data into these verified material descriptors.


## Follow-up within pass 4: authentic packet arena cursor math

After tracing packet allocation from the original `0x3AF8F0` group constructor, the retail helper `0x3B3CE0` was fully identified as a 16-byte-unit **backward** graphics arena cursor adjustment, and `0x3B3D00` as its **forward** counterpart:

```cpp
// Native host offsets, not EE addresses. Source op sequence only:
newBack = (back - units*16) & ~uint32_t(15); // original +0x18, 0x3B3CE0
oldFront = front;                            // original +0x14, 0x3B3D00
newFront = (front + units*16 + 15) & ~uint32_t(15);
```

`native/include/ae3/original_render_packet_arena.hpp` and
`native/src/original_render_packet_arena.cpp` reproduce these original integer
operations using typed offsets within host-owned graphics memory. They add
bounds, overflow and cursor-collision guards **not present in the retail
code** to avoid host memory corruption. The backward routine returns
the **new** cursor; the forward routine returns the **old** cursor. These
different return semantics were checked against retail instructions.

The two new exact original executable source SHA-256 fingerprints are:

- `0x3B3CE0–0x3B3CFC`: `13c8b65e6a2bcac7c1ffb3a980f280d4f2cbe70dcc81ca238fa0d27f993e8f92`
- `0x3B3D00–0x3B3D20`: `4c2ea7ef8022fcd90bf053c968baa32e8650847c95829d708742e35e7639e9bd`

Test: `native/tests/original_render_packet_arena_test.cpp`,
GCC 14.2 `-O2 -Werror` **PASS**, Clang 17 AddressSanitizer +
UndefinedBehaviorSanitizer `-O1 -Werror` **PASS**. Tests include
bidirectional allocation, 16-byte alignment, original return values,
overflow, malformed capacity, and double-sided collision protection.

Integrator-only CMake additions:

```cmake
target_sources(Getchu3Assets PRIVATE src/original_render_packet_arena.cpp)
add_executable(Getchu3OriginalPacketArenaTests tests/original_render_packet_arena_test.cpp)
target_link_libraries(Getchu3OriginalPacketArenaTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderPacketArena COMMAND Getchu3OriginalPacketArenaTests)
```

Native packet arena offsets can back the real `0x3B3CE0` allocation callback
once Windows Runtime provides its graphics arena buffer and byte lifetimes.
The packet allocator cursor is **not by itself** a GPU memory allocator or a
complete model renderer. Full original resource ownership and packet
serialization remain to be recovered and integrated.

**Updated pass-4 focused tests:** four new C++ test suites, **8/8**
GCC/Clang sanitizer test executions passed, separate from prior passes.
No game-native GPU draw or playable build claimed.
