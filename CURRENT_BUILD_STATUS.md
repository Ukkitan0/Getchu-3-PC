# Getchu-3-PC checkpoint — native boot through EE kernel hook boundary

Source canonical: https://github.com/Ukkitan0/Getchu-3-PC
Retail input: NTSC-U SCUS_975.01 (not included or redistributed).
Original SHA-256: 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63

This checkpoint does NOT constitute a native playable port.

## Verified

- Bounded original EE boot prefixes reached SetupThread after 315,897 retired MIPS instructions, with no assumed BIOS return.
- Optional regression-only SetupThread stack pointer plus SetupHeap acknowledgement reach the real first init function.
- 34 more original guest instructions execute through the nested 0x0043FC08 initialization routine, two real CreateSema syscall wrappers, and return at 0x0043FE68. Kernel return IDs are injected explicitly or allocated by the experimental opt-in HLE kernel.
- With the explicitly selected experimental HLE, guest init code executes to SetSyscall 0x74 at 0x0043FE4C and requests registration of handler 0x0043FCC0 for syscall 0x83. No hook is installed. Total observed retired guest instructions: 315,962.
- Three targeted regression tests pass under AddressSanitizer and UndefinedBehaviorSanitizer.
- Three new C runtime modules build into Windows x64 COFF objects with clang-cl /W4 /WX.
- 1,646 candidate entrypoints are in the older limited-function native dispatcher (not full executable coverage).

## Commands

On Linux (requires original executable):
`./build/ae3_boot_probe /path/to/SCUS_975.01`
`./build/ae3_boot_probe /path/to/SCUS_975.01 --test-kernel-fixture --test-sema-hle`

On Windows use `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` in a Visual Studio developer terminal. Default stops at SetupThread; `set AE3_BOOT_TEST_HLE=1` opts into test-only continuation. Windows build and actual runtime still need verification on Windows.

Next: real SetupThread kernel semantics; SetSyscall 0x74 registrations and handler dispatch; deeper EE code/IOP/SIF/VU/GS integration. A running game window is not provided.