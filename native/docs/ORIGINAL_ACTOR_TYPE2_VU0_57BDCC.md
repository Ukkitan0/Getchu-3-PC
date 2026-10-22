# USA original actor type-2 target VU0 interpolation: 0x57BDCC–0x57BED4

## Genuine source and connected behavior

The source is the owner's private US `SCUS_975.01` with SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The entire original `0x57BDCC..0x57BED4` body, inclusive (**67 instructions**), is SHA-256
`7974c5e0c777b060f0e55c0c99c8fca7bb0d1df91b84772486d8c49dbe76136b`.

This code is on the original **action-kind-2** actor movement path. The existing
navigation-team `originalActorType2Target_57bdb8` recovers the source
route's second polygon and the three genuine source `0x21B458` vertices.
This physics recovery begins **after** that original polygon selection.
No source navigation/route-search implementation was edited.

| Original addresses | Recovered original operation |
|---|---|
| `0x57BDCC/BDDC/BDF0` | Three original calls to `0x21B458` reading 16-byte source polygon vertices 0, 1, 2 |
| `0x57BDD4/BDEC/BDFC` | Three `PCPYUD saved,zero,v0` instructions; X/Y lanes zero; source Z/W preserved |
| `0x57BE00/BE24` | Authentic EE RNG calls `0x36F4D4`, `0x36F548`; exactly two draws |
| `0x57BE18..0x57BE84` | Source float weight calculation: coefficients `(a*0.8 + 1/15)`, `(b*0.8 + 1/15)`, `((1-a-b)*0.8 + 1/15)` with original FPU calculation order |
| `0x57BE8C/BEAC/BEC0` | Three original `VMULx.xyzw` operations using source vertex 0, 1, 2 |
| `0x57BEC8/BECC` | Two original `VADD.xyzw`: first product0+product1, then +product2 |
| `0x57BED0/0x57BED4` | Branch to parent epilogue with delay-slot **`SQC2 VF3,0x40(s4)`**, writing all 16 bytes to source controller+0x40 |

The source constants at `0x708A24/28/2C` have raw float32 words
`0x3F800000` (1.0), `0x3F4CCCCD` (0.8), and `0x3D888888` (~0.066666663).
Those are **bitwise identical** to `0x708A64/68/6C`, which belong to
the already recovered `0x57C98C` random-triangle target. The source
calls and native math use the **same operation order**, so the new
original-specific controller writer calls the existing
`originalActorRandomTriangle_57c98c` exactly once. It reuses
an authenticated original routine, not an invented steering algorithm
or a generic replacement physics engine.

## Native interface and integration boundary

`native/include/ae3/original_actor_type2_vu0_target.hpp`:

```cpp
OriginalVU0Vec4 originalActorType2VU0WriteController40_57bdcc(
    const std::array<std::array<std::byte,16>,3>& originalSourcePolygonVertices,
    OriginalGameRandom_36f3c0& originalSharedGameRandom,
    std::array<std::byte,16>& originalControllerVector40) noexcept;
```

Call this **only on the source action-kind-2 path** with
`OriginalActorType2Target_57bdb8::rawOriginalVertices` from the navigation
workstream's native lookup and the actual global/original shared RNG.
The third argument corresponds to the caller-owned **actual controller+0x40**
storage, not the stage polygon record or a synthetic movement vector.

This function writes the full 16-byte VU0 result, as original `SQC2` does.
Do **not** use the old 64-bit-only triangle sampler return value for
this path, do not draw a second set of random numbers, and do not
substitute a new random distribution. Q and R are **not used** by this
original specific block (they remain necessary for `0x57CA68` elsewhere).
The navigation team owns the source action-kind predicate and record selection.

**Integrators only:** add `native/src/original_actor_type2_vu0_target.cpp` to
the native target already containing `original_actor_random_triangle.cpp`,
`original_game_random.cpp` and the original spatial interface.
Register `native/tests/original_actor_type2_vu0_target_test.cpp` alongside
the other original-actor tests. Do not modify the navigation implementation
to duplicate the VU0 calculations. If the actor controller API needs a
span/pointer adapter, implement that at integration while maintaining
the 16-byte owner store semantics.

## Verification and limits

- `native/tools/verify_original_actor_type2_vu0_57bdcc.py` authenticates
  original ELF SHA + all 67 source instruction words, original kind-2
  gating, three exact vertex read calls, three packed source-vertex
  operations, two original game RNG JALs, five VU0 instructions with
  source VF registers/masks, both original constant triplets,
  the epilogue branch and the exact `SQC2 VF3,+0x40` write.
- Private original `woods_c/navigation.pck` SHA-256:
  `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.
  The C++ regression checks **59 original polygons**, **137 original
  links**, and all **15 original kind-two** edges, repeating type-two
  target vector checks for four seeds. It verifies exact output bits
  (including zero X/Y), preserved W, two genuine RNG draws, and full
  source `SQC2` 16-byte storage.
- Locally compiled and run both GCC 14.2 `-O2 -Werror -ffp-contract=off -fno-fast-math`
  and Clang 17 `-O1 -g -fsanitize=address,undefined` using user-private
  original navigation data: **both passed**. This was a focused
  source-equivalent native build with limited headers, not a full CMake
  build, not integrated engine runtime or gameplay boot.
- Some PS2 VU0 denormal/NaN/underflow differences remain unverified
  against hardware. The original actor parent function's broader
  action/transition handling is not recovered here.

No proprietary original game data, executable or resources are
committed to the GitHub branch; no workflow was triggered.
