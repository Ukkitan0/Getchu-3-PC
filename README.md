# Getchu-3-PC — Ape Escape 3 native Windows decompilation

**Active, canonical decompilation project. Experimental. NOT PLAYABLE.**

Goal: reconstruct the US PS2 retail executable `SCUS_975.01` into native C,
then develop a Windows game runtime with genuine EE/IOP services, graphics,
input, sound and saves. Older recompilation efforts are reference material only.
This public repository contains recovery tools, original implementations,
and tests, **not** the copyrighted ISO, ELF, BIOS, ROM or game assets.

## Latest native boot checkpoint — EE Timer 3 and kernel copy

- An optional EE Timer 3 MMIO device handles `0x10001810` and the game's
  genuine mode-bit branch. Disabled CMPE leads to another SetSyscall call;
  enabled CMPE returns to the original caller.
- The original `SetSyscall` / guest syscall `0x5A` wrappers are followed
  through a **test-only compatibility kernel**. The recovered original MIPS
  handler copies the embedded **1,856-byte** game kernel payload from
  `0x006A2678` to `0x80076000`, verified byte-for-byte in shared EE RAM.
- The opt-in diagnostic reaches `0x00440398`, or 321,484 retired guest
  instructions on that controlled path. **Without simulated kernel startup
  services the real guest boot still stops at `SetupThread`.**
- `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` with `AE3_BOOT_TEST_TIMER3_COPY=1`
  selects this experimental check. It is NOT a playable Windows port.

See [CHECKPOINT_TIMER3_KERNEL_PATCH.md](CHECKPOINT_TIMER3_KERNEL_PATCH.md).

## Verified checkpoint — 2026-10-19

- Original retail ELF: SHA-256
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- Recovered **1,646 conservative candidate entrypoints**, not complete-game C.
  The executable's entire `.text` is indexed, but much of it is not translated.
- The real retail startup executes 315,897 instructions up to `SetupThread`
  **without** test-only kernel behavior. A bounded native-C executor preserves
  the original 128-bit GPR state, stack memory, delay slots and guest PC.
- Explicit opt-in compatibility-kernel tests continue through `SetupHeap`,
  `CreateSema`, `SetSyscall`, and the original registered `FindAddress` handler.
- **New:** `ae3_boot_post_find.[ch]` runs the real comparison/backward-loop code
  at `0x0043FDC4`, including further calls to the original guest handler.
  Without authentic BIOS data it requests a **third** syscall `0x83` at
  `0x0043FD04`, not a fabricated success.
- **New:** with *clearly synthetic, matching kernel search entries*, the native
  runner returns to the original next initialization JAL at `0x0043FE70`,
  enters `0x00440338`, and executes the register/stack prologue. It stops
  **before** its original `LW` from `0x10001810` at `0x00440354`. That register
  is **EE Timer 3 MODE**, confirmed by [PS2SDK EE register definitions](https://github.com/ps2dev/ps2sdk/blob/master/common/include/ee_regs.h).
- Original-instruction unit tests, one-step-vs-batched state parity, fault
  recovery, Clang ASan/UBSan and Windows x64 **object** compilation pass.
  **No Windows executable or runtime graphics is claimed.**

See [CHECKPOINT_POST_FIND_TIMER3.md](CHECKPOINT_POST_FIND_TIMER3.md) and
[CURRENT_BUILD_STATUS.md](CURRENT_BUILD_STATUS.md) for exact evidence and limits.

## Try the Windows boot diagnostic (not a game)

In a VS 2022 Developer Command Prompt with Clang tools, set `AE3_ELF` to your
**own** NTSC-U retail `SCUS_975.01` and use:

```bat
set AE3_ELF=C:\ApeEscape3\SCUS_975.01
BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat
```

By default the diagnostic stops at genuine `SetupThread` and does not invent
BIOS state. For the expressly artificial extended trace:

```bat
set AE3_BOOT_NEXT_MMIO=1
BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat
```

This optional mode uses a documented stack-top compatibility policy,
experimental semaphores, and two **synthetic** words in kernel RAM. It stops
at the original Timer 3 hardware read; it never launches gameplay.

## Linux regression checks

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
```

The full generated-C suite can be slow. New focused tests:

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c \
  ae3_ee_memory_bus.c ae3_guest_find_address.c ae3_boot_post_find.c \
  test_boot_post_find.c -o test_postfind
./test_postfind "$AE3_ELF"

cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c \
  ae3_ee_memory_bus.c ae3_guest_find_address.c ae3_boot_post_find.c \
  ae3_boot_next_init.c test_boot_next_init.c -o test_next
./test_next "$AE3_ELF"
```

To run the two different modelled boot paths after building `build/ae3_boot_probe`:

```bash
build/ae3_boot_probe "$AE3_ELF" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-post-find
build/ae3_boot_probe "$AE3_ELF" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-next-mmio
```

## Implementation structure

- `bulk_lift.py`, `cfg_lift.py`, `loop_lift.py`, `resume_lift.py`,
  `call_lift.py`, `stack_lift.py`: bounded binary-to-C recovery.
- `ae3_elf_image.[ch]`: checked retail ELF32 to 32 MiB EE memory loader.
- `ae3_ee_memory_bus.[ch]`: KUSEG/KSEG0/KSEG1 physical RAM aliases;
  unknown hardware addresses cause faults, not fake responses.
- `ae3_boot_prefix.[ch]`, `ae3_boot_memory_fill.[ch]`: true 128-bit reset,
  BSS clearing and original first initialization fill.
- `ae3_ee_boot_services.[ch]`: test-only, guest-derived initial stack/heap
  model — **not Sony BIOS**.
- `ae3_boot_semaphore_init.[ch]`, `ae3_boot_hook_init.[ch]`,
  `ae3_guest_find_address.[ch]`: real guest CreateSema/SetSyscall wrappers
  and original MIPS address-search handler with experimental kernel support.
- `ae3_boot_post_find.[ch]`: search reconciliation and repeated caller path.
- `ae3_boot_next_init.[ch]`: original next callee through EE Timer 3 MODE read.
- `ae3_boot_probe.c`: transparent boot diagnostic; default never invents
  the BIOS state.

## Remaining work before a playable Windows port

Faithful EE kernel/BIOS startup, timing-sensitive Timer 3 registers and
interrupts, broader main-game control-flow and R5900 instruction translation,
IOP and SIF services, VU/GS rendering, game asset/disc access, controller
integration, audio and save behavior. A successful isolated function test or
cross-compiled `.obj` file does **not** establish that the game boots or plays.

All generated game-derived C/ELF/ISO and PCSX2 snapshots should stay outside
public Git history. Historical detailed checkpoints remain under `CHECKPOINT_*.md`.