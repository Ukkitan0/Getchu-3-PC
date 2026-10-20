# Decompiled original Monkey auxiliary virtual methods

**Source:** owner-private original USA `SCUS_975.01` (SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`).

The game's original Monkey secondary virtual table `0x6B14D0` contains
nine methods. Four can now be translated from *complete actual R5900 method
bodies*, without invented game logic.

| Original method | Source-level original behavior | Native C++ implementation |
|---|---|---|
| `0x571648` | Read `(*(this+4)+0x60)->word_08` | `originalMonkeyGetWord08` |
| `0x571658` | Read `(*(this+4)+0x60)->word_38` | `originalMonkeyGetWord38` |
| `0x5716C4` | Store byte **1** at `(*(this+4)+0x60)+0x50` | `originalMonkeySetFlag50` |
| `0x571688` | Follow resolved subobject `+0x20`, then two `+0x00` pointers; read float `+0x37C` and multiply by global `0x708278` | `originalMonkeyAngleToRadians` |

The factor in `0x708278` is **IEEE-754 0x3C8EFA34 = 0.01745329052209854f**,
the game's single-precision degrees-to-radians factor. We preserve its
exact binary representation, rather than substituting a double-precision
math constant.

The native C++ data layouts use confirmed `+0x08`, `+0x38`,
`+0x50` and `+0x37C` offsets, with unknown fields deliberately unnamed.
Caller-side ownership and the original multi-pointer resolution
must be restored from the Monkey constructor and host lifecycle code
before these operations can be invoked in authentic gameplay.

## Verification

The new script `native/tools/verify_original_monkey_accessors.py` checks
the real ELF hash, all five original R5900 instruction sequences,
nine actual vtable pointers, and the floating-point constant.

The source-only local C++ test, linked with the implementation, **passed**
using `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`.
The project CMake now builds `Getchu3OriginalMonkeyAccessorTests` for Windows
CTest. Windows native compilation has **not** been run, and no actual
monkey gameplay has been verified.

Remaining Monkey virtual methods `0x571668`, `0x5716A8`,
`0x5716D8`, `0x57173C`, `0x57178C` have deeper game
dependencies. Their behavior must be recovered rather than replaced.

## Original Monkey main gameplay method — recovered 8-slot cursor

Disassembled original Monkey **main virtual method** `0x571AA4`.
After its existing game conditional checks and original calls, a branch
updates the **32-bit word at Monkey object offset `0x2F0`**.

The exact original machine instructions at `0x571B88` through
`0x571BB4` load this field, compare it to `7`, increment once,
conditionally reset the value to `0` **only when the old value was 7**,
and store it back:

```cpp
const uint32_t previous = monkey.slot_index_2f0;
monkey.slot_index_2f0 = previous == 7 ? 0 : previous + uint32_t{1};
```

This deliberately does not substitute a guessed `(value+1)%8` for
out-of-range values. The original 32-bit wrap semantics are preserved.
Ported to `originalMonkeyAdvanceSlotIndex`, with the actual object's
field offset statically verified and regression tests for normal,
wrap-at-seven, out-of-range, and uint32 overflow cases.

**Limitation:** Only the proven slot update within `0x571AA4` has been
ported. Its surrounding effects, animation/AI calls and conditions
are not translated; callers must only invoke it when the original
conditions are reconstructed. The retail ELF verification tool
additionally checks the exact instruction words for this sequence.
