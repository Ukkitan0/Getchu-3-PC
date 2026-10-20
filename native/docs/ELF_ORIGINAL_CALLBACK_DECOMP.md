# Decompilation of actual USA retail SCUS_975.01: first C++ game callbacks

This is **original game-engine analysis**, not asset recreation or a PlayStation emulator.

**Original executable:** 6,488,148 bytes; SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

All 17 known Lua registrations through function **0x255BD8** were followed into
their **actual original derived C++ vtables**. See
`native/reference/verified_original_retail_callbacks.json` for verified
registration site, vtable and implementation address for each game callback.
This advances beyond the earlier work, which recovered registration names
but had not followed them into actual methods.

## Decompiled original algorithms (USA executable)

These are direct, address-anchored reconstructions of selected method bodies:

- **get_costume_possess**, original callback `0x617924`:
  `return *(int32_t*)0x649948 == 2;` passed to the original Lua boolean
  push routine `0x3E6190`.
- **get_costume_possess_ninja**, original callback `0x617958`:
  `return *(int32_t*)0x649938 == 2;` via `0x3E6190`.
- Both addresses lie in verified original **Data** base `0x649910`.
  Original `Data+0x38` is `costume_possess[7]` and `Data+0x28`
  is `costume_possess[3]`. The original callbacks do not read a Lua
  argument. The old native `get_costume_possess(index)` approximation
  was demonstrably wrong and has been replaced by the recovered algorithm.
- **gw_set_flag**, callback `0x6175E8`: fetches argument one using
  `0x3E5C70` and passes the **constant true (1)** to original global
  flag setter `0x38545C`. The original has no second-argument flag value.
- **lw_set_flag**, callback `0x617728`: obtains name argument one.
  A second argument is interpreted as a boolean **only if Lua boolean
  type==1**; otherwise default true. Dispatches to `0x385368`.
- **area_set_flag**, callback `0x6177E4`: same optional boolean/default
  true pattern as `lw_set_flag`, backed by `0x271764` and
  `0x371424`.
- **gw_get_flag** `0x617618` calls original `0x385404`; both
  world/global flag helpers use the original manager pointer at
  `0x699A0C` (second table entry for gw).
- **is_survival_mode**, callback `0x6178F0`: delegates to game
  function `0x236928` (which performs a separate key-based lookup
  through `0x2374E8`), then pushes the result into Lua. We have
  **not** identified the complete state invariant; do not hardcode it.
- **stage_var_get_int**, callback `0x6176C8`: converts argument one
  through `0x35AB88`, obtains game stage storage via `0x271710`,
  looks up the value with `0x3713F8`, and returns it to Lua as a number.
  The native implementation must not invent a missing stage value.

## Integration status

The initial native C++ repair changes exactly those bindings whose return
values can be proven from the recovered original ELF and the existing Data
layout. Global/local/area flag **storage backends** still need porting;
their temporary per-stage map is not a complete implementation of original
persistent game flags.

These methods do not by themselves implement original player movement,
gadget logic, monkey AI, cameras or all game menus. No Windows retail
gameplay has been shown.

Next steps: trace original `new_ent` / `c_ctrl` registration through
other registry paths, recover original actor constructors, then map original
player/gadget/AI functions and translate them into audited native C++.
