# Getchu-3-PC — Ape Escape 3 genuine native Windows port

**Current state: NOT PLAYABLE. No authentic Ape Escape 3 gameplay build has been verified.**

The goal is the complete original retail Ape Escape 3 USA (SCUS_975.01) rebuilt as a normal Windows PC program. An asset viewer, synthetic scene, test monkey or simulated PlayStation hardware is NOT a finished game and will not be packaged or advertised as one.

## Architecture

Original Lua 5.0 stage bytecode is read directly from the user's retail ISO. The bytecode's 32-bit numeric/string representation is converted for a real host-native Lua 5.0.3 interpreter; no PS2 instructions or devices execute. The native C++20 / Direct3D11 engine uses user-extracted game meshes, collision, textures, animation and game script data.

Actual player progress fields from the original SCUS_975.01 Data layout now back Lua capture and costume checks. An unsaved new-game baseline is explicit; authentic memory-card save reading is **not yet implemented**. Unknown original game engine functions fail visibly instead of inventing values.

## Run the original-game resource path (developer investigation only)

This is **not a playable release**. It is for identifying which authentic game systems fail.

Requirements: owned Ape Escape 3 USA ISO, separately installed Saru3VfiTool (GPLv3), separately installed ae3-sdk with 'ae3' CLI (MIT), Windows Visual Studio 2022 C++, CMake and Python 3.

The real-stage preparation and launch command, in PowerShell from the repository root:

    .\native\tools\PREPARE_NATIVE_ASSETS_WINDOWS.ps1 -Game "C:\AE3\Ape Escape 3 (USA).iso" -Saru "C:\Tools\Saru3VfiTool.exe" -Out "C:\AE3\owned_native_assets" -Reuse -LaunchOriginalStage seaside_a

The stage name is an example: it must exist in your actual original game. The process recovers original models, animations, collision and placements, checks the real stage's Lua bindings, then invokes the real native-game workbench. If an original stage resource or game binding is missing, it **stops with an error**. No synthetic actors or levels are substituted. All extracted commercial game data stays private.

Alternatively the existing compiled native executable can open authentic retail data directly:

    Getchu3Native.exe --retail "<your-original-ISO>" <original-stage-id> "<stage-GLB-folder>" "<original-placeholder.plc>" "<same-stage-collision-directory>" [original-spawn-tag]

This uses the actual script in the original disc archive under debug/us/stage/<stage-id>/area.luc. Offline decompilation to Lua source is optional for further reverse engineering, NOT a requirement for executing the original bytecode.

## Implemented components

Native ISO/VFI/PCK archive handling, original Lua 5.0 bytecode conversion and source execution, partial game binding vocabulary, recovered player-save Data layout, I3D-to-GLB renderer, original I3C collision-data decoder, I3M-derived skeletal animation, native actor/world geometry assembly, owner-local stage asset verification.

## Missing before actual gameplay

An authentic retail title/menu boot; game-accurate player controller, gadgets, capture net and monkey AI; complete Lua binding semantics and actor lifecycle; original animation state graphs, dynamic collision, camera, sound, save loading/persistence, proper stage progression and scene transitions. Retail level correctness on Windows is NOT established.

Internal tests are kept to prevent code regressions, but no synthetic tests are published as user-facing game builds. All PlayStation CPU/IOP/VU/GS emulation is forbidden in the active Windows engine.

## Research

- Original game logic and player progress: https://www.cnblogs.com/algonote/p/15678057.html
- Formats and animations: https://github.com/pxdl/ae3-sdk
- Genuine game asset extraction: https://github.com/mthsk/Saru3VfiTool

Keep your original game ISO, ELF and derived assets out of public Git.
