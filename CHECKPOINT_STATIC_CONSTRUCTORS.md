# Getchu-3-PC — retail static-initializer and constructor-table checkpoint

October 19, 2026 — **not playable; experimental EE compatibility mode**.

## Original retail execution reached

After the already-tested caller at `0x0010033C`, the original `JALR $s0`
selects `0x00628210` and executes a genuine `SD $ra`/stack frame, then
performs both original `JAL`s:

- `0x00628218 -> 0x001000A0`: original registration wrapper. The source code
  calls `0x004270F0`, which tail-jumps to `0x00427078`. If the guest word at
  `0x006B7F90` is zero, the original branch skips registering anything and
  returns. If nonzero, the new runtime STOPS before dynamic registration.
- `0x00628220 -> 0x00428AB8`: original constructor dispatcher. It loads
  callback pointers from guest `0x0062E89C` *backward* until a `0xFFFFFFFF`
  sentinel. The exact source pointer address is determined by the original
  `LUI $t7,0x63; ADDIU $t7,-0x1760; LW $t5,-4($t7)` instruction sequence.
- On the nonempty reference table, 52 original guest instructions execute
  after `0x0010033C`, ending **before** the genuine callback `JALR` at
  **`0x00428AE8`** with first target **`0x0041A390`**. No C++ constructor,
  callback, or native substitute is executed yet.
- Under an **explicit test-only empty table** (first word `0xFFFFFFFF`), the
  same original branch and stack restoration return to the main routine;
  64 original guest instructions execute before the next original `JAL`
  boundary **`0x00100354 -> 0x00428B80`**. This is NOT claimed as the
  real game's constructor-table state.

## Recovered constructor inventory

The owner-supplied *later-game* 32 MiB EE memory image contains a contiguous
original callback table: **143 distinct, aligned, mapped guest code targets**,
from `0x0062E89C` backward to the `0xFFFFFFFF` terminator at `0x0062E660`.
The first three entries are `0x0041A390`, `0x00417C58`, `0x003A6CD0`.
The full address-only inventory is in `retail_constructor_addresses.tsv`.

`verify_constructor_snapshot.py` confirms all table addresses/pointers,
uniqueness, mapping, nonzero first instructions, and the terminator in the
provided RAM reference; it **does not prove** the state of the table at
boot or execute any callback. This later-game snapshot is not distributed.
The **68** new exact-pinned MIPS word rows from the new guest execution
module match the same RAM image, increasing the aggregate repository
opcode-row verification from 436 to **504** (some rows overlap).

## Verification and limitations

- New C module: `ae3_boot_static_dispatch.[ch]`.
- Tested exact PC/opcode allowlist, original J/JAL/JALR/JR, delay slots,
  64-bit guest stack `SD`/`LD`, signed register writes, loop branch,
  MMIO-independent guest RAM reads, and recoverable memory faults.
- Single-step/batched register+memory parity; zero and nonzero registration
  branches; original nonempty callback and explicit empty-list branches.
- Constructor-table scan validates guest pointers *without invoking* them;
  failure keeps the caller's output untouched.
- GCC `-O2 -Wall -Wextra -Werror` PASS; Clang ASan+UBSan PASS;
  `clang-cl /W4 /WX /O2` Windows x64 COFF object produced.
- Prior TLB, post-TLB memset, main return, OSD, and thread tests remain green.
- Integrated boot diagnostic links with 44 native C modules, but there is
  **no retail-ELF end-to-end boot or authentic Sony BIOS test here**.

**Next:** implement real first constructor `0x0041A390`, which transfers into
`0x0041A088`, then advance through actual callback side effects and the
remaining 142 constructor entries. Do not shortcut the constructor table.
The main game, IOP, GS/VU graphics, controller/audio, and native gameplay
runtime are unfinished. This project is not yet a playable Windows port.