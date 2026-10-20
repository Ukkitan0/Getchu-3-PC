# Original Ape Escape 3 Player action dispatcher — source-level C++ port

Verified directly against hash-pinned original USA SCUS_975.01, *not* invented movement behavior.

## Recovered control flow

Player secondary vtable `0x6B3960`, method `0x34A57C`, contains a 12-case
R5900 jump table at `0x7122FC`. The C++ translation is
`native/src/original_player_mode.cpp`, built into `Getchu3Assets`.

| Original selector | Actual original action |
|---|---|
| 0 | call `0x33FCE4(1)` on Player-owned resource |
| 1 | call `0x33FCE4(5)` on Player-owned resource |
| 2 | call `0x33FCE4(10)` on Player-owned resource |
| 3 | no action |
| 4 | call `0x34675C(20.0f)`, original float at `0x7122F4` |
| 5 | call `0x34675C(100.0f)`, original float at `0x7122F8` |
| 6 | no action |
| 7 | call `0x3509AC(1,1)` |
| 8 | call `0x3509AC(2,1)` |
| 9 | call `0x33FD90()` on Player-owned resource |
| 10 | call `0x324260(false)` on original parent object |
| 11 | call `0x324260(true)` on original parent object |
| >=12 | no action |

The five original callees have not yet been ported. The C++ interface requires
a backend supplied by the genuine reconstructed game engine; it contains
**no fake implementations**. Return-value ABI and exact semantics of those
callees still require decompilation before full gameplay integration.

The same ELF proves that main Player and Monkey vtables at `0x6B3918` and
`0x6B1498` each have 11 entries, sharing **three actual no-op virtual
methods** at `0x442028`, `0x442030` and `0x442048`.
These are *not* missing gameplay operations.

## Verification

- The static audit is `native/tools/verify_original_player_dispatch.py`:
  `python native/tools/verify_original_player_dispatch.py <PRIVATE_SCUS_975.01>`.
  It validates the actual ELF's SHA-256 and the exact original table contents.
- The C++ compiled and ran locally against a 12-case routing test using
  `g++ -std=c++20 -Wall -Wextra -Werror`.
  This verifies the reconstructed *dispatcher*, **not** the original
  game running on Windows.
- The Windows CMake target `Getchu3OriginalPlayerModeTests` is registered
  in CTest but has not been run in Windows CI. GitHub Actions was not invoked
  to avoid unwanted notifications.

Next: source-level recover `0x33FCE4`, `0x34675C`, `0x3509AC`,
`0x33FD90`, `0x324260`, then call this dispatcher from reconstructed
original Player lifecycle code, retaining the source game's Lua 5.0 scripts.
