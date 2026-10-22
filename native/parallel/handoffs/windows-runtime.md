# Native Windows Runtime & Integration — specialist handoff

**Specialist branch:** `parallel/windows-runtime`  
**Integration target:** `native/original-elf-engine-recovery-20261020`  
**Starting commit:** `d483436416c14927851853f01c6ad66fe4cfbbf2`

## Delivered source / commits (2026-10-21 America/New_York)

- `974ea123de02f3cc8bde0489e7e4e51070211a58` — `native/src/app_win32.cpp`: strict `--retail` argument gate; missing/invalid original resource stage exits 3; successfully parsed retail stage still exits **4** instead of reporting a booted original engine. The explicit failure identifies missing original `0x57BCB8` manager setup and Player/Monkey frame dispatch; no substituted GLB workbench fallback. The ordinary asset workbench without `--retail` remains available as non-game diagnostics.
- `45cc04041db789278b79a0a1d882a8b9dec13662` — `native/BUILD_NATIVE_WINDOWS.ps1`: direct CMake configure/build; default full CTest (opt out with `-SkipTests`), optional `-RunRetail` invoking the existing original retail resource launcher; configurable offline caches for Lua 5.0.3 and cgltf. No GitHub Actions call and no bundled commercial assets.
- `fad94b277a949ac559dd4ee6504f92406c0b7764` — `native/BUILD_NATIVE_WINDOWS.bat`: single-command PowerShell entry point forwarding parameters and exit codes.
- `88c892741d7c892663270abe5aa0aa0a233b6426` — `native/BUILD_NATIVE_WINDOWS.ps1`: test all registered native components by default; `-SkipTests` is the explicit exception.

## Original engine evidence and first concrete blocker

The current `app_win32.cpp` is a Win32/D3D11 **workbench**. `loadOriginalRetailStage` reads retail ISO `debug/us/stage/<stage>/area.luc`, loads extracted GLB/I3C/PLC data and executes native Lua 5.0 setup, but its `WM_TIMER` frame handler advances debug controller timelines, separate walking code and `NativeActorWorld::tick` for animation. It does **not** construct/run the real Player/Monkey managers nor dispatch the retail engine's frame updates. Report a blocked native boot rather than a successful game boot.

Repository retail opcode audits already pin `0x57BA00` -> `0x57BCB8` (original manager child's genuine initializer, direct original sites `0x57BAE4`/`0x57BB4C`) and Monkey lifecycle `0x571904`. The genuine `0x57BCB8` implementation remains unresolved and is delegated to a callback; original startup/main-loop roots are not yet reconstructed. **No new retail instruction source was available to reverse-engineer this turn.** No original executable bytes committed.

## Tests / verification — precisely scoped

- GitHub connector read of both branch heads and all required workstream instructions succeeded.
- Static CMake source inventory: **154 distinct C/C++ source and test references, 0 missing** in starting tree; **not** a compile/link check.
- **11/11** source-contract assertions passed across the two branches, including actual changed-file contents and exit-code gating; this is **static** source verification only.
- **Not run:** Windows/MSVC build, CMake configure/CTest, Win32 launch, retail ISO/ELF checks, original asset loading, native gameplay, PowerShell execution. Current runtime lacks a Windows compiler/PowerShell; GitHub clone DNS is unavailable. No Actions manually triggered.
- Full source could not be reproduced in the container; do not treat this as a verified build.

## Integration request

The integrator should cherry-pick reviewed commits from `parallel/windows-runtime` onto `native/original-elf-engine-recovery-20261020` only after coordinating, then run `native/BUILD_NATIVE_WINDOWS.bat` on owner Windows (full CTest enabled by default). Do not interpret exit 4 of `Getchu3Native.exe --retail ...` as game boot; it indicates the specific missing original-engine init/tick path. The existing launcher Python tool treats nonzero as failure. No global `native/CMakeLists.txt` edit was justified by the source-inventory audit; missing build/link errors require an actual Windows compile log.

## Remaining highest-priority real work

1. Verify private retail USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`, reverse engineer original startup, manager initialization (`0x57BCB8`), scheduler and frame integration; no invented player update.
2. Integrate source-backed actor and Lua handoff APIs into authentic frame routing, not the debug WM_TIMER replacement.
3. Re-run full MSVC CMake/CTest and actual Windows executable with the user's original resources, recording first failed call and full stack.
4. Only allow native boot success when real original game initialization and frame dispatch have been run; remove the present exit-4 block **only then**.

## Runtime integration pass — boot checkpoints and hardware fail-fast

**Commits:** `f705266da1ced6393804b057de711bffa13f2443` (journal header), `01901067c13faaf016fce86f2cb690f0069279c2` (test), `1f87097605cecf8e544bcf7f9a21521949947864` (CMake/CTest), `a45f90bf72745d84ae0847848f1f224a78250ed4` (Windows app), `b7b1e5dc2c0799e2bb1f9f767943fc8660b9959a` (Release-test assertion fix).

- **Preserved preceding runtime work:** Existing strict `--retail` launch handling, explicit failure exit code 4, evidence for missing original `0x57BCB8` manager startup, and existing `native/BUILD_NATIVE_WINDOWS.ps1` and `.bat` scripts were retained. The branch head advanced concurrently; changes were applied via file-specific GitHub commits, with no ref resets or force pushes.
- **Native app startup:** `app_win32.cpp` now aborts WM_CREATE when D3D11 initialization or Win32 timer creation fails. It verifies the authentic original disc/VFI and original retail Lua header before requiring separately exported GLB geometry. Startup phases include original stage/GLB/I3C/Lua setup and explicit actor/retail-loop failures. Host-only Lua and GLB success are never presented as playable original gameplay.
- **Diagnostic output:** `%TEMP%\\Getchu3NativeBoot.log` plus `OutputDebugStringA` contain `AE3_NATIVE_BOOT|<stage>|PASS/BLOCKED|detail` records. No ISO, retail Lua, ELF or game asset bytes in source control or runtime logs.
- **Compiler/test evidence:** Linux `g++ (Debian 14.2.0-19) -std=c++20 -Wall -Wextra -Wpedantic -Werror -O2` compiled/executed the original host-only BootJournal test locally with PASS; the committed final test has equivalent checks, plus a Release/NDEBUG guard. This does *not* exercise Win32, Direct3D, the full game, or original resources.
- **Repo audit:** Recursive baseline tree 769 entries; 154 explicit CMake source/test paths, no missing referenced file; all 65 `native/src` C++ sources and 89 baseline native test C++ sources were registered before this pass. Added a dedicated CTest target for the new runtime journal.
- **Original source provenance:** No new retail machine-code routines recovered in this pass. The original USA `SCUS_975.01` (repo-referenced SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`) was not accessible in this session. Existing original actor VAs and `0x57BCB8` evidence were preserved; the authentic retail process entrypoint, manager graph, original scene/update scheduler, and complete actor AI/frame dispatch remain unresolved.
- **Build/run limitations:** A complete Windows/MSVC native executable build, a full CMake configure/CTest, and a real owned-disc boot attempt were **not run** in this Linux container (no Windows SDK/MSVC or access to source archives through git/network). GitHub Actions were **not dispatched**.
- **Integration request:** Review the individual commits above and selectively integrate the owned app/CMake/journal work into `native/original-elf-engine-recovery-20261020`. On Windows, run `native/BUILD_NATIVE_WINDOWS.bat` or `native/BUILD_NATIVE_WINDOWS.ps1 -RunRetail -OriginalIso ... -Stage ... -Extracted ... -Glb ...`; read `%TEMP%\\Getchu3NativeBoot.log` for the first blocker. Need source-backed original init `0x57BCB8`, main loop and actor/gameplay handoffs before changing strict exit 4.

### Additional locally executed Release-mode checkpoint validation
- On Linux, configured an isolated CMake harness (CMake 3.31.6; GNU C++ 14.2.0; `CMAKE_BUILD_TYPE=Release`) using a semantically equivalent local copy of the new runtime journal header/test, built its `Getchu3WindowsBootStatusTests` target, and ran `ctest --output-on-failure`: **1/1 passed**.
- This is explicitly **not** a full configure/build of `native/CMakeLists.txt`, does not verify MSVC/Win32 application compilation, and is not evidence of authentic original gameplay boot. Source in GitHub remains authoritative.

## 2026-10-21/22 continuation — staged native specialist integration and reproducible startup failure reporting

**Owned commits (this continuation):**
- `560ea8352796120462573a9430bfad1890c5ae76`: add `native/cmake/ParallelIntegration.cmake` with presence-checked original-source integration groups; a complete reviewed source/header/test set is required before activating each group.
- `92b94b45a8df2836f58cf9423a20175ab7aef360`: include the integration manifest from `native/CMakeLists.txt` after the baseline engine targets/tests.
- `47a2c1ddccfb188d2ad836349c9bf9ef3c4c5cdf`: original VU0 source translation built with noncontracted strict FP expressions (MSVC `/fp:strict` or GCC/Clang `-ffp-contract=off -fno-fast-math`); link Win32 user32 when source-backed input adapter is present.
- `5822388591bb74951b0ca2d93697e0d29a3a53da`: `app_win32.cpp` uses `AE3_NATIVE_BOOT_LOG` environment path for distinct boot-attempt checkpoint logs, with existing Windows temporary-path fallback.
- `dfcb6f41902601ec45eca5c40f888daa55cee0e8`: `native/BUILD_NATIVE_WINDOWS.ps1` gains `-RequireIntegratedWorkstreams`, optional owner-supplied `-OriginalElf` SHA-256 validation and a unique per-attempt retail checkpoint report with first-blocker summary.
- `cafb7ba9b89db8f920d6b530a6febb8a8bab5f09`, `24c9df74af21473de7428ae2a8c6bc02418d5b73`, `36c2031a2dd532b54c3a49cf70ed7bbdf72968a2`: isolated CMake manifest regression test, CTest registration when Python is available, multi-config Windows correction.

**Specialist source review — file-registration only, NOT a cross-branch merge:**
- Assets `parallel/assets`: typed original VFI/PCK owner (header-only), SZ raw-DEFLATE decoder (zlib source), two structural tests, optional private EXDB differential fixture. New `original_resource_sz.cpp` registered only after full seven-file group is present.
- Navigation `parallel/navigation`: stage route/group registry two new sources and two tests, optional owner-private nav-PCK fixture.
- VU0 `parallel/vu0-physics`: original fallback VU0 source plus one focused test, strict ordered floating-point compiler options.
- Graphics `parallel/graphics-animation`: original render instances, early render visibility gate, selector sources and three tests.
- Input/Audio `parallel/input-audio`: original pad/audio runtime, Windows XInput adapter (host transport only, no invented gameplay bindings), one focused test and Win32 user32 link.
- Lua `parallel/lua` and Gameplay `parallel/gameplay-engine` modify already-registered C++/test targets. Their files **must still be selectively merged by integration owner**; CMake presence cannot prove original callback or actor completeness. No other team source has been copied into this branch.

**Exact local checks this continuation:**
1. GitHub API inspected current branch/source and seven specialist heads/changed paths/handoffs; branch untouched outside owned CMake, app, launcher, test and this handoff.
2. Linux host, CMake 3.31.6 / GNU C++ 14.2.0 / Python 3.13.5. **Locally executed source-equivalent CMake manifest fixture**, baseline configure succeeds; strict mode without groups fails; partial navigation (one of six files) fails with explicit missing file list; all five groups staged in temporary CMake-only **placeholder compilation units** and register **9/9 passing CTest fixture targets**. These placeholders only validate CMake's file registration, never ship and never imply working game modules.
3. Re-ran existing isolated `/mnt/data/native_runtime_test` CMake Release journal build and `NativeRetailStartupCheckpointEvidence` test: **1/1 passed**.
4. The exact committed cross-team C++ units have **not** been compiled together in this environment. Source-equivalent tooling, not a GitHub checkout of the full project, was executed. Entire Windows/MSVC build, Direct3D run, genuine original-ISO boot and actual frame dispatch remain **NOT TESTED / NOT WORKING**. No owner-private original data is in this runtime workspace. No GitHub Actions manually triggered.

**Integration directions and runtime contract:**
1. Integrator cherry-picks selected specialist files into `native/original-elf-engine-recovery-20261020`. Newly added source/test groups self-register only when all declared files have landed; partial subsets fail configure rather than silently linking the previous implementation. Request `-DAE3_REQUIRE_PARALLEL_INTEGRATION=ON` after all five new groups are merged. Do not merge blindly; reconcile header/source ABI and run their **actual** unit tests.
2. Source-verified original engine blockers remain: missing startup hierarchy, factory/manager initialization around `0x57BCB8`, stage resource ownership (real original VFI names and allocated base VAs), genuine Player/Monkey per-frame update dispatch, original render callback and physical-input/audio consumers. **None is supplied by the CMake test fixture.**
3. On owner Windows, run `native/BUILD_NATIVE_WINDOWS.bat -RunRetail -OriginalIso <private-iso> -Stage <original-stage> -Extracted <private-extracted> -Glb <private-glb> -OriginalElf <private-SCUS_975.01>` after placing original resources locally. Compare the script's first `AE3_NATIVE_BOOT|...|BLOCKED|` checkpoint with `OutputDebugString` and the game's actual startup; failure is expected while original manager/engine loop remains untranslated.
4. Source authentication: original USA `SCUS_975.01` must SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Optional launcher hash check validates local file bytes but does not authenticate a runnable Windows translation. No ELF/ISO/PCK/Lua/model bytes are committed.

## 2026-10-22 — Genuine original executable startup recovered and first native constructor ported

**Private source recovered for THIS analysis (never committed):** materialized the previously user-uploaded `AE3_original_Lua_ELF_and_WoodsC_private.zip` from the user's private Library, extracted `SCUS_975.01` locally only, and verified SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63` (6,488,148 bytes). Owner-private `area.luc` stage data is also present in the private archive but has not been included in Git.

### Original startup sources now explicitly established

- Executable ELF entry point is **`0x100134`**. Startup at `0x100248..0x1002C4` performs original zero/bss/platform initialization. At `0x1002C4` direct call `0x1036D8` precedes original environment setup; native Windows is NOT to emulate these PS2 hardware/OS operations.
- Source original calls: `0x10031C -> 0x43FE58`, `0x100324 -> 0x435D60`, `0x10033C` indirect call via constant `0x628210`, `0x100354 -> 0x428B80`, `0x10036C -> 0x1037E8`, `0x100374 -> 0x428CB8`; `0x628210 -> 0x1000A0` and `0x428AB8`. Seven direct JAL targets were validated from authenticated ELF bytes. The startup controller **must not claim completeness** from discovering these edges; the initial original OS adaptation and `0x1037E8` are not ported.
- `0x428AB8..0x428B0C` is the original static-constructor dispatch loop: uses pointer `0x62E89C`, traverses **143 actual original constructor function addresses** in descending table address order until `0xFFFFFFFF` sentinel at `0x62E660`. First source address `0x41A390`, second `0x417C58`, last `0x10526C`. Source ELF loop SHA-256 `01e600ce72db3b1290949e9f8f772559faa937eb8b034f4e6bae5688115d7b1b`. Table bytes SHA-256 `a3c5739dcec7d153ee3e72a04b14046ce4e9366d68537258cab6bd168eac99ba`; native ordered-address FNV64 `e8da5c252d4839d5`. The committed C++ contains source-function address metadata only, NOT proprietary machine instructions or commercial assets.
- Original first static constructor `0x41A390` tail-dispatches `0x41A088(1,65535)`. Its **12 original global 64-bit counters** are independently incremented once per invocation. First-increment callbacks all invoke original `0x4173B0`, whose verified complete body is `JR RA; NOP`, with no side effects; native C++ reproduces exact observable increment/wrap semantics using owned native globals, **not PS2 RAM emulation**. Original first ctor function SHA-256 `75a521c6fd605abeba2de1d1b6880f78ec824dbfe3e4e89a4db567ac669437d9`; counter initializer body `07cac5712617a817c73f96b849d99560b9bdfefd2b02f23ea94360020694a809`; no-op `6d64edf91449c1b17746c1ef18afa2eb25c70bdf1322ab3df5a2630993b7e2f1`.
- `0x428B80` original exit-registration routine stores 32 callbacks per 0x88-byte original block. `0x428CB8` starts its original cleanup by calling callback blocks newest-first and each block back-to-front; later original global owner hook +0x3C and exit kernel services remain unimplemented.

### Implemented work and commits (Windows/runtime-owned only)

- Native C++ `native/include/ae3/original_windows_bootstrap.hpp` and `native/src/original_windows_bootstrap.cpp` with authenticated 143 original constructor IDs, fail-closed genuine native callback dispatch, original 32-per-block registration/teardown order, and the *real first recovered* `originalAllocatorCtor_41a390` global-counter initialization. The latter is now actually dispatched by `--boot-original`.
- `native/tests/original_windows_bootstrap_test.cpp`: original table size/first/last/source-order/full fingerprint, first/third missing constructor rejection, 143 callback ordering, 71 destructor callbacks across three source-sized blocks, failure-preserving teardown and native 12-counter/wrap source behavior.
- `native/tools/verify_original_windows_bootstrap.py`: authentic USA ELF SHA and opcode-region SHA validations, 143-member table/sentinel checks, direct original call-edge targets. Never emits original machine bytes.
- `native/CMakeLists.txt`: new production source in `Getchu3Assets`; original Windows bootstrap regression CTest target.
- `native/src/app_win32.cpp`: distinct `--boot-original <original ISO>` entry path. Calls actual source-derived native constructor 0x41A390, then **fails closed at the next untranslated original static constructor 0x417C58**, rather than running the diagnostic Lua/GLB viewer or claiming the retail game booted. Existing `--retail` asset/Lua diagnostic remains separate.
- `native/src/windows_runtime_boot_status.hpp`: original static constructor checkpoint `RETAIL_STATIC_CONSTRUCTORS`. `native/BUILD_NATIVE_WINDOWS.ps1`: `-RunOriginalBoot -OriginalIso <...> [-OriginalElf <owner SCUS_975.01>]` builds native app, optionally verifies actual original executable/table, runs GUI with synchronous process exit and a unique checkpoint log. `-RunRetail` remains a separate existing resource-preflight mode.

### Actually performed tests

- **Private original source audit, RUN and PASS:** Python 3.13.5 tool using the SHA-verified private 6,488,148-byte ELF: full 143 original constructor addresses/sentinel; 7 original startup direct JALs; original termination JMP; 4 independently checked opcode/table SHA-256 ranges (constructor dispatcher, first ctor, 12-counter initializer, true no-op).
- **Host-native C++ Release test, RUN and PASS:** GNU C++ 14.2.0 `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror` on **source-equivalent** local copies of the new C++ logic and all 143 addresses extracted from owner ELF. Source-order/first-missing/recovered constructor/global-counter wrap and teardown block regression passed.
- **Host-native sanitizer test, RUN and PASS:** `-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer` source-equivalent native C++; no sanitizer diagnostics.
- **NOT RUN:** Full repository CMake/CTest, Windows SDK/MSVC compile/link, actual `Getchu3Native.exe --boot-original` Windows process, original stage rendering/input/audio, actual complete original game startup. Network-constrained environment cannot clone GitHub source or execute Windows binaries. C++ source-equivalent local tests do not prove exact final committed GitHub source compiles together; integrator must run the committed test targets in Windows build.
- **No** emulation, guest interpreter, fictional game callbacks, substitute play scene, copyrighted game bytes committed, force pushes, edits to other specialist files, or manually triggered GitHub Actions.

### Updated precise first authentic native game blocker

On the new source-derived static constructor path, **first missing original constructor is now `0x417C58`**. Its own original function tail-calls `0x417B50(1,65535)` and requires original `0x41D6B8` / `0x4240D8` native equivalents and original persistent global result fields. After this constructor and the remaining 141 callbacks, further host adaptations of `0x10031C`, `0x100324`, process startup `0x1037E8` and Player/Monkey manager `0x57BCB8` remain. The previously reported `0x57BCB8` blocker is downstream, not the first original executable startup instruction dependency. Do not change native boot exit code 4 to success until these functions are actually integrated and original gameplay frame scheduling executes.

### First-constructor source and native ownership follow-up

- `2baaabb5e48b14ead2ce1d5593191ccf3763d401`: the `--boot-original` path now **indexes the original ISO9660 DATA.BIN/VFI archive**, refusing arbitrary regular files named `.iso` before dispatching original constructors. A source path merely existing is not an authentic game boot prerequisite.
- `1ae9e7b202aec277322ec03f70d9388ccdf0dab9`: original native allocator global counter state is now **process lifetime**, rather than tied to the short-lived callback adapter. Preserves original global BSS ownership if future constructors and gameplay are recovered.
- Offline original ELF verifier negative-control test: flipped one private sample ELF byte locally; the verifier **rejected it on SHA mismatch**. No modified/private binary was committed or distributed.
- Next authentic function needing native port: original constructor `0x417C58` -> `0x417B50(1,65535)`, requiring original `0x41D6B8` and `0x4240D8`. The first allocator constructor `0x41A390` is real native code and advances execution exactly one entry beyond the formerly completely missing original constructor dispatcher. Original script/asset workbench is not a substitute for this path.

### Optional private executable audit in CTest and boot-order precision
- Commit `78773ee558be1e77e681c1ce6407f0cffda21b29` adds `AE3_RETAIL_USA_ELF` as an explicitly owner-private CMake FILEPATH. When set, CTest runs `OriginalUSAExecutableStaticConstructorOpcodeAudit` using the SHA-pinned original executable. The configure step rejects an absent file and never embeds it into game binaries or Git.
- **Boot-order qualification:** `0x417C58` is the first missing *static constructor within original dispatcher 0x428AB8*. The preceding original process/PS2 OS initialization at entry `0x100134` through `0x628210` still has unported native service adaptations. Do not call this a complete original ELF startup translation. The `--boot-original` path intentionally exercises only the recovered host-native constructor stage after native Win32/D3D11 initialization.

### Final locally executed source-equivalent CMake/CTest validation
- Executed a **new isolated** host CMake 3.31.6 Release build comprising the real-source-equivalent `original_windows_bootstrap.cpp` logic (actual 143 original addresses extracted from the SHA-pinned private ELF), first-constructor/exit-registry regression, and SHA-pinned ELF startup auditor. `cmake -S ... -B ... -DCMAKE_BUILD_TYPE=Release`, `cmake --build ... --parallel 4` and `ctest --test-dir ... --output-on-failure` completed with **2/2 PASS**. Actual source-equivalent GCC 14.2.0 optimized and ASan+UBSan test runs also passed.
- This isolated CMake test is **not** a full checkout or Windows native engine build. The committed GitHub test/source text was not recompiled by MSVC in this pass. Need Windows owner/integration branch run to verify full linking and game initialization.
