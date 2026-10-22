# Original Lua 5.0 Scripting & Game Logic Bindings — specialist handoff

**Specialist branch:** `parallel/lua`
**Integration target:** `native/original-elf-engine-recovery-20261020`
**Pass:** 2026-10-21 — verified-format Lua dependency inventory and native binding/callback bridge
**State:** Implemented and committed; Python audit unit tests run. C++ Lua 5.0 runtime tests and owned retail-script tests are **not run** in the current network-restricted environment. **Not integrated or playable.**

## Source and commits

Starting shared commit: `d483436416c14927851853f01c6ad66fe4cfbbf2`.

- `3182d1de`: `native/include/ae3/lua_stage.hpp` defines `NativeLuaStageBindings`, `OriginalLuaNativeCallback`, `registerOriginalFunction` and `invokeCallback`.
- `2bc08990`: `native/src/lua_stage.cpp` implements native callbacks at the real Lua call site, converts original Lua arguments and native return values without replacing Lua logic, and forwards stage entities, parameters, flags, stage variables, mode and weapon selection to provided manager functions.
- `9f1b2955`, `ffd50212`: preserve unknown `stage_var_get_int` argument shape as the exact Lua value vector (do **not** invent a string-key contract).
- `1ef49bbf`: expand the existing Lua 5.0 C++ test with the stage setup → manager/flag/provider → Lua event callback chain and missing-provider rejection.
- `b80c08cf`: reject unrepresentable original native function arguments and restore Lua stack after a rejected argument during callback invocation.
- `1dc87269`: retail Lua 5.0 C++ regression test for strict flag-manager requirement across consecutive bytecode/source chunks.
- `cf63d235`, `5c738aa1`: `native/tools/audit_original_lua50_calls.py` and its four test cases for offline owner-private original `area.luc` opcode dependency audit. No game data is included.

All writes are within the Lua workstream's source/header/test/tool area and this handoff. No global CMake edits, GitHub Actions, emulation components, or original resources committed.

## Original evidence, distinguished from unverified behavior

- The inherited `retail_lua50.cpp` converter already handles the authentic Lua 5.0 USA chunk header, 32-bit size_t, float32 numbers, preserves the original 32-bit instruction words and nested function bodies. **This pass did not decompile/rewrite game script logic.**
- New offline audit follows Lua **5.0.3** `lopcodes.h`: `OP_GETGLOBAL=5`, `OP_SETGLOBAL=7`, 6-bit opcode and 18-bit Bx at bit 14 (see [Lua 5.0.3 opcode definition](https://sources.debian.org/src/lua50/5.0.3-6/src/lopcodes.h)). Reads/writes are opcode evidence, **not a confirmed native function-call graph**. It recursively traverses nested prototypes and rejects malformed/truncated headers and invalid constant references.
- Source-confirmed original callbacks inherited from prior recovery: `get_sex_distinction` at USA ELF VA `0x219C8C` reads `Data.player_type` at `0x649910`; `set_player_weapon` at VA `0x512000` decodes literal weapons then calls setter `0x266CF4`. Costume query functions at `0x617924` and `0x617958` inspect `Data.costume_possess`. Registration helper investigated in `native/tools/map_original_lua_registrations.py`: `0x255BD8`. This pass has **not** reverified these original ELF addresses against the owner's private binary.
- **No authentic game ISO/ELF/area.luc was accessible in this conversation's container.** Script-binding completeness, authentic stage initialization and the real update/mission callback order remain unverified.

## New C++ integration interface

`ae3::LuaStage::setNativeBindings(NativeLuaStageBindings)` accepts synchronous hooks:
- `createEntity(const StageEntity&)`, `submitParameters(const ScriptValue&)` — call gameplay/actor manager and stage parameter consumer at exactly the Lua invocation; descriptors still recorded for diagnostics.
- `writeFlag(scope,key,bool)`, `readFlag(scope,key) -> optional<bool>` — retail flag values must come from the **game-owned** global/local/area manager, not from the descriptor mirror.
- `readStageInteger(const vector<ScriptValue>&) -> optional<int>`, `readSurvival() -> optional<bool>` — integrator supplies these **only after** recovering their original argument/return/data semantics.
- `selectWeapon(int)` — forward inherited verified string→selection conversion to the original gameplay-owned setter/lifecycle.

`registerOriginalFunction(name,originalVA,callback)` allows independently recovered real Lua registration names to be bound without placing guessed functions in the core runtime. Requires nonempty name, nonzero aligned source VA, callable implementation, and refuses collisions with built-in globals. VA supplied by integrator is provenance metadata **not automatically attested** against an original ELF; independently verify it.

`invokeCallback(name,vector<ScriptValue>) -> bool` invokes a Lua-defined function with preserved Lua argument tables/numbers/strings/booleans; returns false only if no such Lua global exists, throws for non-function or callback error. `callSetupArea(spawnTag)` delegates to the same function with `{spawn_tag=spawnTag}` and preserves its required-function check. This **exposes a callback dispatch bridge**, not an invented original update/event schedule.

`stage_var_get_int` and `is_survival_mode` now reject absent providers for **all** scripts instead of silently returning zero/false. In retail-bytecode mode, `gw/lw/area_set_flag` and `gw/lw/area_get_flag` reject missing genuine game flag-manager hooks rather than applying stage-local synthetic state. Nonretail compatibility fixtures retain their legacy flag-mirror behavior. The retail-mode check is sticky across subsequent Lua source chunk loads.

## Tests, exact scope

Ran successfully in the local container against local authored copies of the exact auditor/test logic before pushing to GitHub:
```bash
python3 -m py_compile /mnt/data/ae3lua/audit_original_lua50_calls.py
python3 -m unittest -v /mnt/data/ae3lua/test_audit_original_lua50_calls.py
```
Results: **4/4 passed** on synthetic Lua 5.0 serialized header/prototypes/instructions, nested global references, source/global disambiguation and corrupted input. The committed copies should be re-run after checkout; no original scripts executed by this Python test.

C++ test added but **not executed**:
```bash
cmake -S native -B native/build
cmake --build native/build --target Getchu3LuaRuntimeTests Getchu3RetailLua50Tests
ctest --test-dir native/build -R 'NativeLua50RealStageExecution|NativeActualRetailLua50FormatRetarget' --output-on-failure
```
This environment does not contain Lua 5.0.3 headers/library or the downloaded FetchContent dependencies, and GitHub Actions must not be manually dispatched. Full Windows executable not built/booted.

Owner-private original source audit once the owned extraction is present:
```bash
python3 native/tools/audit_original_lua50_calls.py C:/AE3_PRIVATE/debug/us/stage \
    --host-source native/src/lua_stage.cpp --out C:/AE3_PRIVATE/lua_globals.json
```
Audit output remains **private**; do not commit original symbol inventories or script bytes. `unresolved_global_reads` is only candidate dependency evidence (may contain legitimate Lua library globals, dynamic globals, and definitions from additional modules). Compare to original ELF registration table and actual stage boot runtime errors.

## Integration request (Windows runtime, gameplay and assets)

1. Windows/runtime: keep existing `native/src/lua_stage.cpp` and existing `Getchu3LuaRuntimeTests` in global CMake. New Python audit/tools need **no CMake change**. Build and run the existing tests plus a privately supplied complete original stage.
2. Gameplay: provide a real stage-world callback/factory for `createEntity` and a real gadget/weapon owner for `selectWeapon`; do not reimplement Player/Monkey logic in Lua. Confirm original registration names and native function VAs against `SCUS_975.01`.
3. Runtime/save owner: wire global/local/area flag manager lifetime and persistence through `writeFlag/readFlag`; wire `PlayerProgress` at genuine game/new-game initialization. Do not reuse temporary stage mirror as retail flag manager.
4. Assets: supply owner-owned `area.luc` bytes through the existing original stage parser/ISO reader in correct verified load order. No changes to Asset parser requested.
5. Integrator: determine from original executable the real entrypoint-to-`setupArea` setup sequence, per-frame/event Lua callback names, registration order and engine-owned callback argument formats, then call `invokeCallback` from those recovered events. Never fabricate an `updateArea` API by analogy.

## Remaining source-backed blockers

- Original ELF/source registration tables and original **C++ callback targets** beyond a small inherited subset still need recovered input and disassembly. `registerOriginalFunction` is transport, **not** a claim the callbacks have been recovered.
- Mission events, stage integer variables, survival state, flag object ownership/persistence, actor instantiation/lifetime, gadgets and monkey AI require recovered original manager interfaces.
- Need exact asset/module script execution order; missing Lua libraries or globals must be resolved from retail evidence, not guessed.
- Original private USA `area.luc` execution must be run against a real `LuaStage` with provisioned game managers; failing unresolved bindings should be catalogued for recovery.
- C++ regression tests and real Windows native game boot remain unverified.

## Next coherent pass

Run the opcode dependency audit on the owner's **original 168 area.luc** samples, match all discovered non-library references against verified ELF Lua registration structures and native callback target addresses, implement a connected group of exact original flag/stage-event/mission bindings, then test complete real stage creation and callbacks under Lua 5.0 on Windows. No replacement gameplay.
