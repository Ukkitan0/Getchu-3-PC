# Getchu-3-PC — Thread priority / OSD checkpoint

October 19, 2026. **Not playable.** Experimental original guest execution now reaches `GetOsdConfigParam` (`0x4B`) at `0x00435B94` via `GetThreadId` (`0x2F`), `ChangeThreadPriority` (`0x29`), and the parent initializer. **323,510** counted original instructions across composed experimental tests; **253** opcode rows match the post-boot savestate.

The running boot-thread ID of **255** is an explicit compatibility test fixture, not a recovered Sony BIOS ID. Source is portable C; Windows outputs are x64 COFF objects rather than a playable EXE. The game's ELF/ISO/BIOS, save-state EE RAM, and assets are excluded. For tests and limitations see `CHECKPOINT_THREAD_PRIORITY_OSD.md`.