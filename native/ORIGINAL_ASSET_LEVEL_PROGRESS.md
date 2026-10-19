# Ape Escape 3 original asset and native movement integration

This phase uses the user's **Exporter.zip** as the original-format
reverse-engineering reference. Its background PCK was inspected privately
to test the original resource parsing without publishing commercial assets.

## Actual real-data investigation (owner-uploaded sample)

- Original PCK member count: **73**.
- Signatures: **25 I3D_BIN models**, **27 TIM2 textures**, **5 I3D_I3M
  animations**, **16 other data objects**.
- The provided JS geometry exporter parsed **all 25 model entries**.
  Two entries had no triangles; the other **23 models contained 18,818
  triangles** in total. Twenty-three resulting private GLBs were also
  opened successfully in an independent PC glTF scene parser.
- The five original I3M clips had **144 quaternion keyframes** across
  9 named tracks. The names include door opening and pendulum animation
  clips. Their associations with real scene bones remain to be validated.
- Only **10 of 24** referenced non-empty material names could be paired
  by exact original name with available textures from this sample.
  No guessed texture substitutions were made.
- This **bg.pck did not contain placeholder.plc**, so the placement
  of isolated objects is not verified from the sample alone.

Original commercial asset bytes were not committed to GitHub. For
personal evaluation a separate private sample package can be created
from the user's original Exporter.zip, but it is not distributed with
the public repository or native executable.

## Implemented native-only code (Windows x64)

- Native PCK archive reader: validates member table offsets and payload
  lengths; resolves original name offsets even when pointing inside a
  name or exactly to its terminating NUL; identifies I3D_BIN, I3D_I3M,
  TIM2 by magic; safely extracts files in a user-selected private
  directory. Getchu3PckProbe.exe exposes extraction/inventory.
- Native placeholder.plc parser: recovers simple fixed-size records,
  exact names and x/y/z world positions. Rejects malformed and
  out-of-range positions. Rotation/scale and variable record classes
  **are not yet recovered**.
- Native original-level scene composition: loads many user-converted
  GLBs, positions those matching PLC names, includes known background
  files, remaps material texture indices and recomputes world bounds.
  Unplaced props are skipped with counts rather than silently placed
  at the wrong location. Getchu3LevelSceneProbe.exe reports results.
- Native I3M animation decoder: reads the original named tracks,
  frame/time data and shared signed-16 quaternion pool; sorts keys and
  interpolates rotations via shortest-arc spherical interpolation.
  Getchu3I3mProbe.exe reports original clips. This DOES NOT yet attach
  original animation tracks to model bones or render animated meshes.
- Host-native walk/jump *engineering test*: samples actual GLB triangles
  to find ground height, supports WASD and sprint, simulates gravity,
  jumping and landing, and uses a simple walkable-slope filter. This is
  an early PC movement/collision controller, **NOT recovered Ape Escape 3
  player mechanics or original collision**. Character animation, gadgets
  and enemy behavior remain missing.
- Lua 5.0 script interpretation/event scheduling, original meshes,
  original textures, and this movement test share the same Win32 host
  event loop; they do not simulate PS2 CPU/IOP/GS hardware.

The current Windows CI uses synthetic assets: original PCK pointer edge
cases, original PLC world record offsets, original I3M signed quaternion
tracks, mesh-ground walking and jumping, and full multi-GLB scene
composition. It cannot inspect the owner's private game data in CI.

## Trying the actual native code on Windows

With an updated Getchu-3-PC checkout, execute:

    BUILD_NATIVE_WINDOWS.bat

Or use the compiled executables under native/build/Release.

**Original PCK resource inventory:**

    Getchu3PckProbe.exe "C:\AE3_PRIVATE\bg.pck"

**Extract the original private PCK members:**

    Getchu3PckProbe.exe "C:\AE3_PRIVATE\bg.pck" --extract "C:\AE3_PRIVATE\bg_extracted"

**Inspect an original animation:**

    Getchu3I3mProbe.exe "C:\AE3_PRIVATE\bg_extracted\0063_a_wod_c_clock_door_open.i3m"

(The exact extracted member filenames use the recovered PCK index and
name; inspect the inventory before selecting one.)

**Load a real native GLB model:**

    Getchu3Native.exe "C:\AE3_PRIVATE\glb\original_model.glb"

**Load a model folder and optional original placement data:**

    Getchu3LevelSceneProbe.exe "C:\AE3_PRIVATE\glb" "C:\AE3_PRIVATE\glb\placeholder.plc"
    Getchu3Native.exe "C:\AE3_PRIVATE\glb"

If placeholder.plc is absent, the GUI reports it and level composition
uses the original exported local geometry without pretending it has
authentic prop placement.

**Experimental native walking:** With a 3D GLB/model scene loaded,
press **F5** to enter first-person native walk/jump mode on mesh
triangles. Use **WASD**, **Space** to jump, **Shift** to sprint and
**left/right arrows** to turn. Press F5 again for free-fly debug camera.
This does not implement authentic player animation, clipping against
original wall collision, gadgets or monkey gameplay.

## Important remaining requirements

The real blocker to authentic playability is still rebuilding the
game's original C++ entity implementations, player/capture/gadget
controller, dynamic collision from I3D_I3C, skeletal animation binding,
monkey AI, area loading and original camera/audio/menus. Lua stage
scripts preserve the original gameplay orchestration, but do not
supply those native engine systems by themselves. A build that only
shows geometry or lets the debug camera walk is not yet Ape Escape 3.
