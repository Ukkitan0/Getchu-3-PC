# Getchu-3-PC — native original post-TLB startup checkpoint

October 19, 2026. Experimental native EE translation, **NOT PLAYABLE**.

The original guest caller at `0x0043FEA0` clears two globals and jumps
into the retail unaligned `memset` at `0x0042B320`. All 512 original `SB`
operations execute (3,609 MIPS instructions). The unique retail startup
call at `0x0010031C` identifies the expected guest return PC `0x00100324`.
Only with that saved RA does a second interpreter execute the guest JR,
FlushCache request and following main branch, reaching the next JALR
`0x0010033C -> 0x00628210` (+12 MIPS instructions).

Composed experimental guest count: **329,097**. **436 opcode rows** match
the owner-provided EE memory snapshot. Standalone optimized/sanitizer and
Windows object tests passed. No genuine BIOS startup, complete retail ELF
verification, IOP/GS/audio, or playable build. See CHECKPOINT_POST_TLB_MEMSET.md.