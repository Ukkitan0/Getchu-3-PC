# Ape Escape 3 USA — native actor navigation route consumers

Owner-private source evidence: genuine USA `SCUS_975.01` SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
and genuine `woods_c/navigation.pck` SHA-256
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.
No original copyrighted input files are committed to the repository.

## Route-to-actor control flow recovered

Five direct retail calls to `0x2207D4` occur at:

- `0x570D80` (owner route field +0x1C, then `0x31759C` actor controller construction)
- `0x574E60` (owner route field +0x18, then `0x317538` actor controller construction)
- `0x575220` (owner route field +0x10, then `0x31759C` actor controller construction)
- `0x5756F8` (owner route field +0x28, then `0x31759C` actor controller construction)
- `0x575918` (route field +0x88 using `0x575EC0`, then `0x317538` actor controller construction)

Original `0x317538` / `0x31759C` each allocate a 0x70-byte controller and
call `0x57BA98` / `0x57BAF8`, respectively. These constructors store the
source actor at +4, the original route at +0x18, flags at +0x1C/+0x1D,
and a movement variant flag at +0x30. The latter constructors call the
larger, **not yet reconstructed** `0x57BCB8` controller initializer.

The original `0x57BCB8` contains a distinct actor-side route consumer:
`0x57BD90` calls `0x221104` with original route pointer and route step
index 0, then `0x57BD9C` calls `0x21B444` to check the selected source
polygon's edge action kind. Kind 2 branches into the original type-2
source action handling. Full actor movement, steering and animation
updates remain incomplete.

## Native source-backed operations

`native/include/ae3/original_actor_nav_route.hpp` and
`native/src/original_actor_nav_route.cpp` implement:

- `originalActorRouteEdgeSlot_221104`: the exact original bounded three-slot
  neighbor scan of adjacent entries in the actual route. The original
  returns 0 for **both** a match in slot zero and no match; it does not
  return -1 on failure. The native port retains that behavior.
- `originalActorRouteEdgeKind_21b444`: reads the 16-bit edge action kind
  from the original 0x1E-byte polygon descriptor's offset +0x0A.
- `originalActorRouteEdgeDecision_57bd88`: links the two original operations
  using genuine route source IDs; identifies original action kind 2.
- `originalActorReplaceRoute_575ec0`: preserves the original identity
  test and teardown order: `0x220A78` releases the route's internal
  allocation; `0x41A3B0` releases the route outer object; the new
  route replaces the old pointer. Callbacks for the original allocator
  functions are explicit; no replacement or fabricated allocator exists.

The source stores original 32-bit guest pointers, whereas the native
implementation receives source polygon indexes and host-owned route
identities. This is **semantic native C++ recovery**, not binary ABI
compatibility, and the engine's actor pointers/lifetime manager are
not yet wired up to it.

The source uses unchecked array/pointer accesses. The native port
explicitly rejects out-of-bounds path indexes, malformed polygon
headers and invalid three-slot accesses for host memory safety.

## Retail verification

`native/tools/verify_original_actor_nav_route.py` SHA-pins **eight whole
machine-code regions comprising 197 original MIPS words**, checks
26 independent opcode sites, 15 call edges (including every original
`0x2207D4` caller and `0x57BD90 -> 0x221104`), and four branch targets.
It also checks all 59 woods_c source polygons, 137 authentic polygon
links and their 15 original kind-2 edges.

The `original_actor_nav_route_test.cpp` CTest verifies all three edge
slots, source action kinds, original no-match behavior, original route
teardown order, identity no-ops and invalid host bounds. With private
`AE3_RETAIL_WOODS_C_NAVIGATION_PCK`, it tests each genuine retail edge
against an independently scanned source descriptor. Local GCC/Clang
Release, ASan+UBSan and UBSan tests passed against the original PCK.

This does **not** establish playable movement or a working Windows game.
No emulation, fabricated gameplay or GitHub Actions workflow was used.