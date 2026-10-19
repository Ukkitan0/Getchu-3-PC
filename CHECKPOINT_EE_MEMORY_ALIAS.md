# Getchu-3-PC — physical EE RAM alias and original guest-handler execution

Checkpoint date: 2026-10-18. Not a playable native Windows game.

## Engineering change

Before this change `ae3_guest_find_slice()` read `0x80000000` from a second
caller-supplied buffer. The actual EE hardware maps KSEG0/KSEG1 to physical
RAM. The new `AE3_EEMemoryBus` uses **the same** 32 MiB guest RAM backing
for ordinary, uncached, KSEG0, KSEG1 and accelerated-uncached RAM aliases.
Optional BIOS and scratchpad byte arrays are only mapped when supplied.
Unmapped I/O/TLB accesses fail, without fabricated values or partial writes.
The original FindAddress instruction engine can now read using the shared bus.
The older linear-window test API remains for backward compatibility.

## Verification completed locally

- GCC and Clang C11 compile with `-Wall -Wextra -Werror`.
- New bus and original guest handler tests pass with both ASan and UBSan.
- Original guest code and branch/delay/loop behavior match the prior linear
  test window. The two-PCXS2-title-screen-snapshot reference queries match:
  first returns 0x80014F0C after 21,444 reads, second returns zero after
  131,073 reads. This is POST-BOOT memory and cannot confirm boot-time BIOS.
- Fresh ELF-only RAM has an uninitialized first 1 MiB. Both syscall-handler
  searches return zero, rather than a made-up BIOS address.
- Opt-in diagnostic exercises actual retail instructions all the way from
  0x00100134 to 0x0043FDC4, **1,626,728 retired guest instructions**, using
  an explicitly synthetic SetupThread return and experimental semaphore HLE.
  Default diagnostic STILL stops at the authentic SetupThread boundary.
- Two modified C modules cross-compiled into x86-64 Windows COFF .obj files
  using Clang with freestanding C headers; Windows executable not run locally.

## Run

- Linux: run `BUILD_AND_TEST_LINUX.sh` with `AE3_ELF` set, or use the focused
  `test_ee_memory_bus`, `test_findaddress_bus`, and `test_boot_probe_alias.py`.
- Windows: in a VS2022/Clang developer command prompt, run
  `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` (default: no fake kernel response).
  Set `AE3_BOOT_TEST_ALIAS=1` to opt into non-authentic test-mode execution.

## Remaining blockers

1. Actual EE kernel initialization and SetupThread/SetupHeap semantics.
2. Correct handling of installed syscall handlers in a populated kernel,
   including interrupts, low memory, IOP/SIF and MMIO interactions.
3. Remaining R5900/VU code execution, GS/VU rendering, input/audio/disc/saves.

No retail executable, ISO, BIOS, or post-boot RAM snapshot is in the public
repository or generated checkpoint package.

Memory mapping references:
- https://www.psdevwiki.com/ps2/Memory_Map
- https://github.com/PCSX2/pcsx2/blob/master/pcsx2/Memory.cpp