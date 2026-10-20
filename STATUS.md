# Native Ape Escape 3 — Actual Retail Game Status

**NOT PLAYABLE. No native Windows executable has been shown to run the complete authentic Ape Escape 3 gameplay loop.**

## Active original game path

- Source: original Ape Escape 3 USA / SCUS_975.01 only. The owner supplies its original ISO locally; source assets stay private.
- Original retail area.luc bytecode is loaded directly from the game's VFI archive and runs in a host-native Lua 5.0.3 interpreter after storage-format conversion.
- The native Windows C++20/Direct3D11 engine loads original I3D models converted to GLB, original I3D_I3C collision, original PLC placements and original I3M-derived animations.
- Original stage PCK members now feed the actual game path: owner-local I3D models, I3M animations and TIM2 textures are extracted before GLB conversion, and PCK-contained I3D_I3C collision is decoded directly by the C++ stage collision loader without duplicating private bytes on disk. Lua model_name binding recognizes unique archive-index-prefixed GLB assets.
- Genuine original PlayerProgress capture flags and owned costumes are exposed to Lua; unspecified original stage-variable and survival semantics are NOT faked for retail scripts.
- A strict native source selector requires the SAME original stage's assets and stops on missing original collision, placement or geometry. Original stage APIs are checked by Getchu3RetailStageAudit before a runtime launch.
- No PS2 CPU/IOP/VU/GS simulation, console BIOS or older ps2-web-recomp runtime is part of the active engine.

## Verified original owner-data evidence (not a playable build)

**Full original owner-uploaded ISO recovered on 2026-10-20.** The seven
uploaded parts were found in the user's ChatGPT file library, reconstructed
in their original order and verified as a 2,174,091,264-byte ISO9660 image
(SHA-256: `5b4c6272c9daf190b3f72081f124edd7ea2af3cc0dafeaef206ded97a022fd09`).
The reconstructed disc holds **3,994 original VFI members**, including
**1,706 SZ-compressed members, 958 original PCK archives, 168 PLC placements,
168 original action sequences and all 168 authentic `area.luc` scripts**.
Every one of the 3,994 VFI members was extracted into private runtime files
with no extraction failures. The commercial bytes are **not in GitHub**.

The actual original retail executable `SCUS_975.01` (6,488,148 bytes)
matches pinned SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
All **168 Lua 5.0 chunks parsed**, revealing 805 Lua functions and 135,820
instructions. The pinned original ELF's registration helper at `0x255BD8`
was independently scanned, recovering **17 exact registration sites** for
callbacks including `createTutorialExit`, `createFeatureLauncher`,
`createWarpGateLauncher`, `LocaleInfo_getID`, `stage_var_get_int`,
and original game/area flags. See
`native/reference/verified_original_retail_api_sites.json` for actual
call addresses, string addresses, and Lua use-site counts; these facts are
now sufficient for code-grounded work on missing Lua game callbacks.

The complete ISO and extracted retail data were assembled privately in
the analysis session and may not persist in future sessions. The public
source repository retains only metadata, decoder code, and tests.
The actual native gameplay loop **is still not implemented**, and no
Windows end-to-end gameplay run has been validated.

The user's private Woods C background PCK contains 73 original asset members: 25 I3D_BIN models, 27 TIM2 textures, 5 I3D_I3M animations, 9 I3D_I3C collisions and 7 other members. Independent offline validation of the nine actual original I3C BVH/vertex structures accounts for 1,904 collision triangles. No original asset bytes are checked into GitHub.

Native code and launch-path fixes have been committed, with internal regression tests for PCK members and unique Lua model names. The latest changes have NOT been validated by a Windows compiler or end-to-end retail game run, and no fake gameplay build is offered.

## Currently blocking real original gameplay

1. Complete original Lua engine callback implementations, especially global/area state, runtime actor creation, triggers and game progression. Each missing callback is a genuine reverse-engineering task, not a stub that should silently return zero.
2. Real retail player controller, gadget/capture system, monkey behavior and physics semantics recovered from SCUS_975.01 logic.
3. Authentic player/monkey asset identity, game animation graphs, dynamic collision, scripted cameras, original sound and stage transitions.
4. Original memory-card persistence and main menu/title/game-start logic.
5. Full end-to-end verification with the now-recovered original ISO and ELF. Original data have been extracted and audited, but Windows full-game execution remains unverified.

## Source and progress

Source: https://github.com/Ukkitan0/Getchu-3-PC

Current original-only implementation: native/src/retail_lua50.cpp, native/src/lua_stage.cpp, native/src/retail_stage_audit.cpp, native/src/app_win32.cpp, native/tools/launch_owned_retail_stage.py, native/tools/PREPARE_NATIVE_ASSETS_WINDOWS.ps1.

Do not advertise tests, mock levels, asset browsers or synthetic characters as a playable Ape Escape 3 port. CI tests remain internal and do not upload fake game-build artifacts.

There is no percentage/time-to-playable estimate that can be justified without a working authentic game loop.
