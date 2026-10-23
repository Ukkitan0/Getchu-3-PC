# Authentic Lua 5.0 inside Unreal: source integration and gaps

## Native and Unreal components

The importer branch includes a real Unreal C++ game and Editor project, plus a
runtime GameInstanceSubsystem. The original Lua 5.0.3 VM, original-bytecode
representation retargeter, and existing LuaStage from the native source tree
are compiled into a standalone Windows Getchu3OriginalLuaBridge DLL. The DLL
does NOT execute Sony MIPS/PS2 hardware or replace original Lua gameplay.

- native/include/ae3/ae3_unreal_lua_bridge.h : version 3 C callback ABI.
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

The C ABI version 4 has no C++ STL objects across DLL boundaries. Entity
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

## Original game manager transaction lifecycle

Original stage initialization may make many source Lua manager calls and then
fail on a later unknown original function. The Unreal runtime now requires
the actual manager provider to implement an **atomic** setup operation:

- BeginOriginalStageSetup is invoked after verified DLL loading, before any
  original Lua bytecode is executed. A provider must create a rollback-safe
  transaction over original actor lifetime, stage parameters, global/local/
  area flags and equipment changes. False means source state is unavailable.
- Every original Lua operation executes inside the transaction. A failed
  callback or missing original script-defined setupArea causes an immediate
  AbortOriginalStageSetup, including when Begin partially allocated resources.
- CommitOriginalStageSetup must apply the exact recovered original game
  state and return true only on success. A rejected commit is followed by
  AbortOriginalStageSetup.
- StopOriginalStage also aborts any still-open transaction before unloading
  the real native Lua DLL. Default manager methods are fail-closed, and
  no generic Unreal actors are silently left behind.

This transaction contract prevents a partially created imitation level.
Implementing an original transactional manager with verified actor/flag
ownership is still a required engine-recovery task, not a completed feature.

## Original stage binding-recovery report

A verified ISO import also runs existing native Lua 5.0 callsite analysis and
verified original SCUS_975.01 CreateBind registration analysis together.
The owner-private getchu3_original_binding_evidence.json contains the
original direct-call names, candidate actor class string literals, source
CreateBind addresses and unresolved original callback priorities. A failure
to produce static evidence is explicitly recorded but does not prevent
independent geometry import. These are static leads, NOT implementations,
a complete call graph or proof of behavior.

## Original stage IDs now resolve directly to verified owner-local bytecode

After a SHA-verified USA ISO import, the plugin creates an original stage
catalog, independently checks each source `area.luc` file's SHA-256 and Lua
5.0 signature, and writes a private verified stage index. This prevents
extracted stage scripts being silently substituted after the ISO verification.

Unreal exposes `UGetchu3OriginalLuaSubsystem::StartImportedOriginalStage`
to Blueprint and native callers: supply the source stage ID, original spawn
tag and a **real** `UGetchu3OriginalManagerProvider`. The subsystem reads
the most recent successful import status under `Saved/Getchu3Private`,
resolves the stage and its unique `area.luc` within that workspace, verifies
the source SHA-256 again, and only then invokes the recovered original Lua 5.0
runtime. The previous raw script-path launch method is now a private C++
implementation detail, not a Blueprint bypass.

Stages without original scripts, duplicate IDs, multiple `area.luc` records,
hash mismatches and paths escaping the imported owner-private tree are
explicitly rejected. All imported original stage data remains local.
No actor, monkey, gameplay save, sound, or camera is fabricated by selecting
a stage; until the original managers are fully implemented the stage still
fails to become playable.

Seven local Python stage-integrity regression tests passed (valid source,
tamper, path escape, symlink escape, ambiguous ID, missing script, and
source-less stages). Windows Unreal C++ compilation and ISO-level gameplay
execution still require external verification; this source gate is not a
claim that any stage can be played.

## Verified binding transport corrections and Windows test gate

The Unreal branch now preserves exact original Lua 5.0 byte strings across
the full native actor/flag/controller path. In addition to entity class/name
and tag identifiers, `gw_set_flag` / `gw_get_flag` and related local/area
flag keys use `lua_strlen` rather than zero-terminated `std::string`
construction. `c_ctrl` stores the original command with `lua_pushlstring`
instead of `lua_pushstring`. This matters for embedded zero bytes and other
non-UTF-8 source characters: the DLL later passes names as hexadecimal bytes,
but it cannot recover bytes already truncated inside LuaStage.

`native/tests/lua_runtime_test.cpp` now checks real Lua 5.0 execution of a
stage function whose flag key and controller command contain embedded NULs.
It asserts exact native callback keys, stage-state flag lookup and nested
entity controller opcode contents. This is an original Lua interpreter
binding test with a synthetic script; it is **not** evidence that all retail
`area.luc` files execute or that actor AI is complete.

The Windows `BuildAndOpen.bat` pipeline now compiles and tests
`Getchu3OriginalLuaStageBindingTests` independently of the entire
Getchu3Assets graphics/navigation/runtime library, **as well as** the
previous `Getchu3OriginalUnrealLuaBridgeTests` ABI rejection suite.
Both are required before Unreal Editor compilation, providing a practical
failure gate for the actual Lua value pipeline.

There is still no verified Win64 build here: Unreal 5.6/Visual Studio
are unavailable in this execution environment. The source tests must
execute in the owner's Windows toolchain and may reveal additional
integration failures. Missing original game managers remain the
main block on running the actual game.

## Win64 stage SHA-256 crash regression fixed

The imported-stage loader previously invoked
`FPlatformMisc::GetSHA256Signature`. The generic implementation is a hard
`checkf` stub on some Windows Unreal engine builds and may terminate the
editor instead of hashing. Original Lua source authenticity now uses Windows
CNG `BCryptHash` SHA-256 through a shared source file
(`native/include/ae3/windows_source_sha256.hpp`). The Unreal Runtime module
links the Windows SDK `bcrypt.lib`. The native Win64 test target
`Getchu3OriginalWindowsSha256Tests` exercises **the identical helper**
with the SHA-256 empty and `abc` reference vectors and a deterministic
synthetic Lua 5.0 header. Invalid arguments and overflowing buffer sizes
must be rejected. A malformed stage catalog is also rejected by
`TryGetBoolField` without assuming a missing JSON field can be read safely.

The one-click Windows build now requires
`OriginalWindowsSHA256SourceVectors` CTest to pass before launching the
Unreal Editor, in addition to the native Lua bridge and stage-execution
tests. No SHA1 downgrade or external network hash verification is used.

Verification performed on the local Linux tooling: all three **reference
test-vector expectations** independently match Python's `hashlib.sha256`;
the seven source-catalog Python integrity tests pass. The BCrypt helper
and Unreal stage loader themselves still require compilation and execution
on Windows with UE 5.6; they are **not** reported as Windows-tested.

## Independent original-Lua bridge build and callback-lifetime hardening

The one-click Windows launcher now configures `native/unreal_bridge`
instead of the complete `native/CMakeLists.txt`. This builds the real
recovered `LuaStage`, `retail_lua50`, `PlayerProgress`, the Lua 5.0.3
runtime and the ABI/stage/Windows-hash tests without configuring unrelated
cgltf, zlib, audio, physics, graphics or navigation targets. The Lua 5.0.3
download URL and pinned SHA-256 remain identical to the full native build.
The isolated build directory is ignored by Git, and the resulting Windows
DLL is staged locally under `unreal/Binaries/Win64`.

Build-structure validation: a local CMake configure/build/CTest pass using
**temporary empty C/C++ sources and a fake Lua source directory** succeeded
for the two non-Windows target definitions. This only verifies the isolated
CMake graph. The recovered native C++ and Lua 5.0.3 code was not compiled
in that smoke test; neither MSVC, the real Windows SHA test nor Unreal
Engine were available. The actual full Windows DLL and game are unverified.

The game instance runtime now sets an explicit native-call guard around
beginning stage transactions, original player-data lookup, Lua bytecode
execution, `setupArea` and the final manager commit. If an original game
manager re-enters the subsystem to Stop or replace the stage while any
such call is active, cleanup is deferred until the call unwinds. A rejected
or cancelled stage transaction is then aborted before the VM and its DLL
are destroyed. The native C ABI returns an explicit failure code if destruction is attempted
while a callback is still executing; Unreal retains the DLL and VM when it
receives that failure instead of unloading live native code. This prevents invalidating the native Lua stack
from an Unreal manager callback but is NOT yet proven in UE5 on Windows.

The original actor factory, monster/monkey update logic, player gadget
systems, stage and camera scheduler, animation bindings and sound engine
remain unimplemented and the Play control is still disabled.

## Verified stage bytes, not just verified file names

`StartImportedOriginalStage` first verifies the owner-private stage catalog
and SHA-256 of `area.luc` on disk. The private `StartOriginalStage` loader
now requires that exact original SHA-256 as an argument and checks the actual
bytes loaded into the `TArray<uint8>` it passes to the native Lua runtime.
If the source file changed between the two loads, the call fails before
loading the DLL or invoking any game manager. This prevents a previously
verified script path from becoming a stale authorization for different bytes.

`Getchu3OriginalWindowsSha256Tests` now tests both a known synthetic Lua
header digest and rejects a one-bit change to that input. This test is wired
into the Windows build gate but has not yet run under Win64 here.

## Explicit native VM destruction contract (ABI v4)

`ae3_lua_destroy` now returns 1 when the VM is gone (including a null
handle), and 0 if it is still active. Unreal must **not** unload the DLL on
0; it retains both the VM handle and library for a later safe cleanup.
A native ABI test calls destruction recursively from the original
PlayerProgress provider during `ae3_lua_reset` and verifies rejection.
The same runtime retains atomic Begin/Commit/Abort manager rollback.
Headers, test code and Unreal share the named `Ae3LuaManagerV5` structure.

## Recovered original ChangeHP gameplay routine (ABI v4)

The source branch now exposes the game's recovered native `ChangeHP(delta)`
routine directly to Unreal as `UGetchu3OriginalLuaSubsystem::ApplyOriginalHealthDelta`.
It operates on the established **1,033-byte original PlayerProgress layout**,
whose health field is at offset +8. The native code calls the existing
`ae3::changePlayerHealth`, implementing the source-recovered
`clamp(life + delta, 0, 100)` behavior; it does not use Unreal's
default health component or invent an independent health variable.

The expanded `Ae3LuaManagerV5` source ABI requires both
`read_original_player_progress` and
`write_original_player_progress` callbacks. The manager must provide
the complete 1,033 bytes and **atomically persist** the updated record to
the real player manager. Every other source field, including capture
and shop flags, is preserved verbatim. A missing read/write manager, failed
write, nonfinite source life/delta, reentrant manager call, or missing active
committed stage is rejected; the call does not claim success. Only after
the write succeeds does it update the retained LuaStage progress snapshot.

Unreal exposes the same explicit manager contract through
`ReadOriginalPlayerProgress` and `WriteOriginalPlayerProgress`
BlueprintNativeEvents. Both default to rejecting requests. An actual
source-backed player/save owner is still necessary to make the operation
usable during gameplay.

New native CTest `OriginalChangeHPSourceStateReadWrite` uses synthetic
1,033-byte player state to exercise -30 damage, +90 healing,
negative overflow, clamp to 0/100, unrelated capture/shop flag integrity,
and rejection of missing or refused source-manager writes. The Windows
one-click source build now includes that test.

**Verification boundary:** The test has been committed and wired to CMake,
but it has not yet been run against the real bridge on Windows or inside
Unreal 5.6. This is one recovered native gameplay function, not a
complete playable Player, actor factory, or stage simulation.

## Original native actor/controller Lua callback dispatch — ABI v5

The existing authentic `LuaStage::registerOriginalFunction` implementation
can install source-recovered original game native function names+addresses,
but the Unreal DLL interface previously had **no way** to register those
functions. ABI **v5** adds `ae3_lua_register_original_void` and the host
`invoke_original_void` callback. Registration is allowed only before an
original Lua stage bytecode execution attempt, only for ASCII Lua global
names with an aligned nonzero independently recovered original function VA,
and only if the host actually supports native implementation dispatch.
Built-in globals (`new_ent`, etc.), duplicates, invalid names, missing
manager callbacks and late registrations are rejected by the real Lua VM.

In Unreal, an explicit source manager may report up to 256
`FGetchu3OriginalVoidFunction` entries via
`CollectVerifiedOriginalVoidFunctions`. Each must contain a verified
original callback VA (NOT a registration `jal` call-site address).
The Unreal subsystem validates those entries before executing `area.luc`
and forwards later source Lua calls through
`InvokeRecoveredOriginalVoidFunction(VA, Name, TypedArgumentsJson)`.
Every Lua argument is marshaled using the same lossless type-tagged original
value representation. A native callback returns **zero Lua values** only
after the actual recovered C++ routine reports success. Any refusal throws
a Lua binding failure, causing the existing transactional stage setup to
abort. No callback name is automatically imported from the ELF report
because observed CreateBind names alone do NOT prove original behavior.

Both new manager events default to **false**, and no source-specific
actor class or controller routine has been fabricated. This is a
**functional registration and dispatch bridge** for future recovered
native original game routines, not a finished actor factory. Actual
stage lifecycle, player, monkey, controller and capture behavior remain
unimplemented until the original manager function bodies are recovered
and registered with verified addresses.

`native/tests/ae3_unreal_lua_bridge_test.cpp` adds ABI regressions
for missing native invoke owner, invalid name/VA, duplicate registration,
built-in global collision, rejecting late registration after an incomplete
retail chunk, and stage VM reset. Those tests are committed but have
not yet been compiled against genuine Lua 5.0.3 or run under UE5.6 Win64.

## Recovered original Player resource routing and actor-child lifecycle in native DLL

The standalone native `Getchu3OriginalLuaBridge` target now compiles the
actual decompiled `original_player_factory.cpp` and
`original_manager_child.cpp` source files directly alongside `LuaStage`.
These are **original game logic**, not invented Unreal components.

The exported `ae3_original_player_resource_lookup` calls the recovered
retail `originalPlayerResourceLookup_34a014` routine. It compares the
entire **16-byte** zero-padded original name and returns the borrowed
original source-owned subobject pointer. Source names include `player`,
`main_player`, `item_chatcher` (original spelling), `force`, and
six further resources behind the original secondary-factory presence
check (`bar_interface`, `iZone`, `iHanger`, `iRope`, `iDashBall`,
`iHeroSwitch`). A missing original resource or gated factory yields
`nullptr`. The bridge never creates a generic Unreal player or returns
a fabricated resource. Supplying authentic source-owned pointers remains
the responsibility of the future original Player manager.

The already-recovered manager-child source implements original constructor
and vtable-dispatch functions `0x57BA98`, `0x57BAF8`, `0x57BC0C`,
`0x57BC54`, `0x57BC88`, and destruction routines. Their original
`0x57BCB8` navigation initializer still needs the genuine actor/stage
ownership, original geometry, and route callback implementation: this is
not yet a functioning actor-spawn factory or game update loop.

Two source-backed CTests — `OriginalPlayerResourceVirtual2` and
`OriginalManagerChildCtorLifecycle` — now compile the actual original
routines, exercise pointer routing, original flag checks, child virtual
calls and destructor ordering, and are part of `BuildAndOpen.bat`.
A separate C ABI regression verifies the public Player lookup wrapper's
exact 16-byte matching and secondary-factory gate. **These new Win64
CTests have not been executed in this environment.**

## Direct original actor initializer — 0x57BCB8 and original stage navigation

The native DLL now directly compiles and exports the recovered actor
controller initializer, originalManagerInitialize_57bcb8, through
ae3_original_manager_init_create/step/destroy. It uses the genuine
decompiled C++ path with exact 0x70-byte source polygon records,
0x1E-byte navigation descriptors, VU0 vectors, original seeded game
RNG (0x36F3C0), source route-kind lookup (0x221104/0x21B444),
and type-2 source target calculation (0x57BDB8/0x57C98C).

The C ABI persists genuine RNG state between calls. The real game
manager must supply the original seed and decoded source records;
the host copies those into aligned, call-local records and returns
an original polygon INDEX rather than a pointer into expired memory.
The API returns the executed original branch and publishes source-
computed controller state only after a successful computation.

The general branch still requires genuinely recovered owner callbacks
for 0x21B854 projection and 0x57C7CC route continuation. The source
polygon and exact four-component vectors are forwarded, unchanged,
and missing/rejected callbacks fail, rather than using Unreal traces
or invented movement. If a real manager changes game state during
those callbacks, the game manager must preserve atomic transactions
and roll back on failure. Reentrant destruction while a source call
is on the native stack is explicitly rejected.

Two new Windows CTest targets have been added:
OriginalManagerInitializer57BCB8 validates original near-owner,
type-2 source polygon and seeded RNG, plus projection/midpoint order.
OriginalManagerInitializerHostAbi checks the public C ABI,
malformed source routes, missing projection/follow-up, and
reentrant cleanup. Both are invoked from BuildAndOpen.bat.

These are tests of recovered source code with synthetic structural
data, not proof that the real USA stage is playable. MSVC/UE5.6
compilation and retail navigation PCK execution remain unverified.
The original Player/Monkey actor owner and gameplay update loop
remain missing.
