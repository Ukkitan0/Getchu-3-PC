# Checkpoint — native full-game batch translation strategy

Date: 2026-10-19. Canonical branch before this checkpoint:
`569404ac96ef791fe41d6aad93320e1d9960b9d9`.

## What actually changed

Stopped manually adding a new C micro-interpreter per retail call. Built and
executed a **whole-game entry-seeding tool** grounded in the existing recovered
US retail EE instruction inventory. It scans 1,351,830 word addresses, loads
12,228 distinct raw direct-JAL target candidates and all 143 constructor
callbacks, and checks that every generated address falls inside the original
instruction inventory. It generates deterministic, deduplicated hexadecimal
entry files that match the entry grammar of upstream `eerecomp`.

The default seed set is **146 entry points**, including every constructor and
five original startup anchors (two overlap). The upstream static analyzer
follows ordinary direct calls. More speculative modes generate thousands of
entries: `common` uses 6,442 multi-site call candidates plus constructors;
`all` uses every recovered candidate, producing **12,373** unique seeds.
The latter contains potential data/code ambiguities and is opt-in.

Added native Windows build orchestration that pins MIT-licensed upstream
`snuri00/ps2-web-recomp` to commit `6526f4803d98e10de50bea3af104a4d26e9df033`,
validates the retail game ELF SHA-256, builds upstream `eerecomp`, translates
owner-provided executable to C++ in one operation, and attempts to link the
upstream `ps2run.exe` as a native Windows x64 program. It uses its own private
workspace and does not import/copy the older recomp project's modified code.
No website, cloud ISO upload, WSL or emulator frontend is substituted.

## Grounding

- Upstream `recomp/main.cpp` `read_entries()` reads whitespace-separated hex
  addresses and supports `eerecomp <elf> --entries <path> --out <dir>`.
- Upstream `CMakeLists.txt` declares `eerecomp`, `ps2runtime`, `ps2game`
  when `PS2_GENERATED` is supplied, and native `ps2run`.
- Upstream `host/main.cpp` declares `ps2run <elf> --disc <iso> --window`,
  where `--window` depends on SDL2 detection.
- Upstream release is MIT licensed; its source is fetched at build time,
  not included here. User must own their own retail disc and executable.

## Tests actually run in this environment

- `python tools/bulk_recompile.py --out fastlane/generated` succeeded.
- `python tools/test_bulk_recompile.py` passed 8 unit/integration tests.
- Tests confirm 1,351,830 recovered instructions, 143 constructor callbacks,
  sorted unique output, default and common/all seed profiles, bad retail ELF
  hash rejection, malformed constructor rejection, and tool CLI argument wiring.
- Entire native game has **not** been translated in this environment because
  the authentic user-owned retail ELF is not available here.
- Windows PowerShell and Visual Studio builds have **not** been executed here.
  The upstream CMake Windows native runner may require portability fixes,
  SDL2 and a Windows-compatible GPU library. Do not claim the script is
  a working game until that has been tested on the owner's Windows machine.
- All previous original-C boot diagnostic modules remain unmodified.

## Next required *measurable* milestone

Run `RUN_BULK_NATIVE_WINDOWS.bat -Elf "...\\SCUS_975.01"` on Windows to
produce translation build logs. Fix all Windows build/runtime incompatibilities
in the shared backend, then enter a real game frame with native drawing and
input. After that, use stage screenshots, IOP trace and VU/GS workload captures
to address broken scene geometry. Do **not** resume one-function-at-a-time
constructor translation unless a differential test specifically requires it.