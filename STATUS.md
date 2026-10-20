# Native Ape Escape 3 — Actual Retail Game Status

**NOT PLAYABLE. No native Windows executable has been shown to run the complete authentic Ape Escape 3 gameplay loop.**

## Active original game path

- Source: original Ape Escape 3 USA / SCUS_975.01 only. The owner supplies its original ISO locally; source assets stay private.
- Original retail area.luc bytecode is loaded directly from the game's VFI archive and runs in a host-native Lua 5.0.3 interpreter after storage-format conversion.
- The native Windows C++20/Direct3D11 engine loads original I3D models converted to GLB, original I3D_I3C collision, original PLC placements and original I3M-derived animations.
- Genuine original PlayerProgress capture flags and owned costumes are exposed to Lua; unspecified original stage-variable and survival semantics are NOT faked for retail scripts.
- A strict native source selector requires the SAME original stage's assets and stops on missing original collision, placement or geometry. Original stage APIs are checked by Getchu3RetailStageAudit before a runtime launch.
- No PS2 CPU/IOP/VU/GS simulation, console BIOS or older ps2-web-recomp runtime is part of the active engine.

## Currently blocking real original gameplay

1. Complete original Lua engine callback implementations, especially global/area state, runtime actor creation, triggers and game progression. Each missing callback is a genuine reverse-engineering task, not a stub that should silently return zero.
2. Real retail player controller, gadget/capture system, monkey behavior and physics semantics recovered from SCUS_975.01 logic.
3. Authentic player/monkey asset identity, game animation graphs, dynamic collision, scripted cameras, original sound and stage transitions.
4. Original memory-card persistence and main menu/title/game-start logic.
5. Full end-to-end verification with the owner's actual original retail game files. The current chat files include a sample background PCK, not a full runnable retail ISO or ELF.

## Source and progress

Source: https://github.com/Ukkitan0/Getchu-3-PC

Current original-only implementation: native/src/retail_lua50.cpp, native/src/lua_stage.cpp, native/src/retail_stage_audit.cpp, native/src/app_win32.cpp, native/tools/launch_owned_retail_stage.py, native/tools/PREPARE_NATIVE_ASSETS_WINDOWS.ps1.

Do not advertise tests, mock levels, asset browsers or synthetic characters as a playable Ape Escape 3 port. CI tests remain internal and do not upload fake game-build artifacts.

There is no percentage/time-to-playable estimate that can be justified without a working authentic game loop.
