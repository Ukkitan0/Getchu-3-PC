# Kernel boundary + first real game initialization — 2026-10-18

**State: still not a native playable game.** The recovered original retail ELF has SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## This checkpoint

The original retail boot previously stopped at PC `0x001002FC` with syscall number `0x3C` (`SetupThread`). This checkpoint implements three native-C components:

- `ae3_boot_kernel_bridge.c/.h` recognizes and captures the *guest-generated* arguments to syscall `0x3C` and `0x3D` (`SetupHeap`). Without an authoritative kernel response it blocks; it never fabricates a stack pointer or marks the syscall finished. Once given a return value from the kernel/BIOS adapter, it resumes the original instructions `0x00100300`–`0x00100318`, including the guest's `move $sp,$v0`, and stops at the first initialization `JAL` at `0x0010031C`. A checked `SetupHeap` acknowledgement records guest heap bounds but is **not** a real heap manager.
- `ae3_boot_init_prefix.c/.h` executes the *actual* call from `0x0010031C` into `0x0043FE58`, its delay slot, the callee's `addiu $sp,-16`, and an `sd $ra,0($sp)` stack save. It stops before the first nested `JAL` at `0x0043FE60 -> 0x0043FC08`.
- `ae3_boot_probe.c` is a **non-game CLI diagnostic**. Default mode executes the real boot up to the pending syscall and prints the real arguments. `--test-kernel-fixture` injects a **clearly artificial** 16-byte-aligned stack return (`0x01FFFFF0`) solely to verify the *subsequent original userland instructions*. That fixture is **not a verified PS2 BIOS stack address**. Default mode never performs the injection.

Actual retail arguments checked by tests:

| Kernel service | Address | Guest arguments |
|---|---|---|
| SetupThread 0x3C | `0x001002FC` | GP `0x00734070`, stack `0x01FA0000`, size `0x00060000`, args `0x0072D6C0`, root `0x00100388` |
| SetupHeap 0x3D | `0x00100318` | heap start `0x00772910`, requested size `0x00010000` |

The prior boot chain retired 315,897 original MIPS instructions to SetupThread. With the *test-only kernel response*, 8 original instructions (2 syscalls, 6 normal) execute through the second syscall, then 4 real instructions complete the call and the callee's prologue. Total **315,909** guest instructions to the first nested call **only in the test fixture**. These numbers are instruction retirement, not function coverage or playable-game completion.

## Verification

- Original ELF loader and prior bounded boot prefix, memory fill and thread prologue run without modification.
- `test_boot_kernel_bridge.c`: validates syscall numbers, all guest argument registers, bounded instruction slices, transactional invalid service responses, memory faults, and snapshot equivalence between one-shot and sliced execution.
- `test_boot_init_prefix.c`: verifies real JAL target and delay slot, 64-bit stack save bytes, saved return address, and a recoverable missing-memory fault.
- Both tests pass with `clang -fsanitize=address,undefined` and `-Wall -Wextra -Werror`.
- The two new runtime modules cross-compile to x64 COFF objects with `clang-cl /W4 /WX`; local Windows MSVC CRT is not available to link/run a Windows executable here.
- To run the *Linux boot CLI*: `./build/ae3_boot_probe /path/to/SCUS_975.01`. To build the *Windows CLI* (not a game), use `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` under a Visual Studio developer environment.

## Remaining hard blockers

A PS2 EE BIOS/kernel adapter must provide the **actual** SetupThread return value and authoritative SetupHeap behavior, not the regression fixture. The next real nested initialization callee at `0x0043FC08` invokes `0x00435AE0` (thread/kernel initialization). Broad R5900 instruction translation, interrupts, EE/IOP, VU/GIF/GS, disc, audio, savedata and graphics remain incomplete. This checkpoint is not a complete decompilation or a testable gameplay build.