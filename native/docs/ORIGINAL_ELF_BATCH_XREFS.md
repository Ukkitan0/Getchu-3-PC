# Original Ape Escape 3 USA ELF: full-program callgraph and code-pointer survey

**Purpose:** Stop making isolated random function passes. Use the actual
private original binary to find the call trees and data-table callback
edges needed to port a complete original subsystem at a time.

Source SHA-256 (USA `SCUS_975.01`):
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Batch scan of the actual original game

`native/tools/scan_original_elf_xrefs.py` scans ONLY original executable
ELF section `.text`; original `.data`, `.rodata`, `.sdata`,
`.ctors` and `.dtors` are surveyed for source pointers into
`.text`. No game-owned data is included in this repository.

The real owner-original file produces:

| Original evidence class | Observed count |
|---|---:|
| Original .text instruction words scanned | 1,351,830 |
| Direct in-.text MIPS JAL sites | 109,169 |
| Distinct direct in-.text JAL targets | 12,228 |
| Direct MIPS J (jump) sites targeting .text | 5,422 |
| Indirect JALR sites requiring vtable/callback recovery | 6,106 |
| In-.text COP2 instruction words, including VU0 macro mode | 30,239 |
| Original conditional branch instruction words | 76,206 |
| Original data section pointers into .text | 23,198 |
| Distinct code pointer targets in original data | 14,126 |

**These counts do NOT represent decompiled functions.** A JAL target
can be a library thunk, raw callback, or an entry overlapping another
source routine. Inferred owner function boundaries are labeled
heuristic. Source `.vutext` is separately stored VU microprogram code
and remains outside this .text scan. Indirect JALR and overlays
must be handled as separate recovery tasks.

### Source verified path now prioritized

- `0x57BCB8`: original actor-controller main initialization;
  original callsites `0x57BAE4`, `0x57BB4C`.
- `0x57C7CC`: original route follower; actual direct original
  callsites `0x57BF28`, `0x57C3D0`.
- `0x57C8EC`: original target-vector selector; original direct
  callsites `0x57C840`, `0x57C878`, `0x57C8DC`.
- `0x57CA68`: original fallback geometry; callsite `0x57C97C`.
- `0x4C1B3C`: original polygon projection has **17** direct callers
  throughout original game, not only the actor controller.
- `0x36F4D4`: original float RNG has **78** direct callers.
- `0x36F548`: original float-range RNG has **325** direct callers.
  It now has an actual native implementation in
  `original_game_random.cpp`.

Example (runs against the user's own private source, not inside GitHub Actions):

```sh
python native/tools/scan_original_elf_xrefs.py /private/SCUS_975.01 \
  --focus 0x57bcb8 0x57c7cc 0x57c8ec 0x57ca68 0x4c1b3c 0x36f4d4 \
  --depth 2 --output original_actor_xrefs.json
python native/tools/verify_scan_original_elf_xrefs.py /private/SCUS_975.01
```

## Project triage, not percentage theater

Use these xrefs to recover a complete chain before declaring runtime
integration: stage-owned source records -> original navigation
route -> actor target selection -> verified source VU0 calculations ->
actor update -> rendering. This helps batch the work in larger
connected passes without hand-picking one new helper address each turn.

Recovery work still needed includes:
1. Source `0x4C1B3C` projection using its original COP2 operations.
2. Source `0x57C98C` random three-vertex vector transform.
3. Source `0x57CA68` fallback VU0 geometry transform.
4. The original `0x57BCB8` actor update and remaining genuine
   Windows runtime initialization, input, graphics and audio.
5. Original assets and authentic original stage Lua binding in a
   complete native executable, not an emulated or replacement game.

No playable native Windows game has been tested.
No emulator or original copyrighted binary is committed.
No GitHub workflow was manually run.
