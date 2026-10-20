# Getchu-3-PC — native Ape Escape 3 for Windows

**Status: NOT PLAYABLE.** There is no verified Windows build that runs the complete retail game or a playable original stage.

## Goal and approach

Recreate **Ape Escape 3 USA (SCUS_975.01)** as a normal, moddable Windows game, using C++20/Direct3D11 and the game's **original Lua 5.0 scripts running as Lua**. No PlayStation CPU/IOP/VU/GS emulation and **no static recompilation of the retail ELF** are part of the active engine.

The original game files are not distributed with this repository. Provide your own legally obtained USA ISO.

## Owner-friendly steps

1. Double-click `RECOVER_ORIGINAL_GAME_WINDOWS.bat` to extract the original ISO, Lua scripts and executable into the git-ignored `native/private/` workspace. Python 3 is required; the launcher selects the ISO or uses an existing default path.
2. Double-click `BUILD_NATIVE_WINDOWS.bat` to build the **development engine** and run internal tests using Visual Studio 2022 with the C++ Desktop workload and CMake. This opens a native Windows development window, **not the playable game**.
3. If testing a retail stage, use `PREPARE_NATIVE_ASSETS_WINDOWS.bat`. This is a separate developer investigation path requiring the external asset tools described in `native/tools/PREPARE_NATIVE_ASSETS_WINDOWS.ps1`; it does not promise authentic gameplay.

The data-recovery workflow produces a local manifest, Lua engine API audit, original ELF registration report where supported, and a private Lua+ELF ZIP. All commercial data remains outside Git; do not commit or publish the output.

## Current technical foundation

- Original ISO9660/VFI, SZ and PCK handling, plus offline extraction tools.
- Original Lua 5.0 bytecode execution in a host-native Lua 5.0.3 interpreter; game-specific callbacks still need reconstruction.
- Native D3D11 window, glTF scene loading, partial I3D/I3M asset conversion, PLC placements and I3C collision decoding.
- Partial native entity instantiation, native walk/collision **test** code, and synthetic unit/regression fixtures.

In the prior private investigation, all seven owner-uploaded ISO parts were recovered and 3,994 VFI entries and 168 original `area.luc` scripts were inspected. This is **offline extraction evidence**, not evidence of a playable Windows game. The actual retail executable and source files are not committed; the relevant code-address metadata is at `native/reference/verified_original_retail_api_sites.json`.

## What is still missing

A correctly rendered *real* retail level on Windows; complete Lua engine callback semantics; real player control, gadgets and capture; monkey AI; dynamic physics, animation and cameras; audio, menus, transitions and saves. See `STATUS.md` for milestones and `native/docs/LUA_BINDING_TODO.md` for confirmed Lua binding leads.

## Repository layout

- `native/` — active native C++ engine, Lua integration, tests, asset tools and reference metadata.
- `archive/recomp-attempt/` — **preserved, inactive** historical PS2 recompilation experiments.
- `.github/workflows/native-windows.yml` — manual-only Windows CI; tests use fixtures, not retail game data.

No commercial ISO, ELF, original `.luc`, models or textures belong in Git. The native port is an original implementation using owner-supplied game assets.
