# Checkpoint — EE interrupt registration and Timer 2 boundary (2026-10-19)

This remains an **experimental native-C retail boot diagnostic**, NOT a playable
Ape Escape 3 Windows executable or Sony BIOS reimplementation.

## Original retail EE execution added

- Previous verified boundary: `0x004357D4`, syscall `0x10` (`AddIntcHandler`),
  interrupt cause `11`, guest handler `0x00440A50`, `next=0`.
- New `ae3_ee_intc_kernel.[ch]`: 64-slot guest-handler registry with bounded
  cause validation, mapped-entry verification, positive locally allocated IDs,
  lookup and removal. These IDs and policies are **not verified BIOS semantics**.
- New `ae3_boot_post_intc.[ch]`: retires the retail `syscall`, `JR $ra`, delay
  slot, `MOVE $v1,$v0`, `BGEZL` and its *conditional* delay slot. A positive
  returned ID reaches the next `JAL` at `0x00440578`; a negative return
  follows the real error path to `0x004405E4` (including the correct annulled
  BGEZL delay slot), without inventing a successful registration.
- New `ae3_boot_intc_critical.[ch]`: follows the retail `JAL 0x0043FBA0`,
  validates its delay-slot guest SW to `0x006A2E28`, and executes the actual
  CP0 Status bit-16-clear path, with an **explicit caller-supplied** COP0
  status. Stops before the next hardware read at guest PC `0x00440590`.
- **Hardware correction:** the next read is `0x10001010` — **EE Timer 2 MODE**,
  not INTC_STAT. Official PS2SDK EE register definitions list
  `T2_MODE=0x10001010`, `I_STAT=0x1000F000`, `I_MASK=0x1000F010`.
- The opt-in boot diagnostic now reaches that pending Timer 2 MODE read after
  **323,264** retired original guest instructions (semantic native-C MMI
  memset not counted); the default boot diagnostic still stops at unmodeled
  Sony BIOS `SetupThread` after 315,897 retired instructions.

## Verification

- `test_boot_post_intc.c` loads the user's retail ELF and executes the preceding
  original allocator, the `AddIntcHandler` wrapper and the full CP0-disabled
  continuation. It checks the positive and negative branches, the actual
  handler-ID memory write, mapping faults, unchanged retry state, instruction
  bytes, 1-instruction vs batched equivalence and handler registry rejection.
- GCC and Clang tests pass under AddressSanitizer/UndefinedBehaviorSanitizer.
  Preceding allocator test passes unchanged.
- The three new modules compile with `clang-cl /W4 /WX` to **x86-64 COFF**.
  This does not establish successful Windows linking, startup or game play.
- The Linux boot probe builds, prints the original `SetupThread` stop in default
  mode, and reaches pending Timer 2 MODE in explicit
  `--test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer3-copy-alloc` mode.

## Still missing

Faithful Sony EE BIOS kernel memory/services; real interrupt delivery and COP0
state; Timer 2 device state; IOP, VU, GS rendering, input/audio and full game
translation. This checkpoint intentionally stops **before** reading unmapped
Timer 2 MODE and refuses the unmodeled COP0 EIE-enabled path.