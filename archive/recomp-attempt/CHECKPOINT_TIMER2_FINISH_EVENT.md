# Getchu-3-PC — Timer 2 second MODE read and next event initialization

October 19, 2026. **Experimental EE compatibility mode. NOT PLAYABLE.**

## Recovery relative to commit 8fa31a7

The earlier original-code chain stopped **before** `LW $v0,($v0)` at
`0x00440710`. This continuation executes that instruction against the same
EE Timer 2 register model at physical address `0x10001010`. No MODE value
is inserted directly into guest CPU registers.

The original instructions at `0x00440714` through `0x0044077C` are now decoded
through the original branch/delay-slot and epilogue paths:

- MODE bit 7 set, saved DIntr state zero: original `LW`, mask, branch, load
  delay-slot, epilogue `LD`, and `JR` restore the caller to `0x0043FE88`.
  **10** original guest instructions executed.
- MODE bit 7 set, saved DIntr state nonzero: stops BEFORE guest `JAL` at
  `0x00440728` to the `EIntr` COP0 routine `0x0043FBF0`, not approximated.
  **6** original instructions executed.
- MODE bit 7 clear: follows the actual branch to `0x00440738`, constructs
  the new MODE argument, and enters the **original** SetT2_MODE JAL at
  `0x00440744`; its existing shared guest writer at `0x00440410` executes
  original MFC0/MTC0/SYNC, KSEG1 register `SW`, and `ERET`. This path has
  **29** original guest instructions including the nested writer, one native
  Timer 2 register write, and reaches the NEXT service JAL at `0x0044074C`
  (target `0x00440CF8`), which remains unexecuted.

On the default experiment’s enabled-Timer2 path, the original parent
`0x0043FE88` next JAL enters `0x004368B8`. The guest checks a real mapped
`LW` from `0x006A1788` before selecting one of two paths:

- Zero/negative state: original guest prepares a descriptor in guest stack
  memory, calls the original stub at `0x00435AE0`, and reaches **syscall 0x40
  at 0x00435AE4**. Service was *not* answered. This path retires **20**
  additional original guest instructions.
- Positive state: original guest skips allocation and returns via actual
  register-restoring epilogue to main initializer `0x0043FE90` after **18**
  instructions. Return register is -1 per the original delay-slot opcode.

Relative to the former **323,393** counted guest instructions, the default
opt-in experimental fixture now has **323,423** accounted for at the pending
syscall 0x40 boundary (10 + 20 additional). This figure is composed from
independently checked paths, **not** obtained from an uninterrupted fresh
retail ELF boot trace here. Earlier native C-translated MMI loop work is
not miscounted as original MIPS instructions.

## Verification

- New `test_boot_timer2_finish.c` covers enabled and disabled MODE bits,
  the EIntr-callee boundary, repeat execution parity (single-step vs batch),
  original nested SetT2_MODE and MMIO alias, code mutation, and recoverable
  unmapped-memory faults. Test opcode fixture is
  `retail_timer2_finish_words.tsv`.
- New `test_boot_event_create.c` covers original zero, negative, positive
  global state branches, guest syscall number and descriptor field contents,
  single-step vs batch RAM/CPU parity, rejected opcode modification, and
  atomic memory-fault recovery. Test opcode fixture is
  `retail_event_init_words.tsv`.
- Both new tests pass Clang `-fsanitize=address,undefined` and optimized
  compile with warnings as errors. Earlier EnableIntc and EE memory bus/
  Timer 3 regressions were rerun and passed. The full boot diagnostic
  compiles and is now wired to follow the actual MODE=enabled path to the
  next event-initialization boundary.
- Changed `ae3_boot_timer2_setup.c` and both new native C implementations
  compile with `clang-cl /W4 /WX` into three valid Windows x64 COFF objects.
  **A Windows game executable was not built or tested**.

## Build and reproduce

Synthetic opcode validation (no original game needed):

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c \
  ae3_ee_timer2.c ae3_boot_timer2_setup.c ae3_boot_timer2_finish.c \
  test_boot_timer2_finish.c -o build/test_boot_timer2_finish
./build/test_boot_timer2_finish
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c \
  ae3_boot_event_create.c test_boot_event_create.c -o build/test_boot_event_create
./build/test_boot_event_create
```

Opt-in full-chain diagnostic **requires user's own original retail
`SCUS_975.01` executable**, absent from this environment:

```sh
export AE3_ELF=/path/to/SCUS_975.01
./build/ae3_boot_probe "$AE3_ELF" \
  --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer2-mode
```

The complete integrated ELF boot chain is **unverified at this checkpoint**.
The default diagnostic still stops at authentic SetupThread, without
synthetically initializing BIOS. Source only, no retail ELF/ISO/BIOS/assets.

## Next blockers

- The game’s syscall `0x40` must be handled using documented EE-kernel
  semantics and its actual guest descriptor, with explicit object lifetime.
- Disabled Timer 2 second initializer must execute original `0x00440CF8`
  and `0x00440800` calls, not skip them.
- Actual BIOS, IOP, renderer (GS/VU), audio, input and gameplay remain
  unimplemented in this project. This is not a playable Windows native port.
