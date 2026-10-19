# Getchu-3-PC — TLB syscall initialization checkpoint

October 19, 2026. Experimental native C guest runtime, **NOT PLAYABLE**.

Original MIPS InitTLBFunctions completed to `0x0043FEA0` after **1,922**
additional instructions, including a 204-store retail guest Copy, two original
FlushCache requests (compatibility-only recording), eight SetSyscall operations
and six actual copied GetEntryAddress guest MIPS table searches.

Composed test count: **325,476**. **391** pinned retail opcode rows match the
owner's independent post-boot PS2 EE RAM reference. Clang sanitizer and GCC
focused tests passed; two Windows x64 objects compiled, and the integrated
Linux diagnostic links, but was not run with the original retail ELF.

No game assets, original ELF, ISO, BIOS or savestate RAM are included. No
actual Sony BIOS scheduler/IOP/GS or gameplay has been implemented. See
`CHECKPOINT_TLB_FULL.md` for reproducible tests and limitations.