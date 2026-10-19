# Native whole-game resume safety and PPM graphics evidence

**Date:** 2026-10-19. **Target:** native Ape Escape 3 on Windows.
**Status:** Windows CI has passed the new source-only regressions; no retail
game was booted in CI and no gameplay is certified.

## Major fixes

### Protect native AOT results on every restart

`tools/aot_cache.py` version 2 hashes each compiled-game translation input
(compiler executable, owner-owned boot ELF, discovered entries and optional
SNR2/code-window images) and **each generated C++ translation unit plus
`units.hpp`**. This invalidates cache hits if source files were corrupted,
renamed, omitted or replaced with other source of equal file count. A
successful translation without complete upstream `table.cpp` and `units.hpp`
registration outputs is rejected; the new validated manifest is only written
after translation succeeds. The source-only Windows tests verify these cases.

Prior checkpoints already ensured original EE constructor plus runtime-discovered
function entries survived a new build, and preserved an authenticated private
instruction inventory from the retail ELF.

### Capture and assess *real* upstream native frame format

The pinned `ps2-web-recomp` runtime emits `frame_XXXXX.ppm` **P6 RGB**
files under `--frames-dir` (not PNG by default). Prior boot diagnostics
ignored those PPM files and could report zero screenshots despite rendering.
The `tools/frame_probe.py` reader now checks P6 dimensions/length, samples
up to 4096 pixels per frame, counts near-black and bright pixels and distinguishes
mostly dark, nearly uniform and varied captures. It emits statistics only,
never stores screenshot pixels in reports.

`tools/boot_report.py` includes frame evidence and prints the last
three frame classifications. **Varied pixels only show an image was rendered;
they do not prove 3D scene geometry, a functioning title menu, player
control, correct GS/VU fidelity, or playable gameplay.**

### Isolate attempts so old screenshots do not create false progress

`RUN_BULK_NATIVE_WINDOWS.ps1` now uses unique
`fastlane/private/shots/session-.../round_XX/` folders for each actual
headless attempt, with a new frame destination for every recompilation round.
The current set is recorded in `explore_summary.json`.
`boot_report` inspects only those directories, and safe diagnostic ZIPs
omit private shot paths. Old captures remain available for independent
comparison and are not destroyed.

## Verification

The Windows CI execution history is visible in
[GitHub Actions](https://github.com/Ukkitan0/Getchu-3-PC/actions).
The generated-source SHA validation and initial PPM support completed green
full workflows. The latest per-session screenshot handling is covered by
Windows Python regression tests; check the most recent run for the complete
ClangCL/WebGPU final result.

The upstream PS2 recompilation project's first public playable titles are
Gran Turismo 4 and God of War II; this source-only pipeline is being
adapted to Ape Escape 3 but has **not** passed the equivalent retail boot.

## How to evaluate a real local game attempt

Run `TEST_APE_ESCAPE_3_WINDOWS.bat` with a US ISO from your own game disc.
The test generates the diagnostic metadata under `fastlane/private`, and
PPM screen captures, if the guest reaches displayable frames, under the new
`shots/session-.../round_XX/` paths.

Inspect:
- `fastlane/private/boot_report.json` — frames reached, guest halt reason,
  source-only screenshot classifications, EE/IOP/GPU error counts
- `fastlane/private/explore_summary.json` — actual code discovered each round
- `fastlane/private/shots/session-.../round_XX/` — original screen evidence
- `fastlane/private/AE3_NATIVE_DIAGNOSTICS.zip` — sanitized JSON/log text
  if a build fails; this omits original game images/code/ISO/RAM

The next correctness milestone is an owner-local build of the retail EE code,
a boot trace containing gameplay frames, and **visually verified actual
level geometry**; neither synthetic AOT smoke tests nor color statistics
can substitute for that.
