# Original retail post-TLB caller and memset checkpoint

October 19, 2026 — **NOT PLAYABLE**; experimental native EE instruction execution.

## Original guest path

- At PC `0x0043FEA0` the original `LD $ra, 0($sp)` restores the caller return
  address from guest RAM; the code jumps to `0x00441B70` with the authentic
  `ADDIU $sp,$sp,16` branch-delay slot.
- Original `0x00441B70` clears guest `0x006A2E40` and `0x006A2E44` with two
  original `SW` instructions, then computes `0x00771288` and tail-jumps
  to the game's `0x0042B320` memset with `$a1=0`, `$a2=512`.
- The genuine MIPS `SLTIU`/`BNEZ`/`ANDI` branch sequence chooses the byte
  loop because `0x00771288 % 16 == 8`; no optimized MMI path is entered.
- The routine executes exactly 512 original `SB` stores at `0x0042B3BC`,
  incrementing its pointer and testing its counter via the original branch
  and delay slot. 512 bytes become zero. Stores to neighboring bytes do not
  occur. The original caller/loop total is **3,609 retired guest instructions**.
- The first bounded interpreter stops *before* original `JR $ra` at `0x0042B3D8`.
  A second explicitly gated stage continues ONLY when the guest-loaded `$ra`
  matches the unique static original caller instruction `0x0010031C` (link to
  `0x00100324`). Otherwise it reports the mismatch without forging a return.
  The original main `JAL` to FlushCache syscall `0x64` is executed/recorded
  in compatibility mode, then its conditional branch reaches the next `JALR`
  at `0x0010033C` with target `0x00628210`. This adds **12** original
  instructions; the composed opt-in count is **329,097**.

## Verification

- **45 newly pinned opcodes** agree with the owner-provided 32 MiB savestate,
  making **436 opcode rows** in the complete repository (rows can overlap).
- GCC `-O2 -Wall -Wextra -Werror` on original guest fixture: PASS.
- Clang `-fsanitize=address,undefined` on guest fixture: PASS.
- Windows x64 `clang-cl /W4 /WX /O2` COFF object: compiled.
- Original TLB, OSD, timer, semaphore and thread regressions kept separate.
- Single-step vs batch CPU+RAM parity, exact guest `SW`/`SB` counts, opcode
  mutation refusal, recoverable uninitialized saved-RA `LD` and destination
  store fault checks. No data from game, ISO, BIOS or savestate is distributed.
- Boot diagnostic builds with new module, but complete end-to-end original
  retail ELF + authentic Sony BIOS path was **NOT executed here**.

Run fixture test without game assets:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c \
  ae3_boot_post_tlb.c test_boot_post_tlb.c -o build/test_boot_post_tlb
./build/test_boot_post_tlb
```

**Outstanding:** successful return to the actual guest caller needs original
`$ra` from startup state. R5900 GS/VU, IOP/SIF, audio, input and most game
execution are not implemented. This checkpoint is not a playable Windows port.