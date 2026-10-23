# Original game engine: Player factory and manager/controller ownership

**Source:** exact Ape Escape 3 USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. This batch is a direct source-level translation of the game executable, not replacement gameplay.

## Main breakthrough: actual Player-to-manager connection

The original global factory at **0x647BD0** is registered with the **original Player object**. The original Player vtable **0x6B3918** has native method **0x34A014** at virtual slot 2. That method compares full **16-byte, zero-padded names**, and returns real Player-owned subobjects:

| Original name | Original return field | Original ELF name location |
|---|---|---|
| `player` | stored pointer at Player +0x5C | `0x7121E0` |
| `main_player` | stored pointer at Player +0x60 | `0x7121F0` |
| `item_chatcher` | embedded subobject at +0x64 | `0x712200` |
| `force` | embedded subobject at +0x180 | `0x712210` |
| `bar_interface` | embedded subobject at +0x70 | `0x712220` |
| `iZone` | embedded subobject at +0x110 | `0x712230` |
| `iHanger` | embedded subobject at +0x188 | `0x712240` |
| `iRope` | embedded subobject at +0x1A0 | `0x712250` |
| `iDashBall` | embedded subobject at +0x1D0 | `0x712260` |
| `iHeroSwitch` | embedded subobject at +0x258 | `0x712270` |

The last six comparisons only execute when real original global pointer getter **0x37D348** returns nonnull (global at `0x6A99BC`). Additional original methods `0x37D354` and `0x37D360` clear that pointer and read byte +0x8A0 when it exists. These functions have been translated too. Missing native Player subobjects remain null and are never faked.

**Original manager initializer `0x57BA00` loads the constant `main_player` from `0x708940`.** It used to call a generic mock factory virtual method in our host code. It now invokes the real recovered `0x34A014` lookup. It still calls the original candidate virtual slot 11 and native `0x21D898` registry selector through explicit unresolved dependencies.

The game sets this Player global during original Player initialization at **0x5B04B4**, and clears it at **0x5B0654 / 0x5B071C** during teardown. Four additional game initialization/teardown callers also use the same setter.

## Connected concrete controller class

The original two allocation wrappers **0x317538** and **0x31759C** both allocate **0x70 / 112 guest bytes** and call original constructors **0x57BA98 / 0x57BAF8**. The newly recovered host-native `OriginalManagerChild70` stores the known original fields, including parent +4, child +0x10, context +0x18, flag +0x1C (signed context[+4] >=2), byte +0x1D, and variant byte +0x30.

Both original constructors call **0x57BCB8**, whose recovered branch, VU0 vector operations, route selection, and original RNG handling are now translated in `native/src/original_manager_initializer.cpp`. The original actor owner/projection effects remain explicitly dependent on genuine game-manager implementations. Variant-one `0x57BAF8` now also preserves the source `+0x20` full 128-bit controller vector before invoking the initializer; variant-zero `0x57BA98` does not write that field. These native constructors still do **not** create a complete live Player/Monkey.

The genuine virtual table **0x6B1820** contains six methods. We translated the first five:

- `0x57BB60` regular destructor and `0x57BBB4` deleting destructor: invoke owned child virtual destructor and, when deleting, original free routine `0x41A3B0`
- `0x57BC0C` query: return 3 if parent +0x40 is set; otherwise call child virtual slot 2 or return 0
- `0x57BC54` forward original arguments to child virtual slot 4, or return its original third argument if no child
- `0x57BC88` invoke child virtual slot 3 if present

The sixth method **0x57C620** has now been audited in full against the verified USA ELF (107 exact instructions), and its source controller-method dispatch and candidate-polygon+0x50 handling are translated in `native/src/original_manager_dispatch_57c620.cpp`. The callee **0x30FFD0** is also translated (51 original instructions) and connected to original source VU0/grid polygon selection. The **0x57C554** type-2 controller child update is source-recovered as an additional 51-instruction native C++ function, including original child constructor/destructor and `0x3161B8` update sequencing. These methods still require original Player/Monkey owner method bodies, actor-owned VU0 and grid operands, exact source diagnostics and real actor-state publication—not a replacement Unreal controller.

## Verified and not yet verified

The owner-private ELF audit `native/tools/verify_original_player_manager_chain.py` passed **93 exact original instruction comparisons, 16 verified direct call/jump targets, two real vtables, and ten exact padded resource names**.

Locally the new Player lookup, child-controller C++, and the corrected manager readiness code passed three native optimized C++20 tests **and** the same three tests with UndefinedBehaviorSanitizer. These are unit tests of reconstructed code, **not** a running retail game. Windows compilation / real game startup have not been verified; no GitHub Actions were triggered.

The original **0x21D898** nearest-polygon calculation is translated and now callable from the mounted stage native bridge. The native original **0x21D46C** stage loader also exposes source-accurate 0x70 polygon records and pending type-1/type-2 descriptors (original record index, action slot, type and argument). This is sufficient to hand the genuine stage data to a future Player/Monkey actor manager, but **not** to implement the still-unrecovered actor spawning semantics by guesswork.

The largest remaining dependencies are the real persistent Player/Monkey factory and owner, original **0x57C620** controller state update, original actor-owned runtime input/route setup and complete update/render/animation integration. Original **0x57BCB8** is translated at source level but is not yet connected to a working live owner. The source executable and all game assets stay private.

## Direct source audit of original 0x57BF5C movement owner

`native/tools/verify_original_manager_movement_57bf5c.py` now
SHA-pins the **complete** original USA code window
`0x57BF5C..0x57C554` (382 MIPS instruction words) and verifies
all **18 original direct JAL call edges** with the exact
owner-private executable SHA-256. The function dispatches several
original dependencies including `0x316174`, `0x21B854`,
`0x441F40`, `0x36F5F4`, `0x30C57C`, `0x3161B8`,
`0x21DB64`, `0x3176AC`, `0x30C1D4`, `0x30C21C`,
`0x30C520`, and the recovered original route controller
`0x57C7CC` (called at `0x57C3D0`).

This preserves a concrete, reproducible source map for recovering
the remaining original type-zero/general movement owner. It does
**not** claim the 382 instruction body has been translated or
that the game is playable. The complete original-game movement
and owner lifecycle, including these dependent original methods,
remain required.
