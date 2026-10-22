# Authentic Lua 5.0 inside Unreal: source integration and gaps

## Native and Unreal components

The importer branch includes a real Unreal C++ game and Editor project, plus a
runtime GameInstanceSubsystem. The original Lua 5.0.3 VM, original-bytecode
representation retargeter, and existing LuaStage from the native source tree
are compiled into a standalone Windows Getchu3OriginalLuaBridge DLL. The DLL
does NOT execute Sony MIPS/PS2 hardware or replace original Lua gameplay.

- native/include/ae3/ae3_unreal_lua_bridge.h : version 2 C callback ABI.
- native/src/ae3_unreal_lua_bridge.cpp : native Lua 5.0 execution and forwarding.
- native/tests/ae3_unreal_lua_bridge_test.cpp : fail-closed ABI tests.
- native/CMakeLists.txt : Getchu3OriginalLuaBridge target and test.
- unreal/Plugins/Getchu3Importer/Source/Getchu3OriginalLua : packaged
  game runtime, Unreal callback contract and GameInstanceSubsystem.
- unreal/BuildAndOpen.bat : Windows local dependency/build/test/Editor launcher.

No retail ISO, original executable, game Lua, textures or audio are shipped.

## Unmodified original script logic

The Unreal subsystem's StartOriginalStage takes a private original area.luc
path, original spawn tag and a UGetchu3OriginalManagerProvider. It verifies the
Lua 5.0 signature and passes raw file bytes into the recovered native
retargeter. The retargeter changes only original binary storage representation
so host Lua 5.0.3 executes original instruction words unchanged. On success it
calls the original script-defined setupArea function.

Runtime manager callbacks are synchronous on the Unreal game thread and must
NOT re-enter the native Lua bridge. Every native manager operation returns true
ONLY after completing the genuine original game behavior.

- new_ent requires a recovered original actor/entity manager.
- lua_param requires the recovered original stage parameter manager.
- game flags require original global/local/area flag owners.
- stage integers and survival state require original recovered owners.
- weapon selection requires the real original player/gadget owner.
- original PlayerProgress is accepted only as exactly 1033 packed bytes
  supplied from owner game/save state; no synthesized save is permitted.

The default Unreal native manager provider rejects every operation instead
of spawning generic Unreal actors or inventing flags. Without real managers,
original game stages must fail and the Play button remains disabled.

## Lossless original Lua transport

The C ABI version 2 has no C++ STL objects across DLL boundaries. Entity
class/name/tag and flag scope/key are exact byte-hex strings. Lua values are
represented in typed JSON with explicit type tags (nil, boolean, number, text,
array, object), byte-hex string contents and byte-hex object keys. This avoids
JSON Unicode rewriting, embedded NUL truncation or accidental type coercion.
Original Lua table/closure identities that cannot cross the typed path are
rejected by the pre-existing native LuaStage, not silently rewritten.

Native source and Unreal source reference the same ABI header: there is no
parallel hand-maintained layout. Bridge ABI and original progress data both
have explicit version or length gates.

## One-click Windows source build

With Unreal Engine 5.6, Visual Studio 2022 C++ tools, CMake, Python 3 and
network access, run unreal/BuildAndOpen.bat. This installs pinned MIT
ae3-sdk Python converter tooling to an ignored local environment, compiles
the authentic Lua 5.0 native bridge and its tests, stages the resulting DLL
under unreal/Binaries/Win64, and builds the Unreal Getchu3Editor target. The
original ISO is selected separately through the already implemented Editor
importer panel. Everything remains local; no game content is redistributed.

Build and test operations deliberately fail on missing real dependencies
and missing original game managers.

## Unverified / still required

The bridge, CMake test target and Unreal C++ plugin are committed but have
NOT yet been compiled or run on Unreal Engine 5.6 Windows. Genuine private
USA ISO-to-Unreal imports, source-backed area.luc execution, retail behavior
comparison, original actor factories and audio/gameplay systems remain
unverified. This is not a game release and not a playable executable.

Original SCUS_975.01 actor manager, stage parameter manager, update dispatcher,
monkey AI, player movement, capture systems, animation transitions, stage
scheduler and source Lua registration callbacks remain necessary before the
new Unreal host can reproduce the original game exactly.
