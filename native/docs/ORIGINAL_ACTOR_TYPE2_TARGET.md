# Ape Escape 3 USA — original actor's type-2 route target

Original user-owned USA retail ELF `SCUS_975.01` SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original private `woods_c/navigation.pck` SHA-256:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

## Source control flow

At `0x57BD88`, the original actor controller calls `0x221104`
to identify the route's first edge; `0x57BD9C` calls `0x21B444` to
read its source action kind. The branch at `0x57BDA4..0x57BDB0`
checks **kind == 2**.

On that branch, `0x57BDB8..0x57BDC8` dereferences the original
route's polygon-pointer array, reads the second source polygon,
and stores its identity at **controller +0x50**.

Three subsequent calls at `0x57BDCC`, `0x57BDDC`, and `0x57BDF0`
invoke original `0x21B458` for vector indices **0, 1, and 2**.
Source `0x21B458` performs original 16-byte EE LQ from
`record + 0x40 + index * 16`. These are original game-data vertices,
not a replacement path planner or synthesized collision points.

## Native integration

`native/include/ae3/original_actor_type2_target.hpp` and
`native/src/original_actor_type2_target.cpp` implement the source
second-record identity and the three **previously recovered**
`originalSpatialVector_21b458` reads. They take the actual decoded
`OriginalSpatialRecord70` collection and route record indexes,
returning a typed source-record pointer and the original vertex bytes.

The original executable uses 32-bit guest pointers. This host-native
implementation uses source IDs resolved to genuine host-owned records.
It does **not** manufacture original game runtime pointers.

The next original MMI/COP2 transformations and interpolation after
`0x57BDD4` remain untranslated; this pass must **not** be described
as a complete 0x57BCB8 implementation or a working actor movement loop.

## Verification

`native/tools/verify_original_actor_type2_target.py` SHA-checks the
owned ELF and original PCK, pins 33 original instruction words in two
complete machine-code regions, checks 21 opcode sites and five
original JAL edges, and surveys all 59 genuine `woods_c` spatial
records with 137 original edges and **15 type-2 action edges**.

`native/tests/original_actor_type2_target_test.cpp` checks that the
second route polygon — not the first — is selected, all three 16-byte
original vectors are preserved, invalid host route indexes are rejected,
and the type-2 operation matches all 15 real source edges from the
original PCK when `AE3_RETAIL_WOODS_C_NAVIGATION_PCK` is supplied.

Tests use the existing native source-loader/spatial record APIs.
The original executable, navigation data and copyrighted assets
are owner-private and are not added to GitHub.

This is one source-backed component toward the native engine.
The complete game is not playable on Windows yet. No GitHub Actions
were triggered and no emulation was added.
