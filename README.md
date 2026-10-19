# Getchu-3-PC — Ape Escape 3 native PC decompilation

**Canonical source project.** Target: a native Windows port of the NTSC-U retail game (*Ape Escape 3*, `SCUS_975.01`). The old recompilation experiments are references only. **This source is not a playable Windows game.** No original game executable or assets are distributed in this repository.

## Latest verified checkpoint — October 18, 2026

| Layer | Recovered candidate entrypoints | Verified behavior |
|---|---:|---|
| Scalar/branch/loop leaves | 1,470 | Native C, instruction-sliced execution, original-MIPS comparisons |
| Callers with direct/indirect calls and loops | 176 | Bounded nested JAL/JALR stack, delay slots, checked indirect targets |
| **Unified guest execution interface** | **1,646** | Routes to appropriate executor; preserves PC, registers, RAM, and fault-retry state |
| **EE ELF image loader** | 1 retail PT_LOAD segment | Maps executable byte-for-byte and clears BSS in 32 MB guest RAM |

**Expanded retail startup execution (verified):** The bounded native C boot chain now executes the initial 143,836 original instructions, enters the first actual `JAL` at `0x001002C4`, runs its original stack-pattern fill routine at `0x001036D8` (24,576 additional 128-bit stores across 384 KiB), returns to `0x001002CC`, and continues to the real `SetupThread` syscall at `0x001002FC`. In total the chain retires **315,897 guest instructions** without fabricating a kernel return value. The `SetupThread` syscall itself has not been implemented. See [CHECKPOINT_BOOT_EXPANDED.md](CHECKPOINT_BOOT_EXPANDED.md).

**Full ELF section inventory:** `full_elf_inventory.py` now exports **1,351,830 EE `.text` words** and **3,200 VU 64-bit packets**, including raw words that LLVM cannot decode. This is exhaustive *binary indexing*, not full game decompilation. The generated disassembly is local/private and excluded from GitHub.

The guest execution adapter and ELF loader are written in portable C11 (`ae3_guest_execution.c/.h`, `ae3_elf_image.c/.h`). They compile as Windows x64 COFF objects with `clang-cl`, without requiring copyrighted game code in the repository. The Linux integration tests passed using the verified retail executable. The original execution entry at `0x00100134` is not in the general translated-function adapter; a separate bounded startup runner reaches the first kernel syscall, but system services and complete game startup are still unsupported.

These 1,646 entries are conservative **candidate** functions, not verified complete-game coverage. We still lack EE full 128-bit/MMI/COP1/VU instructions, sufficient caller/function coverage, EE/IOP/BIOS services, PS2 graphics, audio and disc/game runtime integration.

## Current retail init: EE semaphore and syscall hook boundary (2026-10-18)

**New tested advancement:** a bounded native C interpreter now executes the original `0x0043FC08` initialization routine and both `CreateSema` (`0x40`) syscall wrappers, preserves delay slots and guest stack/register memory, and continues into the next initializer until the actual `SetSyscall` (`0x74`) boundary at `0x0043FE4C`. The guest registers contain a request to register handler `0x0043FCC0` for syscall `0x83`; the real kernel registration is not implemented.

An **opt-in experimental semaphore compatibility kernel** supports 256 guest-visible objects with create, signal and poll behavior. Without a real kernel response, the native guest runner still yields at the original syscall boundary; the boot diagnostic uses clearly labelled synthetic setup and HLE responses only when explicitly requested. Source and tests: `ae3_boot_semaphore_init.[ch]`, `ae3_ee_sema_kernel.[ch]`, `ae3_boot_hook_init.[ch]`, and [CHECKPOINT_SEMA_HOOK.md](CHECKPOINT_SEMA_HOOK.md). The three new C modules compile to Windows x64 COFF objects. **No playable game executable yet.**

## Latest kernel-to-initialization checkpoint (2026-10-18)

The boot code now has an **explicit kernel-service boundary interface** for the real `SetupThread` (`0x001002FC`) and `SetupHeap` (`0x00100318`) syscalls. With a **test-only** stack response, our native C executes the original follow-up opcodes, enters the first initialization function at `0x0043FE58`, saves the true return address on the guest stack, and stops at its first nested call to `0x0043FC08`. Original-instruction behavior and recoverable memory faults are tested; the actual EE kernel implementation has NOT yet been connected. See [CHECKPOINT_KERNEL_BRIDGE.md](CHECKPOINT_KERNEL_BRIDGE.md).

A non-game diagnostic `ae3_boot_probe.c` now works on Linux; Windows users with Visual Studio/Clang tools can compile it using `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat`. Default mode **does not** fabricate kernel responses. No playable game executable exists.

## Rebuild and verify

Use an unmodified `SCUS_975.01` from your own copy of the NTSC-U game. Required SHA-256:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

The Python generators enforce that SHA. Set the path before running them.

**Linux (Python 3.10+ and a C11 compiler)**

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
```

This generates private C from the user's own executable, compiles the recovered code, runs independent MIPS differential tests, maps the original ELF to EE guest RAM, then validates the common execution adapter. A full from-scratch rebuild can take several minutes.

**Windows x64 (Python 3, LLVM `clang-cl` / MSVC C++ tools)**

```bat
set AE3_ELF=C:\ApeEscape3\SCUS_975.01
BUILD_WINDOWS_OBJECTS.bat
```

Windows outputs are object files to link into a later native game runtime, **not a runnable game executable**.

## Architecture

- `bulk_lift.py`, `cfg_lift.py`, `loop_lift.py`: conservative leaf recovery; low 64-bit scalar instructions.
- `resume_lift.py`: independent instruction slices with saved PC and MIPS delay state.
- `call_lift.py`, `stack_lift.py`, `extend_stack.py`, `loop_stack.py`: call/return, conditionals, backward edges and whitelisted indirect targets.
- `ae3_native_dispatch.c/h`: legacy per-kind dispatch; `ae3_guest_execution.c/h`: unified host-facing dispatch with a single typed frame.
- `ae3_elf_image.c/h`: checked, transactional ELF32 little-endian MIPS guest-memory mapping.
- `audit_boot_entry.py`: statically validates the reset instructions.
- `ae3_boot_prefix.c/.h`: the actual guest entrypoint register/FPR reset and BSS clear.
- `ae3_boot_memory_fill.c/.h`: the actual first `JAL` callee, copying a 128-bit pattern over 384 KiB of EE stack RAM.
- `ae3_boot_thread_setup.c/.h`: the real next register/setup instructions stopping at `SetupThread` syscall, with no fabricated kernel response.
- `full_elf_inventory.py`: exhaustive `.text` word inventory and separately indexed VU microcode packets (not full C decompilation).
- `test_boot_prefix.c`, `test_boot_memory_fill.c`, `test_boot_thread_setup.c`: sliced state, memory faults, original register arguments, and UBSan/ASan regression checks.
- `test_guest_execution.c`, `test_elf_image.c`: original-code integration tests; `test_*.py`: differential comparisons with independent MIPS decoding.

The generated C, ELF/ISO, disassembly and game assets stay out of public Git history. See [STATUS.md](STATUS.md) and [CHECKPOINT_GUEST_RUNTIME.md](CHECKPOINT_GUEST_RUNTIME.md) for exact validation and blockers.