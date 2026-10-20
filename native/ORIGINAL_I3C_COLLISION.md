# Native Ape Escape 3 I3D_I3C collision and wall movement

**Active architecture:** real C++20 and Direct3D11 Windows game engine, preserving Lua 5.0 stage scripts. **Absolutely no PS2 hardware emulation**. This is a test of native player physics using the original collision DATA; it is NOT a complete reconstruction of Ape Escape 3 gameplay.

## A game-specific obstacle now addressed

The earlier F5 walking test used arbitrary polygons from the visible GLB meshes as collision. That does not represent the game's authored physics geometry. The open-source [ae3-sdk I3D collision research](https://github.com/pxdl/ae3-sdk/blob/main/docs/formats/I3D.md) documents the actual AE3 collision sub-format I3D_I3C: an indexed vertex array plus BVH leaves that reference original triangle indices, with group names and a collision material-name array. The exact header and vertex formats are documented against 50/50 game collision files in that independent SDK.

We now have a **direct original-format native C++ reader** in `native/src/i3c_collision.cpp`. It decodes:

- Original I3D_I3C eight-byte signature, version 0x00030000, file size and offset fixup flag
- Named A groups and B subpart AABBs
- Vertex arrays as float4 at stride 0x10
- BVH 12-byte nodes, leaf triangles from the original indexed u16 triplets
- Original material names (not inferred from filenames)
- Original right-handed scene coordinates, converted to D3D11 left-handed positions
- Both original `.i3c` and `.i3d` extensions, using **contents**, not extensions, to identify collision files

The source parser bounds-checks every pointer, geometry budget, non-finite coordinate and BVH recursion limit. It converts the original triangle geometry once. **It does not execute PS2 machine code or emulate Sony physics hardware at runtime.**

The original game has the I3C BVH; the current native movement implementation consumes decoded triangles directly. This distinction matters: the player is now actually stepping on author-provided collision triangles when an original I3C is loaded, but the game-accurate collision solver is not reconstructed yet.

## Native wall collision and movement

`NativeWalkingController` now builds a wall-triangle index on reset and performs host-native horizontal capsule tests against steep walls. Movement is split into substeps to reduce tunneling at low frame rates, sliding along the wall when possible. It also supports explicit `resetAt` spawn positions. Gravity, jumping, step height and movement speeds are still experimental approximations.

This is not authentic AE3 collision-response code. In particular, moving platforms, dynamic objects, full player capsule sweeps, water, triggers, terrain material rules, sliding slopes and capture interactions require further work.

## Use the original collision in Windows

1. Update the project on your Windows PC and run `BUILD_NATIVE_WINDOWS.bat`.
2. Obtain matching model GLBs for your own Ape Escape 3 stage using Saru3VfiTool plus ae3-sdk (see `native/NATIVE_I3D_VIEWER.md`).
3. In `Getchu3Native.exe`, load the stage GLB or the level GLB folder.
4. Click **Use original I3C collision** and select a genuine `I3D_I3C` source file from **the same stage**, whether named `.i3c` or `.i3d`.
5. Press **F5** to enter the native walk/jump test. WASD moves, Space jumps, Shift sprints, and left/right arrows turn.
6. Press **F7** to show/hide original collision as a bright green wireframe over the visual model. The overlay is a sampled diagnostic view and intentionally ignores occlusion, so you can compare level geometry and physics geometry.

To inspect original collision without a graphical window:

    native\build\Release\Getchu3I3cProbe.exe "C:\AE3_PRIVATE\extracted\example_collision.i3d"

Or inventory original collision files recursively in an extracted stage directory:

    native\build\Release\Getchu3I3cProbe.exe "C:\AE3_PRIVATE\extracted\stage"

If the collision does not align with the art, the selected files may belong to different stages or require additional scene transforms; do not infer an authentic playable level from mismatched inputs.

## No commercial content needed to test the parser

The project includes a completely original-free fixture generator that writes an **authentic I3D_I3C-format** one-meter cube with eight original-format vertices, 12 indexed triangles, one original-format BVH, and an example material label. It is legal to distribute because it contains no game geometry.

    py -3 native\tools\make_i3c_fixture.py "C:\Getchu3Test\collision_cube.i3c"
    native\build\Release\Getchu3I3cProbe.exe "C:\Getchu3Test\collision_cube.i3c"

The test suite decodes this original-format fixture, drops a vertical ray onto its top face, verifies correct corner bounds and material-name decoding, and checks that malformed indices, pointers or file-size metadata are rejected. A second test confirms collision-wall blocking from both sides, sliding, jumping and explicit spawn.

## Related real animation progress

The separate MIT ae3-sdk has already solved the I3M track-to-bone mapping for its GLB exporter. Our owner-local preparation workflow now supports `-Animate` to request its **conservatively paired** original I3M clips:

    .\native\tools\PREPARE_NATIVE_ASSETS_WINDOWS.ps1 -Game "C:\AE3_PRIVATE\AE3.iso" -Saru "C:\Tools\Saru3VfiTool.exe" -Out "C:\AE3_PRIVATE\assets" -Animate

This produces GLB exports with original animation channels **where matching succeeds**. The native Direct3D renderer does not yet play skinned animation clips; baking animation data into a GLB is the necessary input step, not animation playback.

## Still required for a genuine playable Ape Escape 3 level

An authentic level's model placement, camera and original spawn coordinates, real skinned animation playback, Monkey AI, gadgets, attacks/capture, the original scene Lua bindings, real physical materials, sound, menus and save logic. The engine remains a **native scene/walking development build**, not the fully playable game.
