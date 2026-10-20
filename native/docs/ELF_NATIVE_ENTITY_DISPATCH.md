# Direct decompilation: original entity and controller dispatch

**Source:** Original Ape Escape 3 USA `SCUS_975.01`, pinned SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

The prior 17 Lua bindings were C++ object/vtable registrations. Following
another **original game native registration path** at MIPS functions
`0x3E5F90` (register Lua name) and `0x3E60B8` (associate C callback)
recovered **40 direct registration pairs**, of which several repeat.

Original core Lua engine function addresses:

| Original Lua API | Original callback (MIPS text VA) |
|---|---|
| `new_ent` | `0x21498C` |
| `c_ctrl` | `0x219810` |
| `c_order` | `0x217E38` |
| `lua_param` | `0x219E4C` |
| `get_capture_flag` | `0x219BE0` |
| `set_player_weapon` | `0x512000` |
| `create_object` | `0x511674` |
| `create_bg` | `0x511E24` |

The original `new_ent` at `0x21498C` is a substantial native function,
not just a Lua descriptor callback. Its early engine code parses Lua
arguments and branches through name comparisons (`0x42C44C`). Static
analysis verified **64 original entity type string comparisons** in that
function region, including `Player`, `Monkey`, `Button`, `Cannon`,
`MatTrigger`, `RangeSwitch`, `Antlion`, `AlertTrigger` and cameras.

Original `c_ctrl` at `0x219810` similarly dispatches across **20
original controller operation names**, including `trig_wait`,
`monkey_cap_wait`, `anim_replay`, `weapon_spawn`, `message`,
`se`, `onDamage` and `wait`. The original `c_order` at
`0x217E38` builds a game-owned ordered array of controller references.

**Important:** These are verified real executable entry points and direct
string comparisons, not finished translations of the game state machines.
The engine's `Player` and `Monkey` constructors, actor update loops and
controller effects must now be followed and ported with their real data
structures and full behavior. Do not implement guessed substitutes.

Reproduce the registration/dispatch audit against an owner-private
`SCUS_975.01` using:
`python native/tools/recover_retail_elf_dispatch.py SCUS_975.01 --out private-report.json`.
The tool validates the exact original ELF SHA and outputs *only metadata*;
the original executable is never checked into Git.

Further references:
`native/reference/verified_original_retail_direct_bindings.json` and
`native/reference/verified_original_retail_callbacks.json`.

## Original Player and Monkey dispatch: first code-level trace

At original `new_ent` branch `0x214FD0`, the compiler compares the
requested class name against the original string **Player**
(`0x6E2178`). On equality, the original code passes the Lua state and
parsed entity arguments to the real game function **`0x20DAA4`**
through MIPS JAL at **`0x214FEC`**. On return, it branches to the
common original `new_ent` epilogue at `0x214A98`.

At comparison **`0x21508C`** the original class name **Monkey** is
matched. The Monkey branch is distinctly more involved than Player:
it calls the original argument structure parser **`0x20D70C`** at
`0x2150B0`, then game logic function **`0x20D514`** at
`0x2150C0`, and continues parsing more game-owned parameters.
Do not describe either target as a fully identified constructor until
their own subfunctions have been decompiled.

Original controller dispatcher `c_ctrl` enters at `0x219810` and
uses MIPS `jal 0x42C44C` string comparisons. All 20 recovered strings
in the inspected dispatch body:

`anim_replay`, `anim_roller_play`, `trig_access`,
`trig_wait`, `weapon_spawn`, `monkey_cap_wait`, `wait`,
`se`, `se_stop`, `add_entity_se`, `kick_entity_effect`,
`kick_entity_se`, `message`, `message_b`,
`mgs_signal_alert`, `message_wait`, `path_replay`,
`value`, `onDamage`, `print`.

**Why this matters:** these are actual code routes to the original
game's object and event behavior. The prior native implementation merely
captured Lua data; following these functions and reconstructing their
real side effects is now the main development direction.

## Expanded verified native function routing — 2026-10-20

Direct original ELF static analysis now identifies the **first early JAL
target for 45 of the 64** compared entity names, including many
original camera, mechanism, destructible, enemy, and stage-object handlers.
The remaining **18** names use shared dispatch
blocks or follow indirect/longer branches; see the explicit unresolved
list in `native/reference/original_entity_controller_routes.json`.

For controller actions, **19 of the 20** original commands jump directly
to a distinct native handler. Examples:

| Original controller opcode | Actual MIPS code target |
|---|---|
| `anim_replay` | `0x218A20` |
| `trig_wait` | `0x218E9C` |
| `weapon_spawn` | `0x218D38` |
| `monkey_cap_wait` | `0x218C48` |
| `se` | `0x2191D4` |
| `message` | `0x218F24` |
| `path_replay` | `0x2186E4` |
| `onDamage` | `0x217FE4` |

The `message_b` branch requires additional CFG inspection. The first
`Monkey` call at `0x20D70C` is a Lua parameter parser, NOT a proven
monkey constructor. The original `Player` wrapper at `0x20DAA4`
calls `0x35AB88`, `0x20D514`, `0x20D49C` and `0x34A70C`.
The last routine allocates a 0x260-byte game object and dispatches
original game initialization at `0x5B0258`. This is a function-level
reconstruction **in progress**, not playable player movement.

Two additional small original game functions are now translated directly
into native host C++ with unit tests:

- `get_sex_distinction` at `0x219C8C` reads `Data.player_type`
  at `0x649910` and pushes exact strings `boy`, `girl` or `nil`.
- `set_player_weapon` at `0x512000` maps string `empty` → 0,
  `net` → 1, `club` → 2, other → 3; the original setter
  `0x266CF4` stores the value at `[0x6496D8] + 0xBC`. Native
  code preserves the decoded selection, but the original game-owned
  object creation and active gadget behavior are still outstanding.

Exact route map generator, which verifies the original USA ELF hash:
`python native/tools/map_original_engine_routes.py SCUS_975.01 --out routes.json`.

## Verified real Player/Monkey factory and object-layout addresses

Deep disassembly of the original, hash-verified USA executable now traces
both **real class creation paths**, including their allocated object sizes
and initialized C++ vtable addresses. These facts go beyond merely
knowing the original Lua class strings.

| Original class | Lua wrapper | Game object factory | Allocation | Object initialization | Main vtable |
|---|---|---|---|---|---|
| Player | `0x20DAA4` | `0x34A70C` | `0x260` bytes | `0x5B0258` | `0x6B3918` |
| Monkey | in `0x21508C` branch | `0x315A0C` | `0x300` bytes | `0x5714B0` | `0x6B1498` |

The **Player** constructor writes its main vtable at offset `0`,
secondary vtable `0x6B3960` at offset `0x64`, and internal self
pointer at offset `0x68`. The constructor initializes several more
state fields. Its first two original virtual-method entrypoints are
`0x5B05CC` and `0x5B0694` — *their exact semantic names remain
undetermined*.

The **Monkey** factory first calls `0x3177D0` and `0x316F6C`.
Only if the latter returns nonzero does it request **0x300 bytes**
from the real game allocator `0x41B800` and invoke `0x5714B0`.
The Monkey initializer writes the main vtable `0x6B1498` at offset
`0`, secondary vtable `0x6B14D0` at offset `0x58`, and the self
pointer at offset `0x5C`. Its first two original virtual-method
targets are `0x571D0C` and `0x571D8C`.

The original Monkey Lua dispatch parses `tag_name`, `seq_init`,
`seq_idle`, `seq_esc` and `nav` before invoking the factory;
these strings were verified directly at the referenced original
ELF data addresses. They are not inferred replacement fields.

The complete verified C++ class-layout and virtual-method address
inventory, including 11 Player and 11 Monkey main vtable members,
is recorded without game bytes at
`native/reference/original_player_monkey_constructors.json`.

**Next actual decompilation target:** the virtual methods reachable
through these tables, their original update loops and their
dependencies. Do not present address recovery as reconstructed
animation/physics/AI or as a playable Windows game.
