# Getchu-3-PC — Ape Escape 3 native decompilation

**Canonical project:** original-ELF-first translation of *Ape Escape 3* (PS2, NTSC-U, `SCUS_975.01`) into tested native C, followed by Windows game/runtime work. Previous experimental recompilation trees are **reference only**.

**Current status: native source-recovery tools, NOT a playable Windows game.** The project does not yet boot, render a world, play audio, or run game logic from its own Windows executable.

## Translation milestones (October 18, 2026)

| Translator | Candidate routines | Unique original instruction locations | Verified test executions |
| --- | ---: | ---: | ---: |
| `bulk_lift.py` straight-line scalar leaves | 1,163 | 4,067 | 9,304 |
| `cfg_lift.py` forward control flow | 168 | 1,998 | 1,680 |
| `loop_lift.py` backward branches, bounded loops | 139 | 2,162 | 1,112 in baseline; 3,336 in extended tests |
| **Unified native dispatcher** | **1,470** | **8,227** | Unique entry addresses checked |

These are **candidate routines**, not guaranteed original function boundaries or whole-game coverage. The MIPS-word reference interpreter tests can catch implementation mistakes but do not replace real PS2 execution validation. The translator models *only low 64 bits* of EE general-purpose registers and a restricted scalar instruction subset. Every unsupported instruction causes rejection, not silent skipping.

## From your own retail game

The repository deliberately excludes the ISO, ELF, extracted game assets, and generated source derived from copyrighted game bytes. Supply your own retail `SCUS_975.01` with SHA-256:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

Use Python 3.10+ and a C11 toolchain. Set `AE3_ELF` to your copy of `SCUS_975.01` or place that file in the repository root.

On Linux:

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
```

This generates local C source from the ELF, builds three C shared test libraries and a unified dispatcher test, and runs three MIPS-word comparison suites. *These generated C files are intentionally git-ignored.*

On Windows, install Python 3 and LLVM/Visual Studio C++ Clang tools and run `BUILD_WINDOWS_OBJECTS.bat`. This builds four **Windows x64 object files**, not an executable game.

## Execution status

The loop translator supports short backward conditional branches and direct jumps, with correct MIPS delay-slot ordering and an instruction budget of 4,096. Return code `-3` explicitly means **execution budget exhausted**; the routine has not finished and the modified register/memory state is incomplete. It is not safe to treat `-3` as success or resume without tracking the guest program counter and state.

## Not implemented

64-bit high-half EE GPR lanes, 128-bit MMI, COP1/VU, indirect control flow, mid-function calls, multiple-return shapes, system services, IOP boot, GS/VIF/GIF, audio, disc streaming, controller input, and save-card behavior. A real PC port requires integration and tests for all of these, as well as auditing candidate function boundaries.

The **next milestone** is an explicit resumable guest-context/PC model for loops and indirect dispatch, then integration of native GS/VU rendering and IOP services. No placeholder or emulator wrapper is represented here as a native port.

## Incremental native scheduler (2026-10-18)

The canonical source generator `resume_lift.py` produces `ae3_resumable_leaves.c/h`
from the verified USA retail ELF. It exposes **1,470 candidate scalar routines**
(1,163 straight-line, 168 forward-control-flow, 139 bounded loops) through a
PC-resumable instruction-slice interface. Program counter, pending delay-slot
branch/return decisions, and retired instruction count are persistent in
`AE3_ResumeFrame`. The public native dispatcher exposes:

```c
AE3_ResumeFrame frame;
ae3_native_frame_init(&frame, entry_pc);
int result = ae3_run_native_slice(&context, &frame, budget);
/* AE3_RESUME_YIELDED (1) => call again with same frame and guest context.
   AE3_RESUME_DONE (0) => routine returned.
   AE3_RESUME_MEMORY_FAULT (-1) => memory mapping needed, retry same PC.
   AE3_RESUME_UNKNOWN (-2) => unsupported entry.
   AE3_RESUME_INVALID (-4) => invalid or inconsistent frame. */
```

Run `AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh` to regenerate
translated code, build the shared libraries, run all differential suites, and
run the C dispatcher integration test. On Windows, run `BUILD_WINDOWS_OBJECTS.bat`
with VS/Clang and Python available; this compiles five native COFF objects.
No Windows game EXE or full PS2 runtime exists yet.

The `ae3_resumable_leaves.c/h` files are **derived from the copyrighted game
executable**, generated locally, and excluded from this public repository.

*Known limitations:* R5900 EE high 64-bit GPR lanes, MMI, COP1/COP2, VU,
indirect calls and returns, external IOP/BIOS/SIF devices, sound, GS/GIF/VIF,
level assets and a complete playable runtime remain unimplemented. Candidate
instruction addresses can still be data rather than verified code boundaries.