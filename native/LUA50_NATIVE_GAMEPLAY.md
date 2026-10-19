# Native Ape Escape 3 Lua 5.0 engine (Windows)

**Current result:** original-style Lua 5.0 scripts run on the host processor
inside our native Windows C++ game workbench. They can register original-style
entity descriptions and controller timelines. This is NOT a playable port yet.

## Native language and dependency choice

- The official Lua 5.0.3 source is downloaded from https://www.lua.org/ftp/
  and SHA256-verified against the publisher's checksum by CMake.
- Lua 5.0 source is interpreted natively as a game scripting language on
  Windows. No PlayStation CPU, GPU, GS, IOP, DMA, firmware, PS2 virtual
  machine, PCSX2, or ps2-web-recomp system is compiled into the game.
- The native C API is limited to base, math, string and table libraries,
  with system file access and untrusted module loading disabled.
- Only recovered Lua SOURCE files are accepted. Original PS2 Lua bytecode
  (.luc) is rejected because host bytecode architecture compatibility has
  not been established. Use the offline private recovery tool first.
- The project retains the official MIT notice in
  native/third_party/LUA_5_0_3_LICENSE.txt.

## Current original-style Lua bindings

LuaStage registers the following host-native gameplay-facing globals:

    new_ent, lua_param, c_ctrl, c_order
    gw_set_flag, gw_get_flag, lw_set_flag, lw_get_flag
    area_set_flag, area_get_flag, get_capture_flag
    stage_var_get_int, is_survival_mode, get_costume_possess

Real script flow is executed by Lua 5.0, with the exposed setupArea(opts)
function invoked using opts.spawn_tag. Original Lua tables are recursively
converted into strongly typed native C++ ScriptValue trees.

CAUTION: stage_var_get_int, is_survival_mode and costume possession
currently return defaults; this is **not complete game compatibility**.
Their original data semantics and many additional bindings remain to
be reconstructed.

The resulting NativeStageWorld stores original entity names, types, tags,
fields, stage flags, and nested controller sequences, but does not yet
instantiate real animated physical characters.

## Native controller execution

NativeControllerTimeline evaluates the original Lua c_order/c_ctrl
structure with a host-game-time counter, not a PS2 system clock.
Implemented: nested finite repeats, -1 infinite repeats, timed waits,
trigger waits that pause until a matching host signal, and dispatch
of typed events (se, value, message_b, etc.). Script commands currently
emit logged event records; they do NOT yet perform physical movement,
real game audio, original entity spawning during updates or collision.

## Running the native engine

On Windows with Visual Studio 2022 and CMake, double-click
BUILD_NATIVE_WINDOWS.bat. The native program is:

    native/build/Release/Getchu3Native.exe

It has a "Run original Lua 5.0 stage" button: select a locally
recovered area.lua. It executes setupArea and displays the native
entity registry and scripted controller events. The "Signal Lua trigger"
button allows event-trigger testing. This interface is an engine
development workbench, NOT the full game or an authentic world.

No original game files are needed for the independently authored test
script shipped with this repository:

    native/build/Release/Getchu3LuaStageRunner.exe native/examples/native_binding_demo.lua entry

It must report four native entity descriptors (Player, Monkey,
Breakable, Controller).

If you own Ape Escape 3, use PREPARE_NATIVE_ASSETS_WINDOWS.bat with the
optional Saru3VfiTool extractor, then native/tools/recover_lua50.py and
a user-provided Lua-5.0-compatible unluac JAR. Keep extracted Sony data
and recovered scripts local. They must not be uploaded to the public repo.

## Verified game-free tests

CMake CTest compiles and runs a real Lua 5.0 interpreter, evaluates
setupArea control flow and Lua params, exercises deeply nested controller
trees, tests numeric/boolean table values and flags, verifies native
host-time wait/repeat semantics, and rejects loading raw PS2 bytecode
without validated conversion.

## Required remaining work

Full AE3 stage binding semantics; real level and animation rendering,
asset/material pairing; entity lifecycle, player movement and gadgets,
monkey AI, physics and collision, stage cameras and transitions,
real controller event handlers, sounds, original menus, real saves.
The currently executed script is not enough to play an authentic level.

Original stage script research:
https://www.cnblogs.com/algonote/p/15678057.html
