# Getchu-3-PC — OSD original return checkpoint

October 19, 2026. Experimental native C EE compatibility runtime, **NOT PLAYABLE**.

The original retail instructions now execute through `GetOsdConfigParam`
(0x4B), `SetOsdConfigParam` (0x4A), second getter, original configuration
restoration, and parent return at `0x0043FE98`. **44** further original MIPS
instructions; **323,554** on the composed test path. **290** pinned game
opcodes verified against the owner's post-boot memory snapshot.

The OS configuration is supplied explicitly (`--test-osd-roundtrip=XXXXXXXX`)
and is not claimed as Sony BIOS-observed. Linux optimized and sanitizer tests,
Windows object compilation and source-only full diagnostic linking pass.
The retail ELF, ISO, BIOS, save-state RAM, assets, and gameplay are excluded.
See `CHECKPOINT_OSD_ROUNDTRIP.md` for exact tests and limitations.