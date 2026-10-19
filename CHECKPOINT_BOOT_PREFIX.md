# Boot prefix execution checkpoint — 2026-10-18

**Target:** original user-supplied NTSC-U `SCUS_975.01`, SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Verified native execution

The new `ae3_boot_prefix.c/.h` interprets the *actual mapped retail ELF* from EE entry `0x00100134` through the first call site `0x001002C4`, then stops before executing that JAL (to `0x001036D8`). It is not a replacement executable or a playable build.

- 143,836 guest instructions retired, with correct branch-delay-slot handling; 20,551 arbitrarily-sized execution slices produce the **same complete saved state and 32 MB RAM** as a single execution.
- 29 `PADDUW` instructions clear all 128 bits of each named GPR. Four HI/LO bank clears, a taken `BGEZALL` link, 32 `MTC1` FPR clears, `ADDA.S` zero accumulator, `SYNC` and `CTC1` control register reset execute using saved architectural state.
- 17,969 real `SQ $zero` operations clear every byte of `0x0072C600` through `0x0077290F` (287,504 bytes). The game only starts using `SQ` after constructing its base and aligned end using actual `LUI`, `ADDIU`, `AND`, `ANDI`, `ORI` instructions.
- A memory mapping fault on the first real `SQ` instruction leaves both architectural state and guest memory unchanged; resuming after the mapping is corrected reaches the same result.
- A modified unsupported original opcode causes an explicit rejection without incrementing retired instruction count or changing PC. Restoring the code permits successful continuation.
- Guest RAM loaded from the original ELF with `ae3_elf_image.c`; the tests seed its BSS with nonzero bytes before verifying that real machine instructions clear it. Adjacent sentinel bytes remain unchanged.
- Verified native Linux C compilation with `-Wall -Wextra -Werror`, then AddressSanitizer + UndefinedBehaviorSanitizer. Windows x64 COFF produced via `clang-cl /W4 /WX`.

## Scope and next call

The runner is **restricted to the real boot prefix only**. Its COP1 `ADDA.S` special case deliberately supports the reset's 0+0 inputs, not arbitrary FPU arithmetic. No guest BIOS, IOP, GS/VIF/VU, audio, media loader, input polling, memory-mapped device buses or scheduler are implemented by this module.

At `0x001002C4`, the next instruction is `JAL 0x001036D8`. That callee uses 128-bit loads/stores and memory-mapped PS2 hardware. The next meaningful milestone is executing that function with a typed guest MMIO interface and returning to the caller—not falsely forcing past the call.

## Reproduce

Set `AE3_ELF` to the *original* executable and run `bash BUILD_AND_TEST_LINUX.sh`, or build only this stage:

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_boot_prefix.c ae3_elf_image.c test_boot_prefix.c -o boot_prefix_test
./boot_prefix_test /path/to/SCUS_975.01
```

Generated game-code C and the ELF/ISO remain private and must not enter this public Git repository. `ae3_boot_prefix.*` is our independent interpreter/host-side source, not copied code from the retail binary.