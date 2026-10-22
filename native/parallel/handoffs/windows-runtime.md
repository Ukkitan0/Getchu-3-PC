# Native Windows Runtime & Integration — specialist handoff

**Specialist branch:** `parallel/windows-runtime`  
**Integration target:** `native/original-elf-engine-recovery-20261020`  
**Starting commit:** `d483436416c14927851853f01c6ad66fe4cfbbf2`

## Delivered source / commits (2026-10-21 America/New_York)

- `974ea123de02f3cc8bde0489e7e4e51070211a58` — `native/src/app_win32.cpp`: strict `--retail` argument gate; missing/invalid original resource stage exits 3; successfully parsed retail stage still exits **4** instead of reporting a booted original engine. The explicit failure identifies missing original `0x57BCB8` manager setup and Player/Monkey frame dispatch; no substituted GLB workbench fallback. The ordinary asset workbench without `--retail` remains available as non-game diagnostics.
- `45cc04041db789278b79a0a1d882a8b9dec13662` — `native/BUILD_NATIVE_WINDOWS.ps1`: direct CMake configure/build; default full CTest (opt out with `-SkipTests`), optional `-RunRetail` invoking the existing original retail resource launcher; configurable offline caches for Lua 5.0.3 and cgltf. No GitHub Actions call and no bundled commercial assets.
- `fad94b277a949ac559dd4ee6504f92406c0b7764` — `native/BUILD_NATIVE_WINDOWS.bat`: single-command PowerShell entry point forwarding parameters and exit codes.

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
