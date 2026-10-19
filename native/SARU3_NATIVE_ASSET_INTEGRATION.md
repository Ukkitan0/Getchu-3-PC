# Saru3VfiTool integration — true native Ape Escape 3 pipeline

**Purpose:** derive original game resources for the *native* Windows C++/Direct3D11 port. [Saru3VfiTool](https://github.com/mthsk/Saru3VfiTool) is an **optional external offline development tool**, not part of the game EXE. This workflow never executes or simulates the PlayStation 2 EE, IOP, GS, or firmware.

## Source-verified capabilities

Saru3VfiTool's current README and ExtractCommand cover VFI `DATA.BIN`, SZ decompression, nested PCK extraction, TIM2-to-PNG plus mipmaps and JSON sidecars, EXDB to editable JSON, typed text BIN, and SBT subtitle cues. It also supports rebuilding extracted containers from manifests, though rebuilding is not part of our native engine workflow. This makes it much more useful than writing custom decompression/container tools from scratch.

**Important limitations:** Texture conversion is labeled experimental upstream; not all TIM2 layouts produce correct output. I3D meshes/animations are preserved as original assets but **not converted into directly renderable PC geometry by Saru alone**. For those, use I3D research and format converters from [pxdl/ae3-sdk](https://github.com/pxdl/ae3-sdk), validated with original assets. Some Lua scripts may remain in nested containers or use renamed entries. No real retail game files were available to CI, so their extraction coverage is unverified.

Saru is **GPL-3.0**; our native app does not copy/link/ship its source or binaries. Users install it separately, and the bridge invokes it only when preparing their own game files. Observe Saru's license obligations if redistributing the tool itself.

## One-click owner-local workflow

On Windows 10/11 with Visual Studio 2022 C++/CMake, Python 3, and a downloaded Saru3VfiTool Windows release or locally built executable:

1. Run **`PREPARE_NATIVE_ASSETS_WINDOWS.bat`** from the root of Getchu-3-PC.
2. Select a legitimately owned Ape Escape 3 ISO or an extracted `DATA.BIN` file.
3. Select the separately obtained `Saru3VfiTool.exe` or `.dll` (.NET 10 may be needed).
4. The script builds only the host-native asset tools. An ISO is opened with the C++ ISO9660 parser; it streams the byte-exact `DATA.BIN` into a private output folder.
5. Saru processes that `DATA.BIN` using `extract --tim2` (recursive SZ/PCK and PNG conversion).
6. Getchu3StageProbe scans extracted assets and records Lua 5.0 area-script candidates.

The local output is **`native/private/owned_assets`**, excluded from Git:

- `DATA.BIN` — ISO-derived original game data, or original external file if already supplied
- `extracted/` — Saru output and manifests
- `native_asset_inventory.json` — extension counts and selected relative filenames
- `native_stage_script_inventory.txt` — Lua 5.0 stage discovery

Original game files are never overwritten. Re-running an existing extraction is an explicit `-Reuse` option; the default refuses to overwrite existing private data.

Command-line alternative (the paths below are examples):

```powershell
.\native\tools\PREPARE_NATIVE_ASSETS_WINDOWS.ps1 -Game "D:\OwnedAE3.iso" -Saru "C:\Tools\Saru3VfiTool.exe" -Out "C:\AE3_PRIVATE_ASSETS"
```

Or invoke the standalone Python converter bridge after building the host-native tools:

```powershell
py -3 native\tools\prepare_owned_assets.py --game "D:\OwnedAE3.iso" --out "C:\AE3_PRIVATE_ASSETS" --saru "C:\Tools\Saru3VfiTool.exe" --probe "native\build\Release\Getchu3AssetProbe.exe"
```

**No Saru installation is required to launch `Getchu3Native.exe`.** Saru is used only to generate game-development assets locally; Java, unluac and Saru are not linked into the native game runtime.

## Optional direct I3D-to-GLB conversion (MIT ae3-sdk)

Saru extracts I3D models but does not itself convert those meshes and
animations into PC geometry. We also verified the independent
[pxdl/ae3-sdk](https://github.com/pxdl/ae3-sdk) **MIT-licensed** `ae3 gltf`
converter. Its documented original-game coverage is 154/154 I3D_BIN models
with native glTF meshes, skeletons, skin weights and materials. Keep
this converter offline, outside the Windows EXE; the resulting GLB files
are regular portable 3D assets for a PC renderer.

Install ae3-sdk's Python tools in your development environment using its
[tools README](https://github.com/pxdl/ae3-sdk/blob/main/tools/README.md),
then request glTF export while running the one-click prep launcher:

```powershell
.\native\tools\PREPARE_NATIVE_ASSETS_WINDOWS.ps1 -Game "D:\OwnedAE3.iso" -Saru "C:\Tools\Saru3VfiTool.exe" -Out "C:\AE3_PRIVATE_ASSETS" -Gltf -Ae3Cli ae3
```

Or reuse an already extracted Saru asset folder:

```powershell
py -3 native\tools\convert_models_to_glb.py --extracted "C:\AE3_PRIVATE_ASSETS\extracted" --out "C:\AE3_PRIVATE_ASSETS\glb" --ae3 ae3
```

The converter checks the actual I3D_BIN file signature to avoid passing
collision I3D_I3C or animation I3D_I3M files to the mesh converter. It
preserves relative folder structure, refuses to overwrite GLBs, emits
a conversion manifest and counts conversion failures rather than
pretending unsupported formats were exported. Conversion may still have
texture mapping/animation-pairing limitations; those need owner-local
visual validation. **The current Getchu3Native.exe cannot yet import
GLBs or render actual retail 3D levels**; this is the asset pipeline
feeding that next engine milestone.

## Verified changes

- `Archive::extractDataBin` streams original `DATA.BIN` bytes out of an ISO without loading the entire disc. Overwrite is refused.
- `Getchu3AssetProbe --copy-databin <output>` performs a native host-file handoff to Saru.
- `native/tools/prepare_owned_assets.py` invokes a separate GPL utility and inventories resulting textures, models and Lua files.
- Synthetic Windows CI validates ISO-to-DATA.BIN byte identity, overwrite protection, external command arguments, independent converter support and reusable extraction.
- `native/tools/PREPARE_NATIVE_ASSETS_WINDOWS.ps1` builds the host-native tools and invokes extraction with a file picker.

## Next work toward gameplay

The **next blocker** is rendering real I3D meshes and animations from the extracted files in native Direct3D11, and connecting scene entities to the recovered Lua 5.0 stage scripts. Asset conversion alone does not implement Monkey AI, Sora-equivalent player (Satoru/Sayaka), gadget mechanics, collision, native scene rendering or menu logic. A playable Ape Escape 3 build remains unverified.

