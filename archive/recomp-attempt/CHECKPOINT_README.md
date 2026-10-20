## Windows toolchain smoke now compiled a native runner (game-free stub)

GitHub CI at commit `b98c874` compiled the upstream real EE and VU compilers,
then built and linked native ClangCL/SDL2 `ps2runtime.lib`, `ps2game.lib` and
`ps2run.exe` on Windows Server 2022. That CI run returned failure from its
no-argument usage-exit check **after** the executable was built. This is not a
retail Ape Escape 3 execution. The runner only links empty registration stubs.
The usage-exit check was corrected, and a second CI compilation targeting the
verified official Win32 wgpu-native GPU path was added. Neither corrected
workflow's final result has been confirmed as passing.

## 2026-10-19: rebuild private opcode inventory from owned ELF (CI/public-repo fix)

A fresh public checkout omits retail code bytes by design. The native bulk
translator now derives executable instruction words and 12,228 raw JAL call
candidates from a **SHA-256-validated owner-provided ELF** into private output.
A synthetic ELF test verifies the automatic regeneration path, invalid ELF
rejection, and existing-inventory reuse. **28/28 offline Python tests pass.**
Initial Windows CI exposed the public missing-file assumption; the smoke workflow
has been restricted to tests requiring no retail ELF/derived opcode bytes.
A Windows game build has NOT been verified. See `CHECKPOINT_PRIVATE_ELF_RECONSTRUCTION.md`.

## 2026-10-19: native Windows runtime overlay + iterative AOT exploration (SOURCE ONLY, NOT PLAYABLE)

Reused MIT upstream `snuri00/ps2-web-recomp` at pinned `6526f48`, adding Win32 SDL2
HWND + WebGPU surface plumbing, MSVC GPU library linking, Win32 metrics, pin-verified
Windows wgpu-native v29.0.1.1 setup, mandatory window-enabled SDL2 build, ClangCL
toolchain, and bounded code-miss / VU1 exploration loops. Added an isolated CI smoke
workflow and 22 green Linux-side Python tests. **Windows runtime compilation and game
execution remain unverified.** See `CHECKPOINT_WINDOWS_NATIVE_ENGINE.md`.

# Getchu-3-PC — bulk native recompilation checkpoint

Date: October 19, 2026. This is source and Windows build orchestration,
**not** a validated playable game or a completed end-to-end native translation.

## Major new route

- Original-game full EE text inventory: 1,351,830 words loaded and validated.
- Discovered entry candidates: 12,228 distinct direct-JAL targets.
- All 143 constructor callback addresses recovered and seeded for translation.
- Default safe generated upstream entry list: 146 distinct addresses.
- Optional broader direct-call tiers: `common` (6,442 multi-site
  candidates) and `all` (all raw candidates; 12,373 seeds total).
- GitHub/MIT upstream `snuri00/ps2-web-recomp` pinned at
  `6526f4803d98e10de50bea3af104a4d26e9df033`.
- A new isolated, one-click native Windows CMake/MSVC build attempt, with
  exact original US retail executable SHA-256 verification. No WSL and no
  browser substitution.
- Eight Python tests passed; existing C boot diagnostic and its earlier tests
  remain available without being rewritten.

## Files to start with

`RUN_BULK_NATIVE_WINDOWS.bat` — native Windows build entry point.

`tools/bulk_recompile.py` — data-verified bulk entry generator and
external recompiler invocation.

`tools/test_bulk_recompile.py` — data/CLI unit tests.

`fastlane/README.md` — prerequisites and limitation disclosure.

`CHECKPOINT_BULK_RECOMP.md` — tested scope and next milestones.

**Game ELF/ISO, Sony BIOS, built copyrighted game code and gameplay assets
are NOT shipped.** No title screen, graphics or playable build has been
validated in this new native route. The previous incremental C boot
execution still ends at the inner second constructor `0x00423E20` call.