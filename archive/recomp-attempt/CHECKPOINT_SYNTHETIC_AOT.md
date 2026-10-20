# Native Windows EE AOT execution checkpoint — 2026-10-19

**Goal:** Establish a game-data-free, end-to-end *execution* validation path for
`snuri00/ps2-web-recomp` on Windows: MIPS ELF => eerecomp generated C++ =>
ClangCL ps2run.exe => original MIPS JAL/JALR, delay slots, memory and Exit.
This closes the gap between a Windows `ps2run.exe` that merely links and a
Windows executable that actually runs recompiled R5900 code.

## Implemented

- `tools/synthetic_aot_smoke.py` builds a deterministic ELF32 little-endian
  `EM_MIPS` test program from original, game-free instruction constants.
  No Ape Escape 3 ISO, retail ELF, BIOS or generated commercial data is used.
- Synthetic program starts at `0x00100100`, uses direct `JAL` to `0x00100140`,
  then indirect `JALR` to a separate seeded callback at `0x00100160`,
  returns with accurate delay slots, computes `0x1234`, writes it at RAM
  `0x00012000`, then performs genuine PS2 kernel `Exit` syscall `0x04`.
- Explicit seed list `00100160` proves that the compiled runtime can resolve
  dynamically referenced code, directly relevant to 143 AE3 constructors.
- `tools/run_synthetic_aot_windows.ps1` automates the AOT translation, ClangCL
  runtime rebuild, bounded execution, guest-memory and Exit assertions. 30 s
  timeout prevents indefinitely hung CI.
- New CI step uses the **already compiled** upstream eerecomp and runtime build
  directory; after this game-free test the existing GPU compiler check remains.
- The emitted JSON/trace artifacts record game-free runtime evidence.

## Verified locally

- Eleven standalone Python fixture, contract and independent MIPS control-flow
  tests passed on Linux (not an actual Windows compiler/runtime execution).
- Original ELF fixture identified by `file` as ELF32 little-endian MIPS.
- Do **not** claim the Windows AOT smoke has passed until GitHub Actions reports
  success. The complete private Ape Escape 3 native boot and 3D rendering remain
  unverified, and a playable Windows game has not been produced.

## How to run on a prepared Windows native toolchain

```powershell
pwsh -File tools/run_synthetic_aot_windows.ps1 `
  -Upstream 'C:\path\ps2-web-recomp' `
  -Compiler 'C:\path\tools-build\Release\eerecomp.exe' `
  -RuntimeBuild 'C:\path\runtime-build' `
  -VcpkgToolchain 'C:\vcpkg\scripts\buildsystems\vcpkg.cmake'
```

Successful evidence: `aot-smoke-result.json` reports `PASS`; stderr trace
contains `[peek] 00012000: 00001234` and an `[ee] stopped ...: Exit` line.
Any other result is a failure and should be preserved for diagnosis.

This is a native recompilation fixture, **not a commercial game test**.

## Added 143-callback stress fixture

`tools/synthetic_ctors_smoke.py` synthesizes **143 different** MIPS R5900
callbacks at code addresses `0x00100200` onward and a 144-word pointer table
terminated by zero. Each callback increments a guest register and returns via
`JR $ra`. The real MIPS dispatcher loops through all 143 original test
addresses with `LW`, conditional branch, delay slot and indirect `JALR`.
At the end it stores `143` (`0000008F`) into RAM `0x00012000` and calls the
original test syscall `Exit` (`0x04`). The explicit generated `--entries` list
contains all 143 callback addresses.

The mode is selected using `-FixtureMode constructors` in
`tools/run_synthetic_aot_windows.ps1`; `-FixtureMode calls` remains the
smaller default. It compiles and executes the code with the exact same Win32
CMake build and runtime as the smaller proven smoke test.

The 17 standalone Python tests for both fixtures passed locally, and
`llvm-objdump` independently decoded the MIPS code and branch destinations.
A successful Windows CI run is required before claiming the stress dispatcher
actually executes in the native runtime; it is **not** a claim that 143
commercial Ape Escape constructors have been executed.