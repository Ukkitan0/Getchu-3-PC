# 2026-10-19 — EE Timer 3 and original game-installed kernel syscall 0x5A

This checkpoint does **not** boot Ape Escape 3 and is not a playable Windows game.
The original NTSC-U SCUS_975.01 retail executable is used locally only.

## Original guest-code execution verified

With *explicit* test-only EE boot services and artificial words in kernel RAM
for the earlier FindAddress pair, the original guest code now executes:

- `0x00440354`: `LW $v1,0($v0)` accesses hardware register `0x10001810`
  (EE Timer 3 MODE), via an attached native timer model.
- `0x00440358`–`0x0044035C`: compares MODE bit `0x0100`, the compare-interrupt
  enable (CMPE) bit per PS2SDK/PCSX2. The original branch delay slot
  is respected and PC/registers are preserved across execution slices.
- With CMPE=1: restores `s0/s1/s2`, stack and returns to `0x0043FE78`.
- With CMPE=0: reaches original `JAL` at `0x00440378` -> `0x004402D0`.
- Original `SetSyscall` wrapper, syscall `0x74`, registers the guest handler
  `0x004402F0` for syscall `0x5A`. The registration is performed by the
  **explicit experimental compatibility kernel**, not actual Sony BIOS code.
- Original call at `0x00440390` enters the syscall `0x5A` wrapper at
  `0x004402E0`. The compatibility dispatcher routes it to the *original MIPS*
  handler `0x004402F0`, which loops over 464 source words (1,856 bytes),
  copying them from `0x006A2678` to KSEG0 `0x80076000` (physical `0x00076000`).
- Byte-for-byte RAM equality with the original ELF payload is checked.
  Control reaches caller `0x00440398`. The guest syscall's BIOS trampoline,
  kernel privilege/context saving and handler-return ABI are **modeled**, not
  reconstructed from Sony BIOS execution.

**Observed end-to-end counter (test-mode only): 321,484 retired original
guest instructions.** Earlier authentic code without mock kernel state
continues to stop at `SetupThread` (315,897 retired instructions).

## Hardware model

`ae3_ee_timer3.[ch]` implements a bounded Timer3 COUNT/MODE/COMP register
model, write-one-to-clear event flags, divider choices for EE CPU clock cycles,
compare and overflow, atomic unsupported gate/HBLANK modes. Bus attachment is
optional: default MMIO remains unmapped. This is a limited register model, not
an accurate interrupt scheduler or a cycle-accurate PS2 emulator.

PS2SDK register definitions: https://ps2dev.github.io/ps2sdk/timer_8h_source.html
PCSX2 legacy EE timer reference: https://sources.debian.org/src/pcsx2/1.6.0%2Bdfsg-2/pcsx2/Counters.cpp

## Verification

- Four new focused native tests pass using the original executable:
  `test_ee_timer3`, `test_boot_timer3_resume`, `test_guest_kernel_copy`,
  `test_boot_timer3_syscalls`.
- All four tests pass Clang AddressSanitizer/UndefinedBehaviorSanitizer with
  `-Wall -Wextra -Werror`.
- Four new runtime C modules compile with `clang-cl /W4 /WX /O2` to Windows
  x86-64 COFF objects; a real Windows executable was **not** run on Windows.
- One-step vs batched execution parity; retryable faults on missing hardware
  and unmapped kernel-copy destination; mode bit clear and set are checked.
- The default boot diagnostic is unchanged and still correctly pauses at
  `SetupThread`. Earlier `--test-next-mmio` mode still stops *before* the read.
- The full legacy generated-C regression suite was **not** rerun end-to-end in
  this checkpoint; focused new checks and the runtime boot probe were run.

## Windows diagnostic

`BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` with `AE3_BOOT_TEST_TIMER3_COPY=1` opts
into the experimental guest-kernel copy path, **not** into a playable game.
The default still stops before unsupported BIOS services.

## Remaining blockers

Real EE kernel state and syscall exception handling, IOP/SIF, EE MMI/FPU
coverage, VU/VIF/GS rendering, audio/input, and complete game integration.
The original source's entire instruction inventory is not a complete native-C
translation. The user-supplied retail game is never included in the repo/ZIP.