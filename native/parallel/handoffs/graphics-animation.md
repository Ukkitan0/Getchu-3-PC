# Graphics, Models & Animation — specialist handoff

**Specialist branch:** `parallel/graphics-animation`
**Starting commit:** `d483436416c14927851853f01c6ad66fe4cfbbf2`
**Integrator target:** `native/original-elf-engine-recovery-20261020`
**Status:** Native render traversal, original context register stores, actor resource/clock/effect wrapper recovered and tested; a previous cull polarity error is corrected. Original stage/character drawing is **NOT yet integrated or demonstrated**.

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
- **Recovered first render gate `0x280838–0x280884`**: 64-bit `object+0x20 & scene+0x58` group overlap; enabled source object flags at byte `+0x30` and `+0x35`; if byte `+0x32` is nonzero, call spatial rejection `0x26C9B8(context+0x2C, object+0x50, object+0xC0)` and skip when **ZERO** (actual original `beqzl $v0` at `0x28087C`; previous opposite statement corrected in pass 3). Branch target skip `0x280958`; next source call `0x26B550`. This is an **early eligibility gate only**, not the complete original visibility/culling, render ordering or scene renderer.
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


---

## Graphics specialist continuation — Original context, actor state and effect pipeline (pass 3)

**Latest source:** owner-private retail USA `SCUS_975.01` verified SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Primary source cross-check used the original executable's MIPS disassembly (`llvm-objdump`) and a direct 10-region SHA-256 audit. No copyrighted data added to Git.

### Crucial functional correction from actual retail instructions

At `0x28087C` original MIPS **`beqzl $v0,0x280958`** skips the first-pass actor when `0x26C9B8` returns **ZERO**. At `0x280E6C`, the secondary pass also **skips on ZERO**. The older graphics branch mistakenly treated a nonzero return as rejection; that logic could hide genuinely visible actors.

**Fixed at the public API:** `rejectByOriginalBounds` renamed to `originalBoundsVisible` in `original_render_visibility.hpp/.cpp`, `original_render_pass.hpp/.cpp`, and their tests. Both passes now advance on a nonzero source result and skip on zero; the first invokes the callback only when source actor byte `+0x32` is set, whereas the second invokes it for every eligible actor. Source VU0 callback is mandatory when needed. Tested under GCC/Clang.

### Exact original graphics state and actor lifecycle delivered

- `native/include/ae3/original_render_context.hpp`
- `native/src/original_render_context.cpp`
- `native/tests/original_render_context_test.cpp`

  Original renderer context setters `0x3B76D0` / `0x3B76E8` / `0x3B76F8` / `0x3B7708` store native resource handles in source offsets `+0x828`, `+0x82C`, `+0x830`, `+0x834`; the first additionally copies source `+0x0C` into context `+0x818`. Original getters `0x3B7718–0x3B773C` read the corresponding slots. `applyOriginalRetailRenderTrace` now executes those recovered state changes in original event order, with a resource resolver and **mandatory actual drawing callbacks**. This is not a PS2 graphics emulator nor a finished GPU draw.
  
- `native/include/ae3/original_render_resource.hpp`
- `native/src/original_render_resource.cpp`
- `native/tests/original_render_resource_test.cpp`

  Actor render resource `0x26A854` allocates only when original actor `+0x110` is null. `0x26A510` initializes inner resource bytes `+0x00/+0x08/+0x10` and pointer fields `+0x40–0x50`; uninitialized retail float/index fields `+0x04/+0x0C/+0x14` stay marked unset in host state, not invented. Original wrappers `0x26AFAC`/`0x26B3D0`/`0x26B550` return zero without resource handle; original setters `0x26A8DC`/`0x26A968` and scalar activation `0x26A8AC`/`0x26A938` are recovered. Existing selector set/clear +0x10/+0x14 semantics are reused.

- `native/include/ae3/original_render_actor_entry.hpp`
- `native/src/original_render_actor_entry.cpp`
- `native/tests/original_render_actor_entry_test.cpp`

  Recovered entry `0x26831C–0x268424`: always call `0x23C9CC` and `0x23CAA4` with original owner `+0xFC`; call `0x23CAE8` if the second result is nonnegative; when actor U32 `+0xF8==1`, update float actor `+0xF0` by the separate multiply/add of runtime float `[0x635488]` and actor float `+0xF4`. Early return when actor `+0x30==0` or original global byte `[0x6496F0]!=0`. Callbacks for the three unknown `0x23Cxxx` routines are mandatory. Source global delta is runtime data, **not** the zero stored in the file.

- `native/include/ae3/original_render_effect.hpp`
- `native/src/original_render_effect.cpp`
- `native/tests/original_render_effect_test.cpp`

  Recovered the `0x2804F4–0x280540` preparation chain: ensure actor render resource via `0x26A854`, source wrapper `0x26A4AC` delegates into original `0x269F7C` descriptor converter, `0x3AE900` produces packet handle and `0x3B76F8` writes it to resource context `+0x830`. Since the original converter and packet body are not yet decompiled, their callbacks must be supplied or the call throws. This **does not implement a replacement effect**.

Detailed original instruction provenance: `native/docs/ORIGINAL_RETAIL_RENDER_CONTEXT_AND_ACTOR_ENTRY.md`.

### Source fingerprints and local tests

- `native/tools/audit_original_graphics.py` now adds **10 original executable SHA-256 instruction span checks** for the context stores, actor resource initialization, actor render clock, draw setup and packet/effect wrapper. Direct private-ELF SHA-256 checks in this session: **10/10 match**; additional earlier scan spans were not changed. Full executable hash matches project reference.
- Focused C++20 test suites:
  `original_render_visibility_test.cpp`,
  `original_render_selector_test.cpp`,
  `original_render_owner_schedule_test.cpp`,
  `original_render_instances_test.cpp`,
  `original_render_pass_test.cpp`,
  `original_render_pipeline_test.cpp`,
  `original_render_context_test.cpp`,
  `original_render_resource_test.cpp`,
  `original_render_actor_entry_test.cpp`,
  `original_render_effect_test.cpp`.
- Local **GCC 14.2.0**: all **10/10 PASS** with `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`.
- Local **Clang 17.0.0**: all **10/10 PASS** with `-std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Werror`; no sanitizer errors. One global batch timed out during its final build, so that test was recompiled and rerun separately and passed. **20/20 focused executions passed**.
- No bundled private original stage art was available for 3D scene visual verification; no full project CMake/CTest, Windows application build, actual D3D11 draw, game boot or playable milestone was claimed. The committed complete audit Python script itself was not run from a freshly checked-out Git clone (only equivalent raw-byte SHA checks on its source ranges were run).

### Windows Runtime integration request (Windows owner only)

Do **not** edit `app_win32.cpp` / `native/CMakeLists.txt` on this specialist branch. In shared CMake append:

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_render_context.cpp
  src/original_render_resource.cpp
  src/original_render_actor_entry.cpp
  src/original_render_effect.cpp)

add_executable(Getchu3OriginalRenderContextTests tests/original_render_context_test.cpp)
target_link_libraries(Getchu3OriginalRenderContextTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderContextRegisters COMMAND Getchu3OriginalRenderContextTests)

add_executable(Getchu3OriginalRenderResourceTests tests/original_render_resource_test.cpp)
target_link_libraries(Getchu3OriginalRenderResourceTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailActorRenderResources COMMAND Getchu3OriginalRenderResourceTests)

add_executable(Getchu3OriginalRenderActorEntryTests tests/original_render_actor_entry_test.cpp)
target_link_libraries(Getchu3OriginalRenderActorEntryTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailActorRenderEntryAndClock COMMAND Getchu3OriginalRenderActorEntryTests)

add_executable(Getchu3OriginalRenderEffectTests tests/original_render_effect_test.cpp)
target_link_libraries(Getchu3OriginalRenderEffectTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderEffectPreparation COMMAND Getchu3OriginalRenderEffectTests)
```

Use previously handed-off graphics modules/tests as well. `OriginalRetailDrawProviders::originalBoundsVisible(id)` **must return true only when the original 0x26C9B8 returns nonzero**. This API rename is required for any new integration wiring. Draw traces now have a native renderer-resource state executor, but real draw callbacks for `0x26831C` downstream geometry, `0x2803B4` VU0 transform/render-resource packet, and `0x2804F4` genuine descriptor/packet conversion remain necessary. Connect source-owned actor instance IDs/transform/animation state from Gameplay, stage data from Assets and original VU0 math from Physics.

### Blockers and next source-backed subsystem

**Next graphics-owned batch:** recover original `0x26841C` actor geometry draw path and `0x2803B4` transformation state production, then `0x269F7C` material descriptor conversion and `0x3AE900` packet values to native renderer-facing data. Ask VU0/Physics for real `0x26C4B0` visibility math; never replace it with a generic frustum. Integrate real I3D mesh data, I3M animation, material ordering and original camera before visual-validation claims.

No original copyrighted models/ELF/PCK/ISO bytes committed, no emulation, no changed shared build/launch/gameplay files, no GitHub Actions dispatched.


---

## Graphics specialist continuation — original model packets and genuine material groups (pass 4)

**Working branch:** `parallel/graphics-animation`, inherited source originally based on `d483436416c14927851853f01c6ad66fe4cfbbf2`. All work is graphics-owned; no shared app, CMake or other team's source changed.

### Original executable evidence

Reopened the owner's private USA `SCUS_975.01` from its previously supplied encrypted-free ZIP; SHA-256 matches reference `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Used local `llvm-objdump` on private bytes to inspect original render packet routines. This session's original byte comparisons yielded source-region hashes for:

- `0x280328–0x280364` original graphics packet wrapper; existing audit hash retained.
- `0x280364–0x2803B4` original descriptor initializer, model conversion and graphics bind.
- `0x2803B4–0x2804F4` original actor/special-model packet path; existing audit hash retained.
- `0x3AF8F0–0x3AF9C4` original **five material packet groups**, including each count gate and original function VA.
- `0x3AF8B8–0x3AF8F0` original packet reset/setup dependencies.
- `0x3AF478–0x3AF530` complex original group zero.
- `0x3AF600–0x3AF680` **original fixed 48-byte group copy**.
- `0x3AF680–0x3AF6F8` **original fixed 32-byte group copy**.
- `0x42B16C–0x42B224` retail general-purpose memory-copy routine.

New SHA fingerprints appended to `native/tools/audit_original_graphics.py`; actual evidence, semantics and exact fingerprints in `native/docs/ORIGINAL_RETAIL_MODEL_DRAW_AND_PACKET_ASSEMBLY.md`.

### New native interfaces, source paths and commits

- `985b80e` (header), `a996221` (C++), `9b31421` (test): `native/include/ae3/original_render_draw_packet.hpp`, `native/src/original_render_draw_packet.cpp`, `native/tests/original_render_draw_packet_test.cpp`. Typed native `OriginalDrawScratch` of size **0x390** with descriptor at `+0x330`; the actual retail **0x280364 -> 0x51C9B0 -> 0x2802F8/0x27C614 -> 0x280328 -> 0x3AF8F0 -> 0x3B76E8** chain for static model packets; original actor path from **0x2803B4 -> 0x280310/0x27C520 -> optional 0x26A854/0x26A8F4 -> 0x280328**, preserving byte+0x00 owner flag and actor+0x118 scratch source word. Unknown VU and original converter blocks are **required callbacks**, never guessed. Packet returned by `0x3AF8F0` updates original renderer context slot **+0x82C** through the already recovered setter.
- `4d2b111` (header), `16182be` (C++), `8bc1aa1` (test): `native/include/ae3/original_render_packet_assembly.hpp`, `native/src/original_render_packet_assembly.cpp`, `native/tests/original_render_packet_assembly_test.cpp`. Recovered original **0x3AF8F0** five-group assembler control flow. Preserves required original packet allocate `0x3B3CE0(gfx,4)`, reset `0x3AF8B8`, setup `0x3AF8A8`, and conditionally dispatches source groups at descriptor pair offsets **0x10/14, 0x18/1C, 0x20/24, 0x28/2C, 0x30/34**. Original handlers are **0x3AF478, 0x3AF680, 0x3AF6F8, 0x3AF530, 0x3AF600**. No guessed contents for the original packet attachment handlers.
- `770c878` (header), `e24dd7d` (C++), `790eb74` (test): `native/include/ae3/original_render_packet_copies.hpp`, `native/src/original_render_packet_copies.cpp`, `native/tests/original_render_packet_copies_test.cpp`. Directly recovered and portable original data copying for packet group 1 `0x3AF680` (**32 bytes per original record, 2 allocation units of 16 bytes**) and group 4 `0x3AF600` (**48 bytes/record, 3 units**). Produces owned native source bytes unchanged, fails on bad counts/truncation and rejects other source groups whose transforms remain unported.
- `7247d65`, `25b67cf`: SHA-gated original USA ELF audit updated with group-constructor and copy instruction fingerprints.
- `a3d4bab`: `native/docs/ORIGINAL_RETAIL_MODEL_DRAW_AND_PACKET_ASSEMBLY.md` with original game source provenance, ordered data dependencies, test commands and exact Windows integration recipe.

### Local verification (no game assets in Git)

Three new C++20 focused tests, all compiled and executed twice from the graphics-only test tree under `/mnt/data/graphics_work`.

**GCC (release):** `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -Inative/include`.
**Clang (sanitized):** `clang++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Werror -Inative/include`.

| Test | GCC | Clang ASan + UBSan |
| --- | --- | --- |
| Original static/actor descriptor and packet bind chains | PASS | PASS |
| Five original material packet groups + D3D context binding | PASS | PASS |
| Authentic binary packet copy lengths and budget limits | PASS | PASS |

**6/6 additional test executions pass**, no sanitizer warnings. All checks are **synthetic unit-test data**, not user's original graphics resources; no original level or actor animation was rendered on Windows, and full-project CMake/CTest was not run. The local tests used source-equivalent precommit copies; GitHub's committed files were independently reviewed but a newly cloned GitHub checkout was not built because outbound network access to GitHub was unavailable in the local compiler sandbox.

### Windows Runtime integration recipe (integrator only)

Add the following C++ source files and tests to the existing `Getchu3Assets` CMake target (do not modify CMake in specialist branch):

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_render_draw_packet.cpp
  src/original_render_packet_assembly.cpp
  src/original_render_packet_copies.cpp)
add_executable(Getchu3OriginalRenderDrawPacketTests tests/original_render_draw_packet_test.cpp)
target_link_libraries(Getchu3OriginalRenderDrawPacketTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailModelToPacket COMMAND Getchu3OriginalRenderDrawPacketTests)
add_executable(Getchu3OriginalRenderPacketAssemblyTests tests/original_render_packet_assembly_test.cpp)
target_link_libraries(Getchu3OriginalRenderPacketAssemblyTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailFiveMaterialGroupAssembly COMMAND Getchu3OriginalRenderPacketAssemblyTests)
add_executable(Getchu3OriginalRenderPacketCopiesTests tests/original_render_packet_copies_test.cpp)
target_link_libraries(Getchu3OriginalRenderPacketCopiesTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailPacketRecordCopies COMMAND Getchu3OriginalRenderPacketCopiesTests)
```

All three modules are C++20 with no external dependencies beyond other graphics-owned headers already included in previous handoff commits. To connect to original actors, pass actual model source pointers at **original model owner +0x28**, resource flags at **actor +0x110**, exact original descriptor data and actual original game resource tokens. Source group 1/4 can now produce native owned byte arrays directly using bytes resolved by Assets team. **Original global graphics context, original packet allocation, original packet setup, original 0x27C614/0x27C520 converters and original 0x51C9B0/VU block are not fabricated**: integration must provide source-equivalent implementations. The existing Windows D3D11 renderer has not been modified.

### Remaining blockers and next connected pass

The highest-value original graphics recovery is now translating the **three remaining material packet transforms** (`0x3AF478` group 0, `0x3AF6F8` group 2, `0x3AF530` group 3), their source VU/packed math and allocation semantics, plus the model conversion routines `0x27C614/0x27C520` and the actor VU block `0x28040C–0x2804C8`. The Assets team must expose original mesh and material buffers; VU0/Physics supplies math where ownership applies; the Windows Runtime integrator owns CMake and actual GPU integration. Do not call rendering/playability complete until real retail models and levels appear correctly in the native application.

**No original copyrighted ELF/models/textures checked in, no PS2 emulation, no app entry point/CMake/cross-team edits and no manual GitHub Actions.**


### Source-backed graphics packet arena (pass 4 extension)

The original packet constructor's allocation dependency has an accurately
recovered native cursor implementation:

- `1bc9ed2`: `native/include/ae3/original_render_packet_arena.hpp`.
- `d23a24a`: `native/src/original_render_packet_arena.cpp`.
- `e58bab4`: `native/tests/original_render_packet_arena_test.cpp`.
- `44ac9a1`: retail `SCUS_975.01` byte fingerprints added for original allocation VAs.

The original `0x3B3CE0` reads/writes graphics owner U32 at `+0x18`:
`back=(back-count*16)&~15`, returns **new** back.
The complementary `0x3B3D00` reads/writes `+0x14`:
`front=(front+count*16+15)&~15`, returns **old** front.
Both original instruction ranges were matched to the private ELF SHA-256:
`0x3B3CE0–0x3B3CFC` =
`13c8b65e6a2bcac7c1ffb3a980f280d4f2cbe70dcc81ca238fa0d27f993e8f92`,
`0x3B3D00–0x3B3D20` =
`4c2ea7ef8022fcd90bf053c968baa32e8650847c95829d708742e35e7639e9bd`.

Native host offsets preserve the original 16-byte alignment and return
conventions, with additional host-safe overflow/bounds/cursor-overlap rejection.
Neither a guest memory emulator nor a replacement allocator. The original
graphics arena buffer/source pointer ownership remains a Windows integration
dependency. This is the math for a real renderer allocation interface, **not
actual GPU allocation or game rendering**.

Focused suite built and run with GCC 14.2 `-O2 -Wall -Wextra -Wpedantic -Werror`:
**PASS**; Clang 17 `-O1 -g -fsanitize=address,undefined -Werror`: **PASS**,
no sanitizer findings. Pass 4 therefore contributes **4 new native suites,
8/8 GCC/Clang executions**. It does NOT include an actual Windows graphics
device rendering test.

Windows integration owner only, add:

```cmake
target_sources(Getchu3Assets PRIVATE src/original_render_packet_arena.cpp)
add_executable(Getchu3OriginalPacketArenaTests tests/original_render_packet_arena_test.cpp)
target_link_libraries(Getchu3OriginalPacketArenaTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRenderPacketArena COMMAND Getchu3OriginalPacketArenaTests)
```

Documentation: `native/docs/ORIGINAL_RETAIL_MODEL_DRAW_AND_PACKET_ASSEMBLY.md`
includes all retail details and updated integration recipe. These commits
do not touch any other category or shared orchestration files.


---

## Graphics continuation: retail transformed packet groups + true model source strides

**Source:** private USA SCUS_975.01, SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Do not commit game bytes.

### New graphics-owned files

- `native/include/ae3/original_render_transformed_segments.hpp`
- `native/src/original_render_transformed_segments.cpp`
- `native/tests/original_render_transformed_segments_test.cpp`
- `native/include/ae3/original_render_packet_storage.hpp`
- `native/src/original_render_packet_storage.cpp`
- `native/tests/original_render_packet_storage_test.cpp`
- `native/include/ae3/original_render_model_streams.hpp`
- `native/src/original_render_model_streams.cpp`
- `native/tests/original_render_model_streams_test.cpp`
- `native/docs/ORIGINAL_RETAIL_TRANSFORMED_PACKET_RECOVERY.md`

### Original instructions recovered

`0x3AF478` (packet group 0) has original record size **0x20**, 2 arena units/record and packet count/handle offsets +0x10/+0x14. `0x3AF6F8` (group 2) has original record size **0x40**, 4 units, offsets +0x20/+0x24. `0x3AF530` (group 3) has record size **0x30**, 3 units, offsets +0x28/+0x2C. All three require original `0x3BDF08` VU/packed transformations; native source code refuses to process nonempty records without a source-equivalent VU callback. No guessed memcpy substitution.

Inside group 2, the actual scalar original code `0x3AF7AC–0x3AF858` is ported: two original floats at source record +0x34/+0x30, 1.569796... upper clamp, 0.001... lower clamp, 0.95 relative limit, original math `0x41D3D8` called **twice** and `0x41D490` called **once**, output floats written at +0x30/+0x34/+0x38/+0x3C. Native callbacks for those two original math routines remain mandatory; replacing them with arbitrary std::sin/std::cos is not allowed until source equivalence is demonstrated. Fail-closed host-safe validation for nonfinite sources and invalid denominator.

Original `0x3AF8B8` packet reset and `0x3AF8A8` first-quadword copy are now directly implemented rather than abstract callbacks. Original packet header is **0x40 bytes**, zeroes bytes 0..59 (last word +0x3C undefined), saves original 32-bit context token at +0x38 and copies first 16 bytes of descriptor at +0x00. The packet storage uses the previously recovered original rear allocator `0x3B3CE0` and checks actual allocated-header identities.

Original renderer model conversion `0x27C614` consumes **five separate source streams**, begin/end pointer offset pairs [0x04/08,0x14/18,0x24/28,0x34/38,0x44/48] and authentic original per-record strides **[96,96,112,80,128]**. Actor conversion `0x27C520` additionally iterates `+0x54/+0x58` at 128-byte stride. The recovered decoder returns raw original numeric addresses and counts only, never casts PS2 pointers into Windows pointers. Per-record VU/mesh decoding remains incomplete.

The original executable's exact instruction-region SHA-256 fingerprints and the four original group-2 IEEE-754 constants are now pinned in `native/tools/audit_original_graphics.py`. Original source details and all hashes are in `native/docs/ORIGINAL_RETAIL_TRANSFORMED_PACKET_RECOVERY.md`.

### Actual focused testing and integration

A complete local graphics-focused compile-and-run regression was performed, using the source-equivalent staged C++20 files:

- **GCC 14.2** with `-O2 -Wall -Wextra -Wpedantic -Werror`: **17/17 suites pass**.
- **Clang 17** with `-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Werror`: **17/17 suites pass**, no sanitizer finding. The first global Clang batch timed out and remaining suites were linked/executed in a second call.
- The test fixtures use synthetic structural bytes only. This does **not** validate original model rendering or actual game boot.

**Windows integrator action only:** append three source files to `Getchu3Assets` and register the corresponding three `_test.cpp` executables in `native/CMakeLists.txt`, along with all prior graphics-owned modules. Source names: `original_render_transformed_segments.cpp`, `original_render_packet_storage.cpp`, `original_render_model_streams.cpp`. All are C++20 and source-owned.

**Next dependency requests:** Assets team resolve actual original source model ranges/asset bytes to native span; VU0 team provide original `0x3BDF08` quadword processing and `0x26C4B0` bounds; Graphics continue recovery of original `0x27C614/0x27C520` record transforms, `0x41D3D8/0x41D490` math and original render material/VU operations. Windows Runtime must connect this to the true model/stage owner and native Direct3D draw.

**Status:** Source-backed model/packet preparation has meaningfully advanced. Full genuine original model and level draw, animation, shaders, and playable native executable remain **unverified**. No shared CMake, app, or other specialists' paths modified; no GitHub Actions dispatched.
