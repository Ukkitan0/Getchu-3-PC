# Getchu-3-PC — original EnableIntc and next Timer 2 read

October 19, 2026. **Experimental native EE compatibility runtime. NOT A PLAYABLE PORT.**

## Concrete new original-code execution

Starting at the prior retail PC `0x004405C8` after the three original Timer 2
COUNT/COMP/MODE writes, this checkpoint now implements the game's authentic
`EnableIntc` MIPS wrapper at `0x00436628` and original syscall stub at
`0x00435820`. The actual guest `SYSCALL` at `0x00435824` is the EE kernel
service **0x14** with cause **11** (EE Timer 2). The guest executes its own
`SD`/`LD` saves/restores, `MFC0`, branches, JAL/JR delay slots and return to
the original allocator's parent at `0x0043FE80`.

In the explicit experimental kernel, only a registered INTC cause can be
enabled. The native handler registry records the cause bit, supports an
idempotent repeated enable and clears the bit after the final removal. The
chosen success/failure return convention is **our test policy, not a verified
Sony BIOS return value**. No INTC_MASK physical register is yet emulated.

The next actual game initializer JAL at `0x0043FE80` enters `0x004406F0`.
Its original `DIntr`/COP0 path with supplied Status EIE=0 proceeds through
`0x0043FBA0` and reaches the original hardware `LW` at **`0x00440710`**
(address **`0x10001010`**, Timer 2 MODE). That read has deliberately not yet
been retired in this checkpoint. The complete opt-in test-mode path now
accounts for **323,393 original guest instructions** (323,338 prior + 37 new
EnableIntc wrapper + 18 next-initializer prologue). Previously semantically
translated MMI memset internal instructions remain excluded.

## Evidence

- `retail_enable_intc_words.tsv` records the exact opcode words from the
  user's earlier retail `SCUS_975.01` instruction inventory, independently
  retained in `analysis/full_elf/ee_text_word_inventory.tsv.gz`.
- `test_boot_enable_intc.c` verifies success, missing-handler service failure,
  original saved-interrupt-state alternate path, single-instruction vs batch
  parity for guest CPU/RAM/kernel state, recoverable unaligned/unmapped stack
  memory and opcode/syscall mutation rejection. Each test uses the pinned
  retail opcodes mapped into a synthetic guest RAM image, **not an ELF boot**.
- GCC -Wall/-Wextra/-Werror build and Clang AddressSanitizer /
  UndefinedBehaviorSanitizer regression passed. Windows x64 COFF objects
  compiled with `clang-cl /W4 /WX`; a Windows executable was **not run**.
- The boot diagnostic is wired to carry on from Timer 2 setup through syscall
  0x14 and the next Timer 2 MODE LW, but its full chain requires the user's
  original legally obtained ELF and **could not be run here**.

## Reproduce

With the user's own US retail ELF:

```sh
AE3_ELF=/local/path/SCUS_975.01 ./BUILD_AND_TEST_LINUX.sh
./build/ae3_boot_probe "$AE3_ELF" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer2-mode
```

On Windows, use `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` in a VS Developer Command
Prompt with `AE3_BOOT_TEST_TIMER2=1`, `AE3_ELF` set and `clang-cl` available.

Without that retail executable, the standalone synthetic-opcode regression
is compiled as:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c ae3_ee_intc_kernel.c \
  ae3_ee_timer2.c ae3_boot_timer2_continue.c ae3_boot_timer2_setup.c \
  ae3_boot_enable_intc.c ae3_boot_timer2_recheck.c \
  test_boot_enable_intc.c -o build/test_boot_enable_intc
./build/test_boot_enable_intc
```

The default diagnostic still halts at authentic `SetupThread`. No genuine
Sony BIOS setup, general CP0 exception behavior, IOP runtime, GS/VU rendering,
controller/audio/gameplay or playable native Windows game has been completed.
The next specific implementation target is actual `LW` at `0x00440710`, its
bit-7 branch behavior and original caller continuation at `0x0043FE88`.
