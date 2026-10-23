# Retail Ape Escape 3 nonterminal movement-child setup

This records the verified original USA actor-controller code after a
nonterminal `0x36F954` result. It is native C++ recovery of original
game code, NOT an Unreal Actor approximation and NOT a playable game.

## Authoritative original binary

- Original private USA executable: `SCUS_975.01`
- Executable SHA-256:
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
- Complete original `0x317B9C..0x317BD4` function:
  **14 MIPS instruction words**, SHA-256
  `f707fecf862b59b269a01af92dfa8983b113f96aecb8ca9d876bb5bb333cf296`
- Original `0x557ED8..0x557F40` source window:
  **26 instruction words**, SHA-256
  `dc27cc5e5af609496f6e01ba0420d7aedb0a97ac3da5a3b409b24002733379d0`.
  The translated setup ends at **`0x557F30`**; this ELF window
  includes the next three boundary instructions through `0x557F3C`.
  The original `0x557F38` JAL invokes unrecovered `0x317B34`
  branches in some cases.
- Complete original `0x317B34..0x317B9C` float resource routine:
  **26 instruction words**, SHA-256
  `66e99b9cf7fadebdbb0a4befc31cdfce5aeefa3beba5c08787351c2ea9b09b4c`.

The private original executable was inspected directly in this session
and is never committed. The source auditors are:

- `native/tools/verify_original_normal_movement_557ed8.py`
- `native/tools/verify_original_manager_resource_float_317b34.py`

## Original resource selector `0x317B9C` — complete

`native/src/original_manager_resource_selector_317b9c.cpp`
reproduces all source branch outcomes:

| Original source parent fields | Retail resource pointer |
| --- | --- |
| `parent+0x40 == 1` | `*(parent+0x10) + 0x1C0` |
| Otherwise, `mode & 0xFF == 0` | `*(parent+0x10) + 0x220` |
| Otherwise, `*(parent+0x34) == 0` | `*(parent+0x10) + 0x120` |
| Otherwise | `*(parent+0x10) + 0x140` |

The first source branch must win even for zero mode. Original
`0x317BB4 BEQZL` reads `parent+0x34` **only** on the
nonzero-mode path. Genuine missing field data is refused there.
Source guest pointer addition wraps at 32 bits; native code does
not cast original guest virtual addresses to Windows pointers.

## Original `0x557ED8..0x557F30` — recovered preparation

For actual source child byte `+0xC0 == 0` and a nonterminal
`0x36F954` result, the native
`originalMovementNormalBegin_557ed8` reproduces these source
operations and their order:

1. Read source child mode byte at **`+0x09`** (`0x557EE0`).
2. Set original child status at **`+0xB4=0`**
   (`0x557EE4`).
3. Invoke completely recovered **`0x317B9C`**, with its
   original `0x557EEC` JAL delay slot setting
   original child **`+0xB8=0`**.
4. Call genuine **`0x276B68`** using original parent
   `+0x14` and the selected resource guest VA,
   `a2=0`, `a3=0`, `f12=0`.
5. Only if that source method returns nonzero, call genuine
   **`0x2767F4(parent+0x14)`**.
6. Always call original **`0x2773D4(parent+0x14,0,0)`**.
7. Set original child **`+0xC0=1`** (`0x557F2C`)
   and reload original mode (`0x557F30`).

The actual source-dependent `0x276B68`, `0x2767F4`
and `0x2773D4` methods are mandatory game-owner
callbacks; neither fake successful collisions nor a substitute
route manager are introduced.

`OriginalMovementChildLifecycle_557e30` can now execute this
normal setup **only** when an explicit genuine normal owner is
supplied. The default existing owner still fails closed.
After the verified setup, it refuses the unfinished original
`0x557F34` continuation. It does not publish a false
child-frame update; source controller status `+0x14`
is not changed by the exception.

The genuine source writes to `+0xB4/+0xB8` occur before
external route callbacks. If a required callback fails, those
earlier writes remain; `+0xC0` is not set until all setup
callbacks have completed. Native tests validate this original
side-effect ordering.

## Original `0x317B34` resource float — two complete branches

The next source call at `0x557F38` invokes `0x317B34`.
The native `originalManagerResourceFloatBits_317b34`
recovers its two source returns without VU0 math:

- `parent+0x40 == 1`: source `JR` delay slot loads
  **raw COP1 f0 bits** from
  `*(parent+0x08)+0x374`.
- Otherwise, original child mode low byte zero: source
  `JR` delay slot loads raw COP1 f0 bits from
  `*(parent+0x08)+0x3B4`.

When both are false, original `0x317B58..0x317B90`
requires the genuine source VU0/SQRT path. It is
explicitly **not recovered** in this translation.
There is no host-derived float approximation.

## Tests, known limitations, and next original targets

New native CTest targets in `native/unreal_bridge/CMakeLists.txt`:

- `OriginalManagerResourceSelector317B9C`
- `OriginalMovementNonterminalSetup557ED8`
- `OriginalManagerResourceFloat317B34`

Local GCC and Clang C++20 isolated tests using strict warnings and
UndefinedBehaviorSanitizer passed source branch, pointer-selection,
parent state, original callback ordering, field mutation and
failure propagation checks. Original ELF full-window fingerprints
and 8 corresponding branch/direct-call targets matched.

The full repository CTest suite and Windows/Unreal DLL remain
unverified. The next game-critical functions include original
`0x317B34` nonzero-mode VU0 arithmetic, real
`0x276B68/0x2767F4/0x2773D4` effects, and
`0x36FB18` movement integration after the resource metric.
Player/Monkey manager ownership, full stage animation,
renderer and a playable native game are still outstanding.
