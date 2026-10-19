# Getchu-3-PC direct-JAL checkpoint — 2026-10-18

This checkpoint implements real MIPS direct function calls across translated native-C code. It is **not a playable game or general PS2 emulator**.

## Contents

- `call_lift.py`: conservative caller discovery and native-C call scheduler generator.
- `test_calls.py`: independent original-instruction-vs-C execution differential tests.
- `ae3_native_dispatch.c/.h`: new direct-call entrypoints, while retaining all existing leaf interfaces.
- `BUILD_AND_TEST_LINUX.sh` and `BUILD_WINDOWS_OBJECTS.bat`: full generation, regression and Windows object compilation.
- `ae3_call_leaves.c/.h`, `ae3_call_leaves_windows.obj`, `libae3_calls.so`: generated build products (local ZIP only, deliberately omitted from public GitHub).

## Verified

30 disjoint caller candidates, 35 direct JAL edges and 550 new original instruction sites. Together with the previous 1,470 candidate leaves, the scheduler now recognizes 1,500 candidate entry addresses.

- 90 caller execution trials, 1,167 native-vs-original instruction-slice comparisons passed.
- 81 executions reached normal return; 6 memory accesses faulted as expected; 3 did not terminate within the test limit and were left explicitly unfinished.
- Verified a stack-save memory fault can be retried with the same saved PC after mapping memory.
- Common C dispatcher integration passes.
- Windows x64 COFF compilation verified using clang-cl. No game executable generated.

## Reproduce

Use your legally obtained NTSC-U game `SCUS_975.01` with SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

Linux: `AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh`.
Windows: set `AE3_ELF` to the retail executable and run `BUILD_WINDOWS_OBJECTS.bat` with Python and LLVM/Clang.

Original ISO/ELF and translated machine-code-derived game C are not committed to the public repository; the local ZIP includes generated C for the user's own analysis.

The remaining game port blockers include indirect calls, nested caller graphs, full EE semantics and VU/GS/IOP/BIOS/disc/audio/runtime implementations.