# First genuinely animated native Ape Escape 3 I3M skeleton — Windows build

The owner-local data path is now:

**Original I3D_BIN + I3D_I3M + TIM2** → MIT `ae3-sdk` offline glTF/GLB export → **C++20 native glTF runtime** (`native/src/native_skinning.cpp`) → **host-CPU skin transforms** → Windows Direct3D11 dynamic vertex-buffer rendering.

This is NOT a PlayStation 2 emulator, a VU/GS renderer, a MIPS interpreter, or an old ps2-web-recomp runtime. The original game animation data drives a brand-new host-native skeletal animation player.

## Implemented runtime features

- Reads glTF 2.0 skin joint lists and original model mesh JOINTS_0/WEIGHTS_0 vertex influence attributes.
- Loads inverse bind matrices and reconstructs animated parent/child joint world transforms.
- Evaluates skeletal rotation keyframes using shortest-arc quaternion spherical interpolation (SLERP); translation and scale are interpolated per channel.
- Supports original named glTF animation clips, multiple clips, looped animation timelines, LINEAR and STEP interpolation.
- CPU skinning uses the standard `jointWorld * inverseBind * vertex` matrix formula and four native weighted bone influences per vertex; results are converted to Direct3D's left-handed world coordinates.
- Applies the new skinning positions to a **D3D11_USAGE_DYNAMIC host GPU vertex buffer** every Windows frame. Original mesh topology, UVs, material batches and image textures stay intact.
- Sane resource limits and explicit errors for malformed/out-of-bounds skin indices, nonfinite keyframes, cyclic skeletons and unsupported CUBICSPLINE.
- A native CLI `Getchu3SkinningProbe.exe` opens locally converted original game models and prints named clip duration, skeleton joint counts, animated vertex counts and sampled bounds.

## Try it on Windows

1. Update `Getchu-3-PC` to the current `main` branch.
2. Run `BUILD_NATIVE_WINDOWS.bat` to compile and start `Getchu3Native.exe`.
3. Prepare your own Ape Escape 3 game files and install the separate MIT `ae3-sdk` offline tools.
4. Enable original I3M clip export using `-Animate`:

       .\native\tools\PREPARE_NATIVE_ASSETS_WINDOWS.ps1 -Game "C:\AE3_PRIVATE\ApeEscape3.iso" -Saru "C:\Tools\Saru3VfiTool.exe" -Out "C:\AE3_PRIVATE\assets" -Animate

   This flag regenerates GLBs that were previously exported as static models, so enabling animations after an earlier conversion works. It ONLY rewrites the *derived GLB exports*, not the owner's ISO or original source assets.

5. In the native app click **Open native 3D model (GLB)** and choose a converted character model containing original I3M animation clips. Loading a model that has NO valid animation channels still opens the static asset browser; it does not fabricate animations.

### Controls

- **F8** — move to the next named clip (if present)
- **F9** — pause/resume native skeletal animation
- **WASD, Q/E, arrow keys** — explore the model as before
- **F5** — experimental native player walking
- **F7** — toggle original I3C collision wireframe (if loaded)

## Game-free animated test included

The Windows release package includes `native_bone_animation_test.glb`: an original-free glTF fixture with one skeleton joint, a triangle skinned through JOINTS_0/WEIGHTS_0, inverse bind matrix and named rotation/translation animation. It is generated programmatically from scratch, and includes no Sony geometry or proprietary code.

To view genuine **animated triangles** without owning any game assets, open this fixture in `Getchu3Native.exe`. It is intentionally minimalist: one triangle rotates and moves instead of showing a complete character. To verify mathematical skinning without Direct3D:

       Getchu3SkinningProbe.exe "native_bone_animation_test.glb"

The CI runs 15 native C++ integration tests, including original-format I3C collision, walking, Lua 5.0 runtime and controller timelines, and the glTF skeleton keyframe test. It additionally samples the bone fixture through the runtime CLI and publishes the game-free fixture alongside the engine.

## Accuracy/remaining scope

**The native pipeline now SUPPORTS skinned animation**, but authentic Ape Escape 3 character playback has not yet been visually validated with the user's full privately extracted game character data.

Still missing: correct native actor-to-model binding from `area.lua`, multiple simultaneously animated monkey/player actors in one level, live collision on dynamic props, stage progression, player capture/gadgets, enemy AI, game-accurate camera, animation state graphs/transitions, audio, menus and saves.

The current native walking camera is an experiment, not the game-accurate main character controller. This is a substantial native animation development build, **not yet a fully playable Ape Escape 3 game**.

Research reference: https://github.com/pxdl/ae3-sdk/blob/main/docs/formats/I3D.md
