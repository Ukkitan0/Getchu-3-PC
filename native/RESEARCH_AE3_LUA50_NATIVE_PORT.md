# Ape Escape 3: native Lua-stage and player-state reconstruction

**Goal:** true Windows C++/Direct3D11 port, **no PS2 hardware emulation**.
Source: [Byaidu, PlayStation 2 Game Reverse Engineering: Ape Escape 3 (2021)](https://www.cnblogs.com/algonote/p/15678057.html).
See also [pxdl/ae3-sdk](https://github.com/pxdl/ae3-sdk),
[Saru3VfiTool](https://github.com/mthsk/Saru3VfiTool), and
[OpenGOAL](https://github.com/open-goal/jak-project) for native-port architecture.

## Major finding: original levels use Lua 5.0 bytecode

Ape Escape 3 area folders contain area.luc, apparently Lua 5.0 bytecode.
The article documents stages spawning players, monkeys, cars, cutscene
launchers and breakables; controllers sequence flags, sound, waits,
velocities, entity messages, collision and camera changes.

This is valuable recoverable gameplay DESIGN, not a ready game. Lua code
describes scene entities and actions. Native C++ still has to implement
the entity types, player input, gadget logic, monkey AI, collisions,
animation, cameras, graphics, audio, loading and persistence.

**Running original Lua scripts via a native game scripting engine is not
hardware emulation.** It does not interpret or simulate PS2 CPU, IOP,
GS, DMA, GPU or firmware. The finished port must retain Win32/Direct3D11
and host filesystem support without a console runtime.

Original Lua 5.0 chunks use Lua bytecode version 0x50. Modern Lua VM
releases do not normally accept old bytecode without conversion, so
prefer offline source recovery with a Lua 5.0-capable unluac version.
A Lua decompiler may fail with stripped debug information or modified
bytecode. Never silently treat different versions as compatible.

## Research addresses from the US executable

These are article-derived leads, not verified symbols from a locally
supplied retail executable. Reverify before making universal claims.

| US address | Function or data | Native target |
|---|---|---|
| 0x00531F18 | McAccess save data decode | Native save system |
| 0x0036CC44 | zlib decompress routine | Native data decoder |
| 0x00270144 | Player data decode | PlayerProgress state |
| 0x00649910 | Original player Data structure | Native owned game state object |
| 0x0033DB70 | Damage/HP reduction | Native combat system |
| 0x0021A93C | SetupBindLauncher | Native Lua entity registrations |
| 0x003854C8 | SetupBindFlag | Native Lua flags and stage state |

Article-derived player layout:
player_life at offset 8; player_energy at offset 20;
capture_flag at offset 204; shop_item at offset 774.
Native C++ representation is in
native/include/ae3/player_state.hpp.
The recovered ChangeHP routine clamps current HP + delta to 0–100;
native implementation and tests are in native/src/player_state.cpp
and native/tests/player_state_test.cpp.

## Stage scripting binding targets

Documented original names include:
setupArea, new_ent, lua_param, c_order, c_ctrl, get_capture_flag,
createFeatureLauncher, createWarpGateLauncher, createTutorialExit,
createMGSGenerator, gw_set_flag, gw_get_flag, lw_set_flag, lw_get_flag,
area_set_flag, area_get_flag, stage_var_get_int, is_survival_mode,
get_costume_possess, Player, Monkey, Controller, Breakable, Car,
DemoLauncher, ChangeArea, Collidable and Button.

The function names guide reconstruction but do not supply the
corresponding C++ engine implementations.

## Code added to true-native Windows source tree

- native/src/stage_scripts.cpp: native Lua 5.0 area.luc identification.
- native/src/stage_probe.cpp: standalone Getchu3StageProbe.exe scans
  a local ISO, VFI archive or fully extracted game asset folder.
- native/tools/recover_lua50.py: **offline only**, optional batch
  decompilation of locally extracted area.luc to locally stored area.lua,
  using a user-provided unluac JAR and Java installed only for development.
  Java, unluac and PS2 hardware emulation are NOT native-game dependencies.
- native/src/player_state.cpp: independently recovered native HP rule.

This is early native port infrastructure and a small recovered gameplay
fragment; **no playable real game or rendered retail level is verified**.

## How to recover your own Lua 5.0 level scripts

1. Extract your owned Ape Escape 3 DATA.BIN with Saru3VfiTool
   (including nested SZ/PCK containers). Keep extracted data private.
2. Build the native engine using BUILD_NATIVE_WINDOWS.bat.
3. Scan extracted assets locally:

    Getchu3StageProbe.exe "C:\path\to\extracted"

4. Supply a trusted Lua 5.0-capable unluac.jar; execute:

    py -3 native/tools/recover_lua50.py --extracted "C:\path\to\extracted" --out "C:\AE3_PRIVATE_LUA" --unluac "C:\tools\unluac.jar"

5. Read the local lua_recovery_manifest.json. Scripts with incomplete
   debug symbols, modified bytecode or failures need manual work.
6. Never commit original or recovered copyright-protected game assets
   and scripts to a public repository.

## Concrete remaining milestones

Recover complete stage scripts, build native entity registration and
scene data model, import actual I3D models and animations, implement
player/monkey/controller mechanics, camera, collision, HUD and saves,
then validate boot and gameplay with a locally owned retail disc.
Original Lua scripts alone cannot make a native port playable.
