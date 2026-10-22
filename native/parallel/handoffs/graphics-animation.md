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
- `b8cf7a7`, `bb7841a`: `native/docs/ORIGINAL_RETAIL_RENDER_GATE_AND_INSTANCING.md`: detailed VA evidence, MIPS branches, source-derived renderer selector fields, semantics and downstream dependencies.
- `1b928ff`, `f4b29df`, `b836070`: `native/include/ae3/original_render_selector.hpp`, `native/src/original_render_selector.cpp`, `native/tests/original_render_selector_test.cpp`: recovered original `0x26AEB4/0x26AEC8/0x26AED4/0x26AEE0` set/read/clear selector lifecycle, `0x26B550` optional actor resource wrapper, route flags at `0x280888–0x2808D0` controlling the resource-selected pass and independent actor flag `+0x33` secondary pass.
- `5e50b5f`: ELF audit fingerprints all three selector/dispatch instruction regions; no retail bytes checked in.

## Supplemental original rendering source evidence

The original **render resource selector** is now recovered as data/route behavior (not a host D3D draw implementation). Source routines: `0x26AEB4` sets the nested resource's selector at `+0x14` and enables byte `+0x10`; `0x26AEC8` returns byte `+0x10`; `0x26AED4` returns U32 `+0x14`; `0x26AEE0` clears enable without clearing its selected value. `0x26B550` safely returns zero if object pointer at `+0x110` missing, otherwise forwards the enable query; `0x26B57C` forwards selection without a source null check. In render loop `0x280888–0x2808D0`, resource-selected pass is entered only when enabled, while object byte `+0x33` independently gates another draw path. Local read-only SHA256 fingerprints verified on source at `0x26AEB4–0x26AEEC`, `0x26B550–0x26B594`, and `0x280884–0x2808D4`. Exact tests and reference hashes in graphics audit doc. The four SHA-256 instruction-range checks (including the early gate) were re-run against the owner's verified ELF: **4/4 match**.

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

g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \\
  -I native/include native/src/original_render_selector.cpp \\
  native/tests/original_render_selector_test.cpp -o /tmp/render_selector_test
/tmp/render_selector_test
# PASS: source-derived set/read/clear semantics, optional owner, selected/secondary render-path flags
```

Executed equivalent source with **GCC 14.2.0**: 2/2 initial focused tests pass. Rebuilt and ran with **Clang 17.0.0**, `-fsanitize=address,undefined -fno-omit-frame-pointer -Werror`: 2/2 initial tests pass, no sanitizer diagnostics. Also ran new selector lifecycle/dispatch test with **both GCC and Clang**, `-O2 -Werror` and `-O1 -fsanitize=address,undefined -fno-omit-frame-pointer -Werror`: **4/4 additional compiler/configuration tests pass**, no sanitizer diagnostics. During local test compilation a copy of the repository's public `gltf_scene.hpp` was used, no proprietary data.

Re-ran source-equivalent SHA/xref audit against the private 6,488,148-byte ELF: 6/6 anchors and 11/11 reference VAs match expected; retail gate instruction fingerprint matches. The newly committed Python script was not itself executed from a complete GitHub checkout. Updated GLB color regression test was added but **has not been run** against cgltf in the offline environment. No whole-project CMake/CTest run and no native Windows game boot/actual stage draw occurred in this workstream.

## Precise request for Windows Runtime integrator

Only integration owner should edit `native/CMakeLists.txt` and `app_win32.cpp`.

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_render_instances.cpp
  src/original_render_visibility.cpp
  src/original_render_selector.cpp)
add_executable(Getchu3OriginalRenderInstancesTests tests/original_render_instances_test.cpp)
target_link_libraries(Getchu3OriginalRenderInstancesTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRenderInstances COMMAND Getchu3OriginalRenderInstancesTests)
add_executable(Getchu3OriginalRenderVisibilityTests tests/original_render_visibility_test.cpp)
target_link_libraries(Getchu3OriginalRenderVisibilityTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderEarlyGate COMMAND Getchu3OriginalRenderVisibilityTests)
add_executable(Getchu3OriginalRenderSelectorTests tests/original_render_selector_test.cpp)
target_link_libraries(Getchu3OriginalRenderSelectorTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderSelectorAndDispatch COMMAND Getchu3OriginalRenderSelectorTests)
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
2. Decompile and wire `0x26C9B8` render cull dependency: it delegates into `0x26C4B0` (observed in the verified retail executable), which contains VU0/COP2 geometry operations and iterates original spatial-list nodes. This crosses the VU0/Physics team's expertise and requires their authentic math/rejection behavior; request an interface from that team, not a fake frustum or render object substitution. Then recover **remaining behavior and tables downstream of already recovered `0x26B550/0x26B57C` selector wrappers**, `0x2803B4/0x2804F4` draw paths, second render sub-pass near `0x280D90`. Source behavior and function boundaries currently **partially** recovered.
3. **Gameplay and Lua** must supply original object IDs, bone/attachment mappings, per-actor clip indices/pose times, visibility masks/flags, and stage transforms. **Windows Runtime** must connect this queue to the actual game update/draw loop.
4. Original camera matrices, material render modes, transparency, depth sort/LOD, textures, PS2-specific effects and lighting remain unrecovered. Host-native staging is not proof of visual accuracy.
5. End-to-end tests still required on real owner-original models, animations, monkey actors, levels, runtime frames, Windows Direct3D device and original game state.

## Next connected native pass

Reverse engineer the **complete original render traversal** starting at `0x2807xx` and its second pass `0x280Dxx`; recover `0x26C9B8` bounds and the remaining graphics dispatch downstream of recovered `0x26B550/0x26B57C`, then attach those to render-instance visibility and GPU draw submissions without guessed behavior. Obtain original model inputs from Assets and test original stage and actor scene composition before any playable claim.

**Boundaries respected:** no original copyrighted ELF/models/Lua/PCK textures committed, no PS2 emulation/replacement scene, no cross-owner CMake/app edits, no manual GitHub Actions.


---

## Graphics / Animation continuation — native retail render pipeline pass 2 (2026-10-21)

**Scope:** Continued only on `parallel/graphics-animation`; no edits to shared CMake, `app_win32.cpp`, Assets, Gameplay, VU0/Physics, master/default branches or GitHub Actions.

### New source and commits

- `27eaeae`, `fe3bb61`, `a06ae6e`: `native/include/ae3/original_render_pass.hpp`, `native/src/original_render_pass.cpp`, `native/tests/original_render_pass_test.cpp` — original **two distinct** retail object draw traversals recovered, with source-order call events and original entry/rejection tests.
- `cd70a9d`, `9042419`, `facb8c6`, `2c1f3f0`, `27a58d0`, `0ed52dc`: extend retail pass trace with admitted actor identities in traversal order, and graphics-owned `OriginalRenderWorld::composeFrameInOriginalOrder(ids)` to select actual original asset GLB instances for the D3D11 scene uploader without host layer resorting. Tests exercise multiple independently positioned actor instances, shared texture resources, per-actor animated samplers, source draw order, omitted visibility, duplicate and missing actor failures. No generic replacement scene is introduced.
- `9d4fd19`, `0fe7e4c`, `b6a5645`: `native/include/ae3/original_render_owner_schedule.hpp`, `native/src/original_render_owner_schedule.cpp`, `native/tests/original_render_owner_schedule_test.cpp` — original render owner record scan/activation, 64-bit frame counter, 0x90-byte record stride, mode/interval modulo admission, per-record +4/+5 handling. Original initializer at 0x51CDAC remains injectable and mandatory when required.
- `e0a6340`, `3af740d`: updated SHA-verified original retail ELF source audit with ten additional fingerprinted instruction ranges.
- `f3ee9e9`: `native/docs/ORIGINAL_RETAIL_RENDER_PASSES.md` provides the exact data layout, source instruction VAs, source control-flow semantics, retail SHA fingerprints, tests and Windows integration recipe.

### Native semantics established from original executable

**Primary object pass:** `0x280810–0x280968`, alternate actor-flag path at `0x280A90`: 64-bit group-mask and +0x30/+0x35 checks; spatial cull `0x26C9B8` only if byte +0x32 set; source selector `0x26B550/0x26B57C`; selected resource `0x2802D4` and bind `0x3B76F8`; actor +0x33 chooses `0x2803B4` versus `0x3B76E8`; actor +0x110 nested resource byte +0x08 checked **twice** using `0x26B3D0`; byte +0x34 controls resource clear/restore and final bind; source actor mode `0x26831C(...,1)`.

**Secondary object pass:** `0x280E20–0x280EB8`: checks bytes +0x30/+0x35, **always** calls `0x26C9B8` for candidate objects (no +0x32/mask check), flag+0x33 chooses draw/restore, actor mode `0x26831C(...,0)`.

**Render-owner outer scheduler:** `0x280B7C–0x280C80`, owner +0x34 stage slot vector, slot stride 0x90, owner +0x40 counter increments as full 64-bit; +0x38 mode selects special mode 1, +0x3C signed interval clamped to 1; byte4 zero invokes `0x51CDAC`, byte4 always reset in branch delay slot; byte5 false skips record; special schedule uses `0x4233F8` remainder of frame counter over interval compared to ordinal among active records. This preserves frame staggering in original stage render ownership.

**Retail input verification:** Re-examined SHA-matching private `SCUS_975.01`, checked original instruction-byte hashes for both complete traversal ranges, effect wrapper, selector resource setting, source fallback branch and owner scheduler. Refer to `native/docs/ORIGINAL_RETAIL_RENDER_PASSES.md` for ten exact VA-range SHA fingerprints. Only SHA checks and source code are committed; original game data remains private.

### Tests actually executed

Local focused test set in `/mnt/data/graphics_work` (the local sandbox is not the whole Git repository; tests use graphics-owned headers, source and synthetic structural fixture only). Compiler commands use C++20, `-Wall -Wextra -Wpedantic -Werror`, with `-I native/include`.

| Suite | GCC 14.2.0 `-O2` | Clang 17.0.0 `-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer` |
| --- | --- | --- |
| Original render instances + traversal-order scene composition | PASS | PASS |
| Original retail early visibility gate | PASS | PASS |
| Original resource selector & routing | PASS | PASS |
| Original primary/secondary pass control flow | PASS | PASS |
| Original render-owner frame scheduler | PASS | PASS |

**10/10 focused test executions pass**; no sanitizer findings. Original ELF retail SHA/range checks also pass. This does NOT constitute a real Windows boot, D3D11 draw, model visual validation or full-project CTest. Existing `gltf_scene_test.cpp` color change still has not been recompiled against cgltf in this offline environment.

### Precise Windows Runtime integration request (integrator only)

Add the existing prior-pass graphics sources and new ones into `Getchu3Assets` if not already present:

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_render_instances.cpp
  src/original_render_visibility.cpp
  src/original_render_selector.cpp
  src/original_render_pass.cpp
  src/original_render_owner_schedule.cpp)
add_executable(Getchu3OriginalRenderPassTests tests/original_render_pass_test.cpp)
target_link_libraries(Getchu3OriginalRenderPassTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailPrimarySecondaryRenderControl COMMAND Getchu3OriginalRenderPassTests)
add_executable(Getchu3OriginalRenderOwnerTests tests/original_render_owner_schedule_test.cpp)
target_link_libraries(Getchu3OriginalRenderOwnerTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderOwnerSchedule COMMAND Getchu3OriginalRenderOwnerTests)
```

Keep previously requested three tests for OriginalRenderInstances, OriginalRenderVisibility and OriginalRenderSelector in CMake as well. No extra third-party library is needed for these new modules.

Original ownership integration sequence:
1. Convert real loaded game stage render-owner +0x34 records into `OriginalRetailRenderOwner::slots`, with stable actor IDs and real flag byte snapshots; wire source `0x51CDAC` validator (currently missing).
2. Read original render object +0x20 masks, +0x30/+0x32/+0x33/+0x34/+0x35 flags, +0x110 resource pointers and source animation state. Call `traceOriginalPrimaryRenderPass(list,contextMask58,providers)` or `traceOriginalSecondaryRenderPass(list,providers)` as dictated by actual original engine lifecycle; feed `OriginalRetailDrawProviders` with original bounds (`0x26C9B8`) and resource-effect (`0x26B3D0`) bindings.
3. Use `OriginalRenderWorld::composeFrameInOriginalOrder(trace.acceptedIds)` to select and transform owner-extracted mesh data for native Direct3D upload, not to replace original shader/effect execution. Track original draw-call events `trace.events` to recover actual `0x2803B4`, `0x26831C`, `0x2804F4`, `0x3B76F8` and `0x3B76E8` operations. Do **not** silently drop resource events if implementing final render output.

### Blocking dependencies and next large pass

- **VU0/Physics:** source `0x26C9B8` dereferences the render-list owner and calls `0x26C4B0`, which contains genuine VU0 vector/polygon/list tests. The current render pass intentionally fails closed when the original spatial-cull function is required but not provided.
- **Assets:** need real retail PCK/I3D material/mesh payloads and I3M bone/attachment sources, without committing game data.
- **Gameplay/Lua:** supply real stage owner hierarchy, actor ID/flags/animation time and render owner list activation; no invented stage actors.
- **Windows Runtime:** integrate scene queue and original render ownership into its native game loop and update CMake/tests. Full Windows build and D3D11 visual tests are integration-owned.
- **Graphics itself:** decode `0x2803B4`, `0x26831C`, `0x2804F4`, resource tables/material state, second-pass render effects, actual original camera and matrix hierarchy, then validate genuine levels/monkeys/animation against private assets. Source-driven event order is **not** complete graphics rendering.



### Joined native render pipeline verification

- Commit `e9b7c15`: `native/tests/original_render_pipeline_test.cpp` exercises the recovered modules **together**, not separately: original owner 64-bit frame modulo selects two stage/animated actor objects across successive frames; primary-pass original visibility checks preserve those IDs; `composeFrameInOriginalOrder` produces correctly translated Direct3D11 `StaticScene` vertices, source color factors, texture batches and the independently sampled actor clip. The secondary pass verifies unconditional original cull calls and unchanged order for both original objects.
- Test uses an explicitly game-free triangle as a structural unit-test fixture, **not** a runtime game model replacement.
- Compiled and executed on GCC 14.2.0 `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`: **PASS**.
- Compiled and executed on Clang 17.0.0 `-std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Werror`: **PASS**, no sanitizer findings.
- Total this continuation: **six source-owned focused suites, twelve successful GCC/Clang test executions**. Real stage rendering and Windows app integration are still unverified.
- Windows integration owner: include the joined test in `native/CMakeLists.txt`:

```cmake
add_executable(Getchu3OriginalRenderPipelineTests tests/original_render_pipeline_test.cpp)
target_link_libraries(Getchu3OriginalRenderPipelineTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRenderOwnerThroughNativeScenePipeline COMMAND Getchu3OriginalRenderPipelineTests)
```

