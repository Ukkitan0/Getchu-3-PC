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
