# Retail C++ engine reconstruction: manager and action decision paths

**Original source:** Owner-private USA `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

This batch recovers **13 original function entries** as source-level
C++ operations and connected control flow, rather than replacing
gameplay. The source is in `native/src/original_action_runtime.cpp`.
There is still no playable native Windows game.

## Reconstructed connected original engine paths

| Original ELF virtual address | Verified source operation |
| --- | --- |
| `0x3168AC` | Reads the original child object's byte at +0x45 |
| `0x3168B8` | Reads the child's original word at +0x48 |
| `0x3168C4` | Writes the child's word at +0x0C |
| `0x57BA00` | One-time original manager initialization: honor byte +5 guard, clear object +8, mark initialized, honor disabled byte +4, conditionally obtain candidate and store result |
| `0x316FD0` | Initialize manager global 0x696CA0 and return true if it has an object at +8; otherwise initialize and check manager global 0x696CD0 |
| `0x3102F0` | Action is ready iff flags +0x0C bit 0 is set; fields +0x40 and +0x34 are zero; owned child's +0x45 byte is nonzero; and original two-stage manager lookup succeeds |
| `0x310358` | Separate predicate: +0x34 nonzero AND +0x30 nonzero AND +0x40 zero |
| `0x310380` | Return the original nested object's word at +0x70 |
| `0x3103A8` | If requested state differs from original +0x38, call original 0x5638F4 with parent, field +0x14, requested value, then store +0x38 |
| `0x3103F0` | Forward original parent+0xD6 to original 0x165220 |
| `0x276274` | If original byte 0x6354F7 is nonzero, suppress animation-channel call; otherwise forward to original 0x26B310 |
| `0x2762BC` | Forward original parent+0x1C to game channel operation 0x26B524 |
| `0x276580` | Forward original vector field +0x50 to game operation 0x359E48 |

The original manager initializer `0x57BA00` still delegates to the real
original functions `0x221644`, the factory's virtual slot 2,
the returned candidate's virtual slot 11, and `0x21D898`. These
functions and the manager's COP2-initialized fields +0x10/+0x20
must be recovered before real engine managers can be initialized.
The C++ code exposes **these unresolved dependencies**, not made-up
objects or automatically successful lookups.

## Verified dependencies within original `0x56AF44`

The 16 direct callees of that method were inspected. This batch also
identifies deeper original routines such as `0x26B310`,
`0x26B524`, `0x359E48`, `0x57BA00`,
`0x3168AC` and `0x316FD0`. Recovered wrappers preserve their
original call gating/order; vector-unit operations are not guessed.

The owner-private ELF audit:
```powershell
python native/tools/verify_original_action_runtime.py SCUS_975.01 --out private-action-report.json
```
passed **71 original MIPS opcode checks** and **11 direct original
J/JAL call targets**, while sampling 16 bounded related routines.
The script requires the full original USA ELF hash and emits metadata
only, never binary bytes or game assets.

Native host C++ was compiled and tested locally with
`g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`; a second
run with UndefinedBehaviorSanitizer passed. Tests exercise gating,
manager one-time initialization, disabled manager, candidate rejection,
both manager fallback paths, no-change and changed state, and channel
guard/call order. Synthetic objects exist **only in unit tests**.

**Not yet complete:** original candidate factory logic and vtable
calls; real animation math, Player and Monkey main update loops,
asset/render game initialization, or actual Windows gameplay.
Windows CI was not triggered.
