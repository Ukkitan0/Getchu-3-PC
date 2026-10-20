# Checkpoint — Native guest execution and retail ELF memory image

Date: 2026-10-18. This checkpoint supersedes the earlier backward-call checkpoint as the **canonical source**. It does not supersede the unreconstructed game code or imply playability.

## Implemented

1. **`ae3_guest_execution.c/.h`:** Unified C11 bridge to generated scalar-leaf and stack-call executors. 
   - `ae3_guest_open(exec, entry)` chooses an executor for any of 1,646 accepted entrypoints. Unknown entry returns `AE3_RESUME_UNKNOWN` and leaves no active frame.
   - `ae3_guest_slice(exec, ctx, budget)` preserves guest state on a budget yield or memory fault.
   - `ae3_guest_pc`, `ae3_guest_retired`, `ae3_guest_done` expose real execution state; nested leaf PC is resolved from the deepest frame.
   - The actual retail ELF entry `0x00100134` is rejected until its real instructions are supported.
2. **`ae3_elf_image.c/.h`:** ELF32 MIPS EE memory-image loader using a caller-provided RAM region. Validates headers, segments, file/memory bounds, overlaps and entrypoint placement before copying any bytes. Does not bypass executable SHA checking (performed by the existing Python source pipeline).
3. **Build integration:** Linux regression/build script now tests both modules; Windows script compiles two additional x64 COFF objects.

## Locally verified

- Original executable SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- Original ELF: one PT_LOAD; 6,473,168 bytes (`0x62c5d0`) copied to 0x00100000 and BSS of `0x672910 - 0x62c5d0` bytes zeroed; all bytes matched the original ELF.
- ELF input tests: six malformed-header/segment cases returned failure without modifying the destination memory image.
- Native executor parity for recovered leaf and caller, including retail loop with 0 and 3 iterations, a true memory fault and later successful retry, and saved delay-slot PC.
- `clang-cl` Windows x64 COFF generation passed for both new modules. Existing original-word stack, branch, backward-loop and dispatcher tests passed after the change.

## Outstanding before a playable PC build

True startup path at 0x00100134, wider code coverage, EE 128-bit/MMI/COP instructions, register/memory mapping, PS2 services, VU/GS rendering, audio/media, controller hooks, and complete Windows game integration. Source-only objects are not playable. The next engineering priority is mapping the boot entry's verified call tree and implementing the unsupported instruction and service boundaries without mock bootstrap code.

## Retail boot-path decoding (non-executing audit)

`audit_boot_entry.py` now validates 29 MMI `PADDUW` zero-reg writes, four primary/secondary HI/LO reset instructions, a `BGEZALL`, and 32 `MTC1` zero writes before the FPU accumulator instruction at `0x0010023C`. This is an instruction audit only; none of those first boot instructions has yet been executed by the low-64-bit native guest engine. Implement genuine 128-bit GPR, HI/LO and COP1 state before claiming startup execution. Opcode assignments cross-checked against PCSX2 R5900 opcode tables.