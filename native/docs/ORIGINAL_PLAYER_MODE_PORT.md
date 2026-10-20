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

## Decompiled genuine chip counter accumulation

The Player selector case 0, 1, or 2 invokes original function
`0x33FCE4` with amount **1**, **5**, or **10**. That function accesses
original Data at `0x649910` and loads its counter at `0x6499D4`,
which equals `Data + 0xC4` = the recovered `chip_count` field.

Its directly recovered arithmetic is:

```cpp
int32_t pending = wrap32(originalData.chip_count + gained);
int32_t threshold = survivalMode ? 1000
                  : specialHundredLimitFlag ? 100 : -1;
if (threshold > 0 && pending >= threshold) {
    // Original 0x33FD90 and 0x349B2C side effects occur here.
    pending = wrap32(pending - threshold);  // only once, not modulo
}
// Original 0x26FB24 stores/clamps counters;
// original 0x2915EC forwards the display/update event.
```

The arithmetic is implemented in
`originalChipGainBeforeClamp`, keeping its return value explicitly
named *pendingBeforeFinalClamp*. The actual original `0x26FB24`
subroutine **writes both** `Data+0xC4` and `Data+0xC8`
(`chip_count` and `max_chip_count`) using R5900 packed instructions.
It has now been **translated and compiled as native C++**, including
its R5900 packed-word saturation instructions. The remaining event/UI
side effects are not implemented and are not claimed to work.

The 100-limit flag was read at original `0x647930`, and the 1,000
override comes from the same original `0x236928` routine used by
`is_survival_mode`. The conditional wrap also invokes actual
`0x33FD90` and `0x349B2C` (effects not yet fully recovered).

A native, release-mode C++ test covers 100/1,000 threshold crossings,
the no-limit case, a single wrap with a large increment, and two's
complement 32-bit overflow. This is real source-level recovery of a
game counter algorithm, **not a fake collectible or an operational gadget**.

## Completed original storage routine `0x26FB24`

Recovered the real R5900 MMI instructions using original ELF instruction
words and the published R5900 opcode tables (PMAXW, PMINW). The exact
source-level storage operation is:

```cpp
// Incoming value is the pending count after the original threshold logic.
Data.max_chip_count = std::max(Data.max_chip_count, pending);
Data.chip_count = std::min(std::max(pending, 0), 9999);
```

This faithfully maps original `Data+0xC8` and `Data+0xC4` writes
at `0x26FB48` and `0x26FB50`, respectively. Note that the
high-water `max_chip_count` is **not clipped to 9,999** in this
routine, while the current count is.

Implemented as `applyOriginalChipGainDataWrites(PlayerProgress&, ...)`,
so the code now updates the **actual reconstructed Data fields** rather
than an isolated return value. A separate C++20 release-mode test verifies
normal counting, wrapping at 100/1,000, negative values, the 9,999
clip, high-water updates and original 32-bit arithmetic overflow.

The **event and HUD calls** `0x33FD90`, `0x349B2C`,
`0x2915EC` and their dependencies remain **unimplemented**.
The method returns `crossedThreshold` to expose that required
work; it does not invent rewards or silently claim gameplay.

R5900 opcode reference:
https://github.com/PCSX2/pcsx2/blob/master/pcsx2/DebugTools/DisR5900asm.cpp
