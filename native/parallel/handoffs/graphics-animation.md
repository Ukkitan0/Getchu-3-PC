# Graphics, Models & Animation — specialist handoff

**Specialist branch:** `parallel/graphics-animation`
**Starting commit:** `d483436416c14927851853f01c6ad66fe4cfbbf2`
**Integrator target:** `native/original-elf-engine-recovery-20261020`
**Status:** Native scene-instancing and original early visibility gate implemented and unit tested; original USA executable evidence verified. Full retail stage/actor rendering is **NOT integrated or demonstrated**.

## Delivered graphics commits and source

- `0d52f7a`, `4543a3f`, `c72012f`: New `native/include/ae3/original_render_instances.hpp`, `native/src/original_render_instances.cpp`, `native/tests/original_render_instances_test.cpp`. Independently named model instance IDs, world transforms, per-actor clip/time sampler callbacks, scene and visibility flags, explicit layers, material draw batches, shared texture identity reuse, output D3D-compatible `StaticScene`, safety budgets/bounds checks, and per-instance draw metadata.
- `54470c4`, `021624c`: `native/src/gltf_scene.cpp` and `native/tests/gltf_scene_test.cpp`. Removed invented directional sunlight; preserve source-exported baseColorFactor times COLOR_0 and assert exact unmodified color.
- `d437cf0`, `a91d4c5`, `03a1eac`: `native/tools/audit_original_graphics.py`. Read-only SHA-gated retail ELF analysis of six string-xref groups and a source instruction fingerprint. No executable bytes checked in.
- `bc982c6`, `7eafa46`, `4402270`: `native/include/ae3/original_render_visibility.hpp`, `native/src/original_render_visibility.cpp`, `native/tests/original_render_visibility_test.cpp`: source-derived first-pass mask/flag/optional bounds-check gate.
- `b8cf7a7`: `native/docs/ORIGINAL_RETAIL_RENDER_GATE_AND_INSTANCING.md`: detailed VA evidence, MIPS branches, semantics and downstream dependencies.

## Original executable / graphics evidence

Owner-private original USA `SCUS_975.01` was recovered from previously uploaded private archive, examined **only in a temporary local workspace** and never committed. SHA-256 matches original project reference:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

- `I3dRender` label VA `0x6EB3F8`, referenced at `0x253748`.
- Matrix array labels: local `0x6ECE40` referenced `0x268F38/0x269164`; model `0x6ECE58` referenced `0x268F58/0x269184`.
- `calc render` label `0x6EDC40` references `0x2807B8/0x280A48`; `calc render sub` `0x6EDC60` references `0x280D90/0x281070`.
- `camera data` `0x6C3FD0` references `0x114EAC/0x114F54`.
- **Recovered first render gate `0x280838–0x280884`**: 64-bit `object+0x20 & scene+0x58` group overlap; enabled source object flags at byte `+0x30` and `+0x35`; if byte `+0x32` is nonzero, call spatial rejection `0x26C9B8(context+0x2C, object+0x50, object+0xC0)` and skip when nonzero. Branch target skip `0x280958`; next source call `0x26B550`. This is an **early eligibility gate only**, not the complete original visibility/culling, render ordering or scene renderer.
- Exact `0x280838–0x280884` instruction-region SHA-256 `a2ed3f2062ea0d0f7de6d252dd468d1f8345833d57a950cf1f5eb67b64a9fa00` (verified against private retail input). All details and limitations are in the dedicated source audit document.

## Actual tests completed

Local, standalone focused tests with minimal structural fixture, not game data:

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  -I native/include native/src/original_render_instances.cpp \
  native/tests/original_render_instances_test.cpp -o /tmp/render_instances_test
/tmp/render_instances_test
# PASS: instancing, texture dedup, world-space placement, independent animation callback,
# visibility, layer order, bounds, duplicate-ID and bad-texture validation

g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  -I native/include native/src/original_render_visibility.cpp \
  native/tests/original_render_visibility_test.cpp -o /tmp/render_gate_test
/tmp/render_gate_test
# PASS: 64-bit render mask, enable flags, conditional original bounds rejection and missing-callback error
```

Executed equivalent source with **GCC 14.2.0**: 2/2 focused tests pass. Rebuilt and ran with **Clang 17.0.0**, `-fsanitize=address,undefined -fno-omit-frame-pointer -Werror`: 2/2 pass, no sanitizer diagnostics. During local test compilation a copy of the repository's public `gltf_scene.hpp` was used, no proprietary data.

Re-ran source-equivalent SHA/xref audit against the private 6,488,148-byte ELF: 6/6 anchors and 12/12 reference VAs match expected; retail gate instruction fingerprint matches. The newly committed Python script was not itself executed from a complete GitHub checkout. Updated GLB color regression test was added but **has not been run** against cgltf in the offline environment. No whole-project CMake/CTest run and no native Windows game boot/actual stage draw occurred in this workstream.

## Precise request for Windows Runtime integrator

Only integration owner should edit `native/CMakeLists.txt` and `app_win32.cpp`.

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_render_instances.cpp
  src/original_render_visibility.cpp)
add_executable(Getchu3OriginalRenderInstancesTests tests/original_render_instances_test.cpp)
target_link_libraries(Getchu3OriginalRenderInstancesTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRenderInstances COMMAND Getchu3OriginalRenderInstancesTests)
add_executable(Getchu3OriginalRenderVisibilityTests tests/original_render_visibility_test.cpp)
target_link_libraries(Getchu3OriginalRenderVisibilityTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderEarlyGate COMMAND Getchu3OriginalRenderVisibilityTests)
```

To use real owner asset data (illustrative glue to be placed **only** by integrator):

```cpp
ae3::OriginalRenderWorld world;
ae3::OriginalRenderObject stage{};
stage.id=realStableObjectId;
stage.sourceName=realModelResourcePath;
stage.world=ae3::gamePlacementMatrix(originalPlcXYZ);
world.addStatic(stage,std::make_shared<ae3::StaticScene>(ae3::loadStaticGlb(realConvertedModelPath)));

auto skinned=std::make_shared<ae3::NativeSkinnedGlb>(realAnimatedGlbPath);
ae3::OriginalRenderObject character{};
character.id=realActorInstanceId;
character.sourceName=realAnimatedGlbPath.string();
world.addAnimated(character,[skinned](std::size_t clip,double t)->const ae3::StaticScene&{
    return skinned->sample(clip,t);
});
// Reconstruct original actor transform, clip, and visibility each frame:
world.find(realActorInstanceId)->animationSeconds=originalActorClipTime;
ae3::OriginalRenderFrame frame=world.composeFrame();
// Existing private NativeRenderer::displayScene(frame.scene, true), then
// NativeRenderer::updateAnimation(frame.scene) when topology stays fixed.
```

One **separate NativeSkinnedGlb per animated actor** is required to avoid shared mutable CPU skinning pose; each character timeline is owned by original gameplay state. If vertex count or draw batches/texture bindings change, refresh via `displayScene` (which currently recreates the VB and texture resources); `updateAnimation` only updates positions and requires same vertex count. Current app's 65-degree debug camera and ad-hoc draw states are not original game code; replacing them requires further retail recovery. Other team sources have **not** been modified.

## Outstanding blockers (not disguised as completion)

1. No private full stage/model GLB/PCK assets were available for a real rendering test. Coordination: **Assets team** must surface exact, source-derived texture/material/model and I3M ownership/path mappings; no asset bytes in repo.
2. Decompile and wire `0x26C9B8` render cull dependency, `0x26B550/0x26B57C` render-state calls, `0x2803B4/0x2804F4` draw paths, second render sub-pass near `0x280D90`. Source behavior and function boundaries currently **partially** recovered.
3. **Gameplay and Lua** must supply original object IDs, bone/attachment mappings, per-actor clip indices/pose times, visibility masks/flags, and stage transforms. **Windows Runtime** must connect this queue to the actual game update/draw loop.
4. Original camera matrices, material render modes, transparency, depth sort/LOD, textures, PS2-specific effects and lighting remain unrecovered. Host-native staging is not proof of visual accuracy.
5. End-to-end tests still required on real owner-original models, animations, monkey actors, levels, runtime frames, Windows Direct3D device and original game state.

## Next connected native pass

Reverse engineer the **complete original render traversal** starting at `0x2807xx` and its second pass `0x280Dxx`; recover `0x26C9B8` bounds and `0x26B550` graphics state dispatch, then attach those to render-instance visibility and GPU draw submissions without guessed behavior. Obtain original model inputs from Assets and test original stage and actor scene composition before any playable claim.

**Boundaries respected:** no original copyrighted ELF/models/Lua/PCK textures committed, no PS2 emulation/replacement scene, no cross-owner CMake/app edits, no manual GitHub Actions.
