# Getchu-3-PC — Post-FindAddress and EE Timer 3 init checkpoint

**2026-10-19 — Source recovery, not playable.**

## Original retail code implemented

| Guest address | Behavior |
|---|---|
| `0x0043FDC4` | BEQ compares two kernel addresses discovered by original MIPS FindAddress syscall handler |
| `0x0043FDDC` / `0x0043FDF8` | Additional real JAL wrappers call original syscall `0x83` when addresses do not reconcile |
| `0x0043FE18` | Equal-address path stores the result at guest `0x006A1E90` |
| `0x0043FE1C`–`0x0043FE40` | Original 64-bit saved-register restores, JR and delay-slot SP adjustment |
| `0x0043FE70` | Next game JAL, calling `0x00440338` |
| `0x00440338`–`0x00440350` | Real callee stack-frame initialization, LUI and ORI address formation |
| `0x00440354` | Pending LW at **0x10001810 — EE Timer 3 MODE** |

The `0x10001810` identification is grounded in the public PS2SDK
`common/include/ee_regs.h` constant `A_EE_T3_MODE` and `timer.h`
constant `T3_MODE`; no value is injected into that register.

## Execution and limitations

`ae3_boot_post_find.[ch]` executes the original instructions with instruction-
budget slices, rollback-on-fault memory semantics, and handoff to the existing
original MIPS `FindAddress` handler at *additional* original JAL return sites.
`ae3_boot_next_init.[ch]` stops before the first hardware register load.
The early-exit branch and third-lookup paths are tested with user-controlled
RAM fixtures; no unknown BIOS bytes or timer reads are fabricated.

Boot diagnostic modes (require NTSC-U retail ELF; game bytes not included):

```
build/ae3_boot_probe SCUS_975.01
build/ae3_boot_probe SCUS_975.01 --test-ee-boot-model --test-sema-hle --test-ee-alias --test-post-find
build/ae3_boot_probe SCUS_975.01 --test-ee-boot-model --test-sema-hle --test-ee-alias --test-next-mmio
```

- First (default): stops at actual `SetupThread` with NO modelled responses.
- Second: with a stateful compatibility model but NO kernel words, the real
  reconciliation code requests a third `FindAddress` (from low guest address 4).
- Third: two **synthetic 32-bit syscall lookup words** are placed at KSEG0
  addresses `0x80000300` and `0x8000025C`, satisfying the original comparison;
  the native runner reaches the actual Timer 3 MODE read boundary and stops.

## Verification

- Retail ELF SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- Unit tests: actual original opcode assertions, 13-instruction equal-address
  path, 9-instruction first mismatch request, third real guest handler return,
  bounded backward re-search, saved PC/delay state, atomic memory faults.
- New callee: 9 executed original instructions to MMIO pending; original JAL
  return address and stack frame validated.
- GCC / Clang native compile, Clang ASan + UBSan, LLVM cross-compiled Windows
  x64 COFF objects: passed. Windows executable/runtime itself not tested.
- Previous real `FindAddress` regression and original syscall registry tests
  rerun and passed; default non-compatibility boot still stops at SetupThread.

**Next major work:** authentic EE boot/kernel image, a time-accurate EE Timer 3
MODE model, broader guest code / caller coverage, then IOP and VU/GS graphics.
This checkpoint is NOT a playable Windows port and the ELF is not fully decompiled.