# Native call-stack and virtual dispatch checkpoint — 2026-10-18

**This is not a playable native Windows build.** The canonical native C source-recovery project remains `Getchu-3-PC`; the original retail PS2 ELF and ISO are excluded from GitHub and all public releases.

## Verified addition

- `stack_lift.py` generates native C for 62 distinct disjoint scalar caller candidates: 30 existing straight-line JAL callers, 1 additional real nested JAL caller (`0x003B40F8`), and 31 caller candidates using `JALR` through a runtime function pointer.
- The new caller candidates contribute **390 translated instruction sites** beyond the previous 9,940 sites; with 1,470 existing resumable leaf candidates there are **1,532 candidate entrypoints** in total. These are candidate coverage counts, not proof of complete game/function coverage.
- The bounded native call stack stores each caller's original `pc`, link and pending delay-slot state, and preserves it during nested dispatch, instruction-budget yields and memory faults. Maximum depth is 8, with an explicit error on overflow.
- `JALR` reads its MIPS target source *before* writing the guest link register `$ra`, rejects non-canonical 64-bit pointers, and only dispatches to an exact recovered leaf/caller entry. Unsupported virtual function addresses fail explicitly; nothing falls back to arbitrary execution or a NOP.
- Original executable SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Tests run in this environment

- `python3 test_stack.py 3`: passed **1,656** instruction-slice comparisons in **186** caller trials (81 completed, 102 memory faults, 1,473 yielded slices). Nested stack depth 2 exercised on original retail code and 42 checkpoints covered pending JAL delay slots.
- Dedicated real `JALR` wrapper test (`0x0010CD14`) completed with a function pointer loaded from guest RAM and a recovered leaf target, including `$ra` save/restore. Deliberately unknown target `0xDEADBEEF` was rejected with `AE3_RESUME_UNKNOWN` without executing the delay slot.
- Native combined dispatcher integration (`ae3_dispatch_test`) passed against the legacy direct-call path.
- `ae3_stack_leaves.c` and `ae3_native_dispatch.c` compiled to Windows x64 COFF objects using `clang-cl` and also compiled/linked natively using GNU C11.
- Prior scalar, forward-branch, loop, resumable and direct-JAL suites passed in the broader script. The full all-in-one script did not complete before the command timeout; the new stack tests and integration were validated separately.

## How to reproduce

1. Place your own original NTSC-U `SCUS_975.01` in this folder or export `AE3_ELF` to its local absolute path.
2. Run `python3 bulk_lift.py`, then `cfg_lift.py`, `loop_lift.py`, `resume_lift.py`, `call_lift.py`, `stack_lift.py` in order.
3. On Linux, build `libae3_resumable.so` and `libae3_stack.so` using `BUILD_AND_TEST_LINUX.sh`; execute `python3 test_stack.py 3`.
4. On Windows with LLVM clang-cl and Python, run `BUILD_WINDOWS_OBJECTS.bat` to produce **object files**, not an executable game.

## Remaining blockers to a playable Windows port

General dynamic targets and their unresolved dependencies, EE GPR high halves/MMI/COP1/VU instructions, real guest threads/BIOS and IOP services, disc file system, audio, VIF/GIF/GS rendering and gameplay sequencing. This candidate subset by itself cannot launch Ape Escape 3.