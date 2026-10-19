# True-native I3D → GLB → Direct3D11 scene viewer

This is a Windows-native C++/Direct3D11 mesh pipeline, not an emulator. Original Ape Escape 3 resources must be supplied by the owner and remain private.

## Completed code

Saru3VfiTool extracts DATA.BIN, SZ/PCK containers, and TIM2 assets offline. Separately installed MIT ae3-sdk converts original I3D_BIN models to portable glTF 2.0 GLB. The native engine reads GLBs using pinned MIT cgltf version from commit 85cd62382dfea638278962690cf515023f33ed00.

The native viewer now implements indexed triangles, multiple scene nodes and world transforms, native material and vertex colors, TEXCOORD_0 UV coordinates, mesh draw batches, front/back face rendering, depth testing, and bounding-box camera framing. The Windows Imaging Component decodes referenced PNG/JPEG files or embedded image bytes; Direct3D11 uses these as host-GPU RGBA textures. Missing textures fall back to white rather than hiding geometry.

The model viewer and Lua 5.0 stage interpreter may remain loaded together. Lua entities are still descriptors, not fully instantiated interactive actors.

## Windows owner workflow

1. Update the source in your local Getchu-3-PC checkout.
2. Run BUILD_NATIVE_WINDOWS.bat (it builds, runs native regression tests, then opens Getchu3Native.exe).
3. Run PREPARE_NATIVE_ASSETS_WINDOWS.bat for separately installed Saru3VfiTool and your owned Ape Escape 3 ISO.
4. Install MIT ae3-sdk as a separate Python development utility (https://github.com/pxdl/ae3-sdk), then generate original GLB files:

    py -3 native\tools\convert_models_to_glb.py --extracted "C:\AE3_PRIVATE_ASSETS\extracted" --out "C:\AE3_PRIVATE_ASSETS\glb" --ae3 ae3

5. Click "Open native 3D model (GLB)" in Getchu3Native.exe and select a converted model.
6. WASD flies, Q/E changes height, left/right arrows turn the camera, and Shift moves faster. Controls work while the model list or viewport child window is focused.
7. Optionally click "Run original Lua 5.0 stage" to run privately recovered source in the same window.

Or launch a GLB directly from PowerShell:

    native\build\Release\Getchu3Native.exe "C:\AE3_PRIVATE_ASSETS\glb\example.glb"

The original game data and GLBs should remain under a private directory that is excluded from Git. The ae3-sdk exporter writes relative image URIs: retain the relative directory structure between its GLBs and PNGs.

## Game-free renderer test included

The generator native/tools/make_scene_fixture.py writes an independent textured GLB with two translated nodes, four indexed triangles, a native PNG image and UV/material bindings. This is a test fixture, NOT a real Ape Escape 3 level.

Generate and inspect it with these Windows commands:

    py -3 native\tools\make_scene_fixture.py "C:\Getchu3\native_arena_test.glb"
    native\build\Release\Getchu3GlbSceneProbe.exe "C:\Getchu3\native_arena_test.glb"
    native\build\Release\Getchu3Native.exe "C:\Getchu3\native_arena_test.glb"

The Windows CI also generates, parses, and uploads this sample alongside native executables, and runs eight native C++ regression tests.

## Still missing before an authentic playable build

The native viewport currently shows STATIC rest-pose geometry, not fully animated original levels. Original skin-animation playback, I3D_I3C collision, real player controller, monkey AI, gadgets, the mapping between Lua entities and model/collision instances, scene streaming, audio, menus, and save behavior remain incomplete.

Material rendering currently uses base-color PNG/JPEG textures with simple directional shading; exact original PS2-era shader/material fidelity is unverified. Real retail assets have not been visually tested in this environment. Windows CI proves successful compilation and synthetic geometry/resource processing, not authentic retail gameplay.

This is real native rendering progress but must not be described as a completed, playable native Ape Escape 3 port.
