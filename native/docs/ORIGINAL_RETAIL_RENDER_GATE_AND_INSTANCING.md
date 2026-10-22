# USA original graphics recovery: render early gate and native scene submission

## Retail evidence (verified private original executable)
- Owner's **USA** `SCUS_975.01` SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- ELF32 MIPS little-endian, executable `.text` mapped at 0x00100000, file offset 0x1000; original game binary is **not** in Git.
- Source-owned helper: `native/tools/audit_original_graphics.py <private SCUS_975.01>` checks SHA, six named rodata addresses and their LUI references, and the exact hash of 0x00280838–0x00280884.
- Original graphics-related references (all checked against private bytes):

| Label | String VA | Referencing instruction VAs |
| --- | --- | --- |
| I3dRender | 0x006EB3F8 | 0x00253748 |
| local matrix array | 0x006ECE40 | 0x00268F38, 0x00269164 |
| model matrix array | 0x006ECE58 | 0x00268F58, 0x00269184 |
| calc render | 0x006EDC40 | 0x002807B8, 0x00280A48 |
| calc render sub | 0x006EDC60 | 0x00280D90, 0x00281070 |
| camera data | 0x006C3FD0 | 0x00114EAC, 0x00114F54 |

**Important:** these are instruction and data *references*, not proof that the complete functions were ported. Their main usefulness is focusing the remaining rendering decompilation.

## Actual retail control flow recovered: early scene-render filter

At **0x00280838–0x00280884** the original MIPS code (roughly translated, including actual branch-on-likely skip control flow) does:

```cpp
if ((read_u64(object+0x20) & read_u64(renderContext+0x58)) == 0)
    continue; // jumps to loop increment at 0x280958
if (read_u8(object+0x30) == 0) continue;
if (read_u8(object+0x35) == 0) continue;
if (read_u8(object+0x32) != 0 &&
    sub_26C9B8(read_ptr32(renderContext+0x2C), object+0x50, object+0xC0))
    continue;
// next retail routine at 0x26B550
```

`native/src/original_render_visibility.cpp` implements only this exact early gate, taking extracted values from original game owner objects. A callback supplies the **unrecovered** `0x26C9B8` spatial rejection; it throws instead of supplying guessed frustum behavior when the callback is needed but missing. Downstream `0x26B550`, `0x26B57C`, `0x2803B4`, `0x2804F4`, `0x3B76F8`, and their draw/visibility consequences remain unported. This gate returning `true` means *eligible for subsequent original routines*, **not guaranteed visible**.

## Native rendering integration delivered

`original_render_instances.hpp/.cpp` is a native (not PS2/GPU emulation) scene and actor-submission bridge. It consumes the actual `StaticScene` geometry, material batches, UVs and textures, and per-actor `NativeSkinnedGlb::sample` outputs via a callback. For each frame it constructs one D3D-compatible `StaticScene` plus draw records containing stable actor IDs, source names and batch ranges. It preserves separate placements/animation times; it never fabricates world geometry or game behavior. Visibility flags and layers must originate from recovered gameplay/render state. An `OriginalRenderFrame` does **not** implement original camera, blend states, PS2 material microcode or visibility transitions.

This fixes a major architectural gap: `level_scene.cpp` previously merged all original exported meshes into static anonymous vertices and `app_win32.cpp` supported just one animated model. The new bridge can represent arbitrary distinct model instances and multiple independently animated sources, **but runtime integration has not been performed by this graphics specialist**.

Existing `gltf_scene.cpp` also applied invented host directional sunlight to every exported GLB material color. That artificial multiplier was removed: exported `baseColorFactor * COLOR_0` is passed unchanged. This is not yet a restoration of the original game's dynamic light/render state.

## Next retail work

1. Decompile `0x26C9B8` and its matrix/spatial dependencies, then connect it to the early gate with actual original scene-context and object layouts.
2. Recover remaining call tree around `0x2802D4/0x2803B4/0x2804F4` and second render pass around `0x280D90`; derive original visibility/Lod/material order rather than guessing.
3. Coordinate authentic PCK/I3D/texture data with Assets; wire per-object scene IDs, placements and animation update from original Stage/Lua/Gameplay (separate team).
4. Windows integrator connects `OriginalRenderFrame.scene` to `NativeRenderer::displayScene(scene,true)`; call `updateAnimation(frame.scene)` only while triangle counts and batches remain fixed. Dynamic entity creation/removal or changing draw topology needs re-upload. Replace the current host debug camera with recovered game camera and draw state when source-proven.
5. Test on the owner's real stage I3D models and original I3M-driven GLBs with two independently moving actors; actual full game rendering is **not verified**.

All synthetic test geometry is used exclusively as unit-test inputs and is **never** a replacement game stage/model.
