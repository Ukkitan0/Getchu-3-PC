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


## Continuation pass — 2026-10-21: exact Lua 5.0 stack dispatch and corrected bytecode evidence

### Source changes

- `675030ea`, `e82a0e65`, `479c1bbd`, `20002339`: `lua_stage.hpp/.cpp` makes typed Lua-to-C++ marshaling **reject** unsupported function/userdata/thread values, cyclic tables, mixed numeric/string-key maps, nonfinite numbers, oversized strings/nested tables and excessive nodes instead of silently rewriting them. Catches descriptor conversion errors at the `new_ent` Lua boundary. Original Lua values are never rewritten by those explicit rejects.
- `08bb1e56`, `c23b87e3`: `registerOriginalRawFunction(name,VA, std::function<int(lua_State*)>)` and raw callback dispatch for independently recovered original engine functions that need unmodified Lua 5.0 stacks (closures, Lua object identity, non-string keys, userdata); retains the existing typed bridge for simpler values. Runtime registration still requires a confirmed callback implementation and source VA; **no new original game function behavior was invented**.
- `2e80b68a`: adds C++ Lua runtime regressions verifying a cyclic table, same-string/different-numeric key, function closure round trip, failure of lossy typed conversion, and continued VM use after binding errors. **Not compiled/executed yet: Lua 5.0.3 FetchContent dependency unavailable locally**.
- `66d9849a`, `fd8fdb29`: fixes retail-mode poisoning: malformed bytecode cannot enable retail strict flags in a subsequent nonretail Lua chunk; adds C++ negative test. Pending C++ execution.
- `c5067982`, `9776e9f2`: repairs the original Lua 5.0 bytecode auditor's **significant opcode bug**: `iABx Bx` occupies bits 6..23, NOT bits 14..31. Previously passing synthetic fixtures inadvertently encoded an incorrect instruction format, so earlier dependency reports must be rerun. Source proof: exact Lua 5.0.3 `lopcodes.h` defines `POS_Bx=POS_C=6`, `POS_A=24`, `OP_GETGLOBAL=5` and `OP_SETGLOBAL=7` (<https://sources.debian.org/src/lua50/5.0.3-6/src/lopcodes.h>). Both parser and fixtures now use this layout.
- `da3d215f`, `737c21ef`, `802860f4`: further audits real Lua 5.0 **candidate direct global calls** via conservative per-basic-block register provenance: `GETGLOBAL R(A)` → `MOVE` aliases → `CALL/TAILCALL`. Calls across branches and unclassified indirect/upvalue/table invocations are intentionally omitted. Reports `direct_global_calls`, `prototype_calls` (prototype index, line number, candidates) and `unresolved_direct_global_calls` alongside raw loads/writes. These are **candidate direct call sites**, not verified original C++ bindings. Rejects unknown Lua 5.0 opcodes. Output remains private to source owner.

### Source-backed Lua registration leads (not yet implemented)

Published decompilation of `SetupBindFlag` includes **`gw_get_string`** with original binding vtable pointer `0x6B70C0`. This name is absent from the current Lua runtime registration. The same original registration table contains `gw_set_flag`, `gw_get_flag`, `lw_set_flag`, `lw_get_flag`, `area_set_flag`, `area_get_flag`, `get_costume_possess`, `get_costume_possess_ninja`, `stage_var_get_int`, and `is_survival_mode`. Source: <https://www.cnblogs.com/algonote/p/15678057.html>. The vtable pointer is **not** the Lua C++ method implementation VA. Do not register a guessed `gw_get_string` implementation until original callback body and manager semantics are recovered.

### Executed tests and limitations

Ran in this container on local authored copies of the committed Python script/tests (reproducing the revised script and fixtures):
```bash
cd /mnt/data/ae3lua
python3 -m py_compile audit_original_lua50_calls.py test_audit_original_lua50_calls.py
python3 -m unittest -v test_audit_original_lua50_calls.py
```
Result: **6/6 PASS**, including Lua 5.0 bit-position regression, nested prototype scans, GETGLOBAL/SETGLOBAL references, direct-call tracing, branch-barrier rejection, malformed bytecode and exact opcode-index bounds. Synthetic chunks only; original private `area.luc` not accessible. The corresponding GitHub versions have been committed; verification of a fresh checkout remains for the integration owner.

**C++ tests NOT run:** native Lua 5.0.3 headers/library not available locally and repository FetchContent cannot be reached. The new `originalRawLuaValuePreservationTests`, `nativeBridgeAndCallbackTests` and `testRetailBytecodeExecution` cases need to be compiled against the pinned original Lua 5.0.3 library and executed. No successful real Windows build, authentic retail-stage callback execution or boot is claimed.

### Immediate integrator actions

1. Build `Getchu3LuaRuntimeTests` and `Getchu3RetailLua50Tests` with the existing global CMake source list (no CMake file additions required); fix any issues reported by the actual target/toolchain.
2. Use a privately extracted original USA `area.luc` from the owner to run `audit_original_lua50_calls.py`, compare candidate direct calls against **real** `SCUS_975.01` register/vtable and callback code, and verify `gw_get_string` semantics.
3. Gameplay and Windows/runtime: use the new raw Lua stack callback only for original engine routines whose semantics actually require Lua identity/closures; keep stage actor instantiation in gameplay and save/flag storage in game state, not in this Lua adapter.
4. Original script-load order and stage event schedule remain blocked on authentic executable/resource evidence. Never infer an `updateArea` callback from generic Lua conventions.


## Continuation — 2026-10-21: verified ELF registration scan and bytecode call-site correlation

**Branch:** `parallel/lua`, built on `7db7179d744c6de90c4036d84f4b9aa2b1ab2531`.
This checkpoint adds **static original-engine recovery tooling**, not a fabricated native implementation of undocumented Lua functions.

### Git commits and source

- `fca7444d`: replace `native/tools/map_original_lua_registrations.py`'s naive backwards-nearest-LUI heuristic with a conservative register-constant chain: actual MIPS32 `LUI`, signed `ADDIU/ADDI`, unsigned `ORI`, zero-register `MOVE`, caller-saved register invalidation at `JAL`, and original MIPS delay-slot execution. Reject chains across branches and excessive lookback. ELF32 MIPS PT_LOAD bounds, short strings at segment ends, strict `SCUS_975.01` SHA256 check and helper `0x255BD8` are retained.
- `63286fee`: eight synthetic MIPS tests for delay slots, low-half sign extension, register copies, call-clobber barriers, branches, malformed ELF, short segment literal and wrong helpers.
- `a7872882`: new `native/tools/reconcile_original_lua_bindings.py` combines three evidentiary layers: authentic Lua 5.0 global loads and *conservative* direct call candidates; verified ELF `CreateBind` registration names and source code site VAs; and current host `registerCallback` names. It identifies candidate direct calls confirmed by scanned original registrations but absent from the host, without inventing C++ behavior or treating dynamically defined globals as missing.
- `512a3c7b`: four evidence-correlation unit tests, including source-defined Lua functions, absent registration helper families, fake ELF provenance, and invalid original call evidence.
- `f3391447`: original `SetupBindFlag` pseudocode from [Byaidu's published retail reverse engineering](https://www.cnblogs.com/algonote/p/15678057.html) identifies eleven *vtable object addresses*: `gw_set_flag 0x6B70F0`, `gw_get_flag 0x6B70D8`, `gw_get_string 0x6B70C0`, `lw_set_flag 0x6B7090`, `lw_get_flag 0x6B7078`, `area_set_flag 0x6B7060`, `area_get_flag 0x6B7048`, `get_costume_possess 0x6B7018`, `get_costume_possess_ninja 0x6B7000`, `stage_var_get_int 0x6B70A8`, `is_survival_mode 0x6B7030`. Scan the *verified ELF* at these addresses to report 24 raw bytes/six pointer slots per table, marking values inside the original text range as candidates. **No virtual slot is currently identified as the Lua implementation, and no C++ engine callback is fabricated**.
- `0cd00bb4`: ninth registration scanner unit test for vtable pointer extraction and missing PT_LOAD mapping.

### Tests actually executed

On the current local test sources (equivalent to committed Python tools) in this environment:

```bash
cd /mnt/data/ae3lua
python3 -m unittest -v test_map_original_lua_registrations.py \
  test_audit_original_lua50_calls.py test_reconcile_original_lua_bindings.py
```

**19/19 pass** on local synthetic MIPS ELF segment fixtures, bytecode prototypes and fake cross-reference reports. No original game binary or original game script data was present; therefore **zero verified real registration counts, zero authenticated new callback VAs, zero original stage execution tests**, and no C++ native app build are claimed.

For reproducing the committed test suite after checkout:

```bash
python3 -m unittest -v native/tools/test_map_original_lua_registrations.py \
  native/tools/test_audit_original_lua50_calls.py \
  native/tools/test_reconcile_original_lua_bindings.py
```

Owner-private original asset evidence pass (run only where verified original resources are available):

```bash
python3 native/tools/reconcile_original_lua_bindings.py \
  --elf C:/AE3_PRIVATE/SCUS_975.01 \
  --scripts C:/AE3_PRIVATE/debug/us/stage \
  --host-source native/src/lua_stage.cpp \
  --out C:/AE3_PRIVATE/lua_original_binding_evidence.json
```

The report includes exact source registration JAL VAs and separate published flag-vtable pointer candidates; **the report itself and source bytes stay owner-private**.

### Integration requests, blockers, next pass

- Assets/Runtime team: make privately extracted `area.luc` and verified original `SCUS_975.01` accessible to the native integration test environment, without ever committing original bytes. Run the combined report and publish only independently verified callback interface facts to the handoff.
- Gameplay team: once individual Lua callback target semantics are recovered from the real ELF, implement gameplay-facing native manager functions under Gameplay ownership; Lua can forward calls via `registerOriginalFunction` or `registerOriginalRawFunction`. No gameplay behavior is duplicated.
- Lua team: use vtable candidate pointers to identify actual virtual callback methods, arguments, and object state; only then implement source-matched bindings and a complete original stage callback lifecycle.
- Global CMake, workflow, executable entry point, resource parsers, and other subsystem code remain untouched. Existing C++ tests from previous pass remain **not executed**, as pinned Lua 5.0.3 dependencies are missing locally; no real game boot or original-stage behavior claimed.
