# Getchu-3-PC — CreateThread / GetThreadId boundary checkpoint

October 19, 2026. **Not playable.** Experimental native EE boot translation
now reaches the original `GetThreadId` (`0x2F`) syscall at `0x004359D4` via
retail `CreateThread` (`0x20`), native dormant-thread registry, guest JR/BGEZ,
and nested COP0 Status query. The failed CreateThread path executes original
`DeleteSema` (`0x41`). 323,479 counted original instructions, combining
verified experimental guest segments, not authentic continuous BIOS boot.

221 pinned MIPS words verify against the separate user's post-boot EE RAM
reference; original game executable/ISO/BIOS/reference RAM excluded here.
The only Windows outputs are x64 COFF module objects, not a working Windows
game or tested Windows executable. See `CHECKPOINT_THREAD_GETID.md` and
`TEST_RESULTS_THREAD_GETID.txt`.