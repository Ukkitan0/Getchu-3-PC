# Getchu-3-PC — Original Timer 2 hardware initialization

October 19, 2026. **Experimental native EE translation; NOT PLAYABLE.**

## Original retail path

Source is the user's US retail `SCUS_975.01` ELF (SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`).

- Starts at PC `0x00440590`, original `LW $s0,0($v1)`, `$v1=0x10001010`
  (Timer 2 MODE). It uses a real mapped guest `LW`, not a host-injected
  register return.
- Bit 7 clear: original MIPS branches through `SetT2_COUNT` at `0x00440480`,
  `SetT2_COMP` at `0x004404A0`, `SetT2_MODE` at `0x00440490`.
- Bit 7 already set: original branch skips COUNT and COMP setup and calls the
  MODE wrapper directly.
- The three wrapper paths retain the game's `J`, `JAL`, delay slots, `MFC0`,
  `MTC0`, `SYNC`, guest `SW`, and `ERET` operations in the shared routine at
  `0x00440410`. Only the explicit EIE=0 COP0 path is supported.
- The physical `0x10001000/10/20` MMIO registers and uncached aliases
  `0xB0001000/10/20` map to the same native Timer 2 register state.
- MODE=0 initial test: COUNT=0, COMP=0xFFFF, MODE enable flags written,
  66 guest instructions in the three wrappers, reaching the original
  `EnableIntc` JAL at **`0x004405C8`**. The full test-mode boot chain has
  **323,338** counted original guest MIPS instructions. MMI zero-fill is a
  native semantic translation and its original internal instructions are
  not falsely counted.
- MODE bit 7 already set: 22 guest wrapper instructions, MODE only.

## Verification

`test_timer2_boot.c` tests 0, 0x80, and 0x03 MODE inputs, single-step and
batched CPU-state equality, recoverable MMIO read and write faults, opcode
mutation rejection, address and alignment validation, and data-alias equality.
The Timer 2 hardware module rejects unsupported gate/HBLANK timing atomically
and only advances by explicitly supplied EE cycles. Earlier bus, Timer 3,
and interrupt regression programs still pass. Three new C modules compile to
Windows x64 COFF with `clang-cl /W4 /WX`.

Opt-in boot diagnostic:

```sh
AE3_ELF=/path/to/SCUS_975.01
./build/ae3_boot_probe "$AE3_ELF" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer2-mode
```

On Windows use `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` from a VS Developer Prompt
with `AE3_BOOT_TEST_TIMER2=1` and `AE3_ELF` set to the user's legally owned
retail executable. The Windows diagnostic executable was not run here.

## Important limitations

The COP0 Status=0 and initial Timer 2 MODE=0 are explicitly supplied test
inputs, **not observed boot-time Sony BIOS state**. The bounded guest
interpreter cannot execute arbitrary COP0 exceptions or hardware operations.
The next guest syscall/call boundary is `EnableIntc` at `0x004405C8` (lower-level
syscall `0x14` per PS2SDK). Main game entry, IOP, GS/VU, audio, controller,
and other boot services are still missing. A playable Windows game **has not
been produced**.