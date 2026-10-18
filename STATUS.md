# Source recovery checkpoint — 2026-10-18

The **decompilation code here** is canonical. Earlier ps2-web-recomp runtime experiments are external references, not the source tree for this project.

## Verified from original US retail ELF

- SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
- ELF size: 6,488,148 bytes.
- Executable section analysis covers 1,351,830 aligned word locations, including possible embedded non-code.
- Scalar translator: **1,163** nonoverlapping, straight-line *candidate* routines; **9,304** independent reference tests passed.
- Control-flow translator: **168** additional bounded forward-branch/jump *candidate* routines; **1,680** independent reference tests passed (10 per candidate).
- Of the 168 new entries, **48** appear in the entry-reachable direct-call target set, so candidate counts must not be misrepresented as verified runtime coverage.
- 1,998 machine instruction locations in the new control-flow candidates, in addition to 4,067 previously translated scalar instruction locations.
- Linux shared libraries compiled; new control-flow code cross-compiled to a Windows x64 COFF object.
- **Not a bootable native Windows game.** Native GS/VU1 geometry, IOP, disc reads, DSP/audio, runtime state and most R5900 opcode groups are not implemented by this recovery tool.

## Safety and correctness invariants

- Hash-check the original executable before any analysis.
- Reject unknown instructions rather than substituting NOPs.
- Never interpret branches to delay slots or backward branches as simple forward-only functions.
- Preserve branch decision before delay-slot execution, and execute return delay slots.
- Compare generated native C to independent instruction-word interpreter over registers and memory.
- Exclude original copyrighted game files and generated game-derived C from this public repository.

## Next engineering priorities

1. Add bounded loop and multi-return CFG lifting with termination-safe tests; improve candidate/function-boundary confidence.
2. Implement remaining EE scalar/128-bit, COP1, VU and system instructions in a maintained runtime.
3. Build native Windows EXE against compatible EE/IOP, DMA, GS/VU and audio interfaces.
4. Run real Windows hardware tests through title, gameplay, geometry, NPCs, audio, controllers and saves.

Previous V14/V15 code may inform implementation but must not be assumed to be correct or required here.
