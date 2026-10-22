# Ape Escape 3 USA — complete native 0x57C98C random three-vertex actor target

## Genuine retail source recovered, not replaced

The source is the owner's original `SCUS_975.01` (SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`).
The original game uses the source RNG global at `0x6A8E48`.
This recovery consumes an instance of the already recovered native
`OriginalGameRandom_36f3c0`, retaining the authentic evolving state.

The original `0x57C98C..0x57CA64` actor-controller branch is 55
retail instruction words; `verify_original_actor_random_triangle.py`
confirms five original source calls and five original VU0 operations.

## Exact recovered source sequence

- `0x57C98C`, `0x57C99C`, `0x57C9B0`: source
  `0x21B458` returns 16-byte vertices 0, 1, and 2.
- `0x57C9BC`: source `0x36F4D4` draws float `a`.
- `0x57C9D4`: original `0x708A64` contains 1.0f.
  `0x57C9DC`: `remainder=1-a`.
- `0x57C9E0`: source `0x36F548(0.0f, remainder)`
  draws float `b`. Original `0x57C9EC` computes `c=remainder-b`.
- Source floats `0x708A68=0x3F4CCCCD` (0.8) and
  `0x708A6C=0x3D888888` (approximately 0.06666666)
  give native float32 weights `wa=a*0.8+base`,
  `wb=b*0.8+base`, `wc=c*0.8+base`.
- Original macro COP2 instructions at `0x57CA28`,
  `0x57CA40`, and `0x57CA54` are three `VMULx.xyzw`;
  `0x57CA5C`, `0x57CA64` are two `VADD.xyzw`,
  applied in source vertex order:
  `((vertex0 * wa) + (vertex1 * wb)) + (vertex2 * wc)`.
- Original `0x57C948` returns the **low 64 bits** of
  the resulting `VF1`, which the native API returns as
  the exact x/y bits rather than a platform-dependent struct.

This is the game's original random point-within-triangle calculation,
including its inset toward the polygon center. It is **not** a generic
or newly invented barycentric algorithm: every original constant,
RNG call, VU0 multiply/add and source-vertex ordering is anchored
to the actual machine code.

## Connected native engine path

The new `native/src/original_actor_random_triangle.cpp` implements
the entire branch. `OriginalActorNavigationPipeline_57c7cc` now
accepts the original game RNG state as an optional constructor
argument; **when supplied**, source variant-zero actor target
selection goes straight to this recovered C++ function, without
calling the prior placeholder `0x57C98C` transform. Legacy tests
can keep using the old source-callback interface when there is no
owned RNG yet, avoiding unrelated breakage.

The new `native/tests/original_actor_random_triangle_test.cpp`
checks multiple deterministic retail seeds, two-draw RNG progression,
the exact packed target result, and connects genuine decoded
`woods_c/navigation.pck` source vertices to the **full native
route follower -> source target dispatcher -> original game RNG ->
source vector operations** for every original polygon in both
retail navigation graphs (**59 total**).

Local isolated and integrated tests passed with GCC Release,
Clang Release, AddressSanitizer and UndefinedBehaviorSanitizer.

## Remaining unsolved original source, no emulation

`0x57CA68` remains the complex randomized VU0 fallback in the
other controller variant; `0x4C1B3C` and its
`0x21BD3C` source geometry/route projection are not completely
translated. The full actor tick/update, native original renderer
and Windows boot remain blocking tasks. This commit does not
claim the full game runs.

No game ELF, ISO, Lua or polygon data is committed publicly.
No emulated PlayStation CPU/memory implementation was added,
and no GitHub Actions were deliberately invoked.
