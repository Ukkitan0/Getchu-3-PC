# Ape Escape 3 original game RNG — native recovery of seven connected routines

The original United States executable `SCUS_975.01` owns the exact
state and functions below. Original ELF SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Connected retail source routines
- `0x36F3C0`: seed 624 uint32 words with multiplier **1812433253**
  (`0x6C078965`), reset original cursor+0x9C0 to zero, and run exactly
  623 original one-step twists before normal output.
- `0x36F2DC`: original in-place MT19937 state twist, using breakpoint
  index 227, partner 397, and odd-bit XOR `0x9908B0DF`.
- `0x36F444`: snapshot source state word at original cursor BEFORE
  calling `0x36F2DC`, then original temper shifts (11, 7, 15, 18).
- `0x36F4AC`: masked nonnegative 31-bit integer `& 0x7FFFFFFF`.
- `0x36F4D4`: original CVT.S.W followed by source float multiplier
  at `0x7157A8`, raw float bits `0x30000000` (2^-31).
- `0x36F508`: original signed SUBU/DIV/MFHI integer range with
  signed-remainder semantics. Native version throws rather than
  executing undefined division by zero.
- `0x36F548`: original f12 + float_random*(f13-f12).

The source default seed is exactly `0x1571` (5489). This state
machine uses MT19937 parameters, but **simply substituting
`std::mt19937` would be incorrect** because of retail warm-up and
snapshot/update ordering.

## Working native C++

`native/include/ae3/original_game_random.hpp` and
`native/src/original_game_random.cpp` implement the full chain,
including original bitwise shifts, wrapping cursor, stream continuity,
integer and float transforms. This is native typed C++, no MIPS
interpreter or emulated EE memory.

`native/tests/original_game_random_test.cpp` covers multiple
seed-specific, deterministic, exact 32-bit expected output sequences,
original cursor wrapping, original float bit pattern, both range
methods, and continued shared state across actor-target samples.

Owner-private ELF verifier `native/tools/verify_original_game_random.py`
SHA-pins all three original regions, checks **148 instructions**,
**43 individual opcodes**, and **six original call edges**.

Tests executed locally using GCC, Clang, and UBSan. These original
retail routines support actor target selection: e.g. `0x57C9BC` and
`0x57C9E0` call original `0x36F4D4` / `0x36F548` and must consume
the same original mutable RNG state.

## Scope boundary

Native game RNG is now reconstructed; the original VU0 geometry that
USES RNG results at `0x57C98C` and `0x57CA68` is not yet translated.
This commit is not a claim of playable actor movement or Windows boot.
No copyrighted executable or game asset is committed; no workflow
dispatch or emulator was used.
