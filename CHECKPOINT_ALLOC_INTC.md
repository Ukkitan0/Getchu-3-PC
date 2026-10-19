# Checkpoint: retail allocator startup and first AddIntcHandler boundary

Date: 2026-10-19. Original NTSC-U `SCUS_975.01`, verified ELF SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This is source-recovery/runtime engineering, NOT a playable Windows game.

## Original retail execution

The prior native initialization runner returns to 0x0043FE78. New code
`ae3_boot_alloc_init.[ch]` executes the original `JAL 0x004404B0`, branch
conditions and delay slots. The retail executable's 0x006A2E28 sentinel is
initially -1. Consequently its original MIPS takes the setup path:

- Save six nonvolatile registers and enter the genuine initializer.
- Encounter `JAL 0x0042B320`: this is an optimized R5900 MMI memset; for the
  exact observed invocation (`0x0076EE80`, byte 0, 8192 bytes), it is
  **semantically translated** to an atomic, bounded native-C RAM write.
  The internal MMI instructions are *not* claimed executed or counted.
- Execute original MIPS 128-iteration descending 64-byte pointer chain
  loop, writing 128 pointers into that initialized 8-KiB allocation area.
  The last pointer is cleared by the original JAL delay-slot store.
- Execute the real nested `0x00441780` routine, creating a 64-node
  16-byte-spaced free list at `0x00770E80`, then return to caller.
- Stop at 0x00440558 before next nested call, then explicitly verify the
  original `JAL 0x004357D0`, its delay slot, and the wrapper's
  `ADDIU $v1,$zero,0x10`. Stop at `syscall` 0x004357D4.
- At that syscall, guest arguments are: cause `$a0=11`, handler
  `$a1=0x00440A50`, next `$a2=0`. PS2SDK identifies 0x10 as AddIntcHandler.
  **No native interrupt handler/BIOS result is fabricated or returned.**

The alternate branch is verified by deliberately replacing the sentinel
with zero: the original routine returns guest error `0x80008001` without
allocating/clearing the region. This is a *fixture*, not the default ELF.

## Verification

`test_boot_alloc_init.c` passes on original ELF with Clang ASan and UBSan.
The test compares per-instruction and batched execution states and all 32
MiB of guest RAM, checks 8-KiB pointer bytes and free-list links against
independently constructed expectations, injects retryable stack and
zero-fill mapping faults, and tests atomic rejection of modified syscall
wrapper code. Original preceding kernel-copy and GetEntryAddress tests
continue to pass. The default diagnostic still stops at SetupThread.

- Windows: `clang-cl /W4 /WX /c ae3_boot_alloc_init.c` verified x86-64 COFF
  using temporary standard-library header shims in this Linux runner.
  A runnable Windows diagnostic has NOT been compiled/launched on Windows.
- Linux: `BUILD_AND_TEST_LINUX.sh` includes the test. For an isolated run:
  `cc -std=c11 -O2 -Wall -Wextra -Werror ae3_boot_alloc_init.c ae3_ee_memory_bus.c ae3_elf_image.c test_boot_alloc_init.c -o test_boot_alloc_init`.
- Boot diagnostic: `build/ae3_boot_probe SCUS_975.01 --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer3-copy-alloc`
  reaches 0x004357D4 after **323,244 original guest instructions retired**.
  This number includes translated-code boundaries but DOES NOT count the
  internal original MMI memset instructions that were not executed.

## Limitations and next task

SetupThread, SetupHeap, semaphore allocation, cache services and interrupt
setup remain experimental compatibility behavior. Complete R5900 MMI,
EE BIOS, IOP, VU/GS rendering, audio/input and gameplay are not yet
implemented. Next: implement or faithfully bridge AddIntcHandler for
EE INTC cause 11, then continue original 0x00440560 instructions and nested
initialization. An honest BIOS execution path is still a separate priority.