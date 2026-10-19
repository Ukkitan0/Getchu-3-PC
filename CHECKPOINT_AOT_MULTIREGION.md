# October 19, 2026 — native AOT nonlocal callback and Windows test-build checkpoint

**Scope:** source-only native Windows compilation and game-free R5900 execution
regression. The complete commercial Ape Escape 3 native game is **not playable**
or verified by this checkpoint.

## Verified baseline recovered from GitHub

The previous Windows Actions workflow run
[39316546929](https://github.com/Ukkitan0/Getchu-3-PC/actions/runs/39316546929)
at commit `7190a3c` finished successfully. It compiled the pinned upstream
`eerecomp`, `vurecomp`, ClangCL runtime and Win32 WebGPU path, and ran two
game-free EE AOT fixtures: direct/indirect JAL+JALR and 143 packed callbacks.
Neither fixture contains an original Ape Escape 3 executable or proves gameplay.

## New coverage added in this checkpoint

- New `tools/synthetic_pages_smoke.py` emits a deterministic original
  ELF32/MIPS32 test image with 143 indirectly called callbacks placed at
  0x00104000 + 0x300*i. They span **seven** different 16-KiB code regions.
- A guest pointer table at `0x00120000` dispatches each callback via real
  MIPS `LW` + `JALR`, with branch and call return delay slots exercised.
- Every callback increments a counter, stores and reloads it from a unique
  address `0x00013000 + 64*i`. These 143 scratch words span **three**
  distinct 4-KiB RAM pages. The dispatcher writes expected `0000008f`
  to `0x00012000` and calls genuine guest `Exit` syscall 4.
- New `tools/test_synthetic_pages_smoke.py` implements an independent,
  tiny instruction oracle for the whole fixture, including RAM page
  round trips and all 143 callback destinations.
- `tools/run_synthetic_aot_windows.ps1 -FixtureMode pages` recompiles,
  links and actually runs the fixture with the real upstream native Windows
  compiler/runtime, with the same timeout and trace assertions as previous
  fixtures.
- `.github/workflows/windows-fastlane-smoke.yml` now executes this additional
  end-to-end case and uploads its result/trace on failure.

The independent Python control-flow oracle was executed successfully in a
GitHub Windows CI run; its initial seven-region coverage assertion had an
outdated hardcoded callback stride and was corrected. **The expanded native
Windows pipeline has now passed end-to-end** in GitHub Actions run
[39318625918](https://github.com/Ukkitan0/Getchu-3-PC/actions/runs/39318625918).
The same run passed Win32 WebGPU compilation/linking. This proves execution of
original *synthetic* R5900 test code, not the retail game.

## Improvements for actual owner-local game attempts

- `RUN_BULK_NATIVE_WINDOWS.ps1` now discovers `VCPKG_ROOT` and
  `VCPKG_INSTALLATION_ROOT` or bootstraps a private vcpkg clone when neither
  is configured. It runs `vcpkg install sdl2:x64-windows` before native
  configuration, eliminating silent SDL2 dependency omissions.
- Missing ISO input now fails at preflight when native execution/exploration
  is requested, before lengthy compiler work.
- When the SDL2 shared DLL is installed in vcpkg, it is copied next to
  `ps2run.exe`, even if vcpkg app-local dependency copying is disabled.
- Existing SHA-256 retail-ELF validation, anchored Win32 overlay, private
  code generation, exploration loop and WGPU pin are retained. Original
  ISO/ELF and generated copyrighted C++ remain strictly owner-local.

## Native full-game milestone still missing

Run on the owner's Windows machine using their validated `SCUS_975.01`
and their ISO. The next required evidence is the actual native execution
trace, translated-code miss inventory, graphics behavior and first fully
rendered game frame. Unit-test and compiler CI success must not be described
as a game boot.

Example owner-local command:

```powershell
.\RUN_BULK_NATIVE_WINDOWS.bat -Elf "C:\path\SCUS_975.01" -Disc "C:\path\Ape Escape 3 (USA).iso" -ExploreRounds 8 -ExploreFrames 1800 -Run
```
