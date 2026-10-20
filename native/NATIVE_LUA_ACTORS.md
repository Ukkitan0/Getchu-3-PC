# Native Ape Escape 3 Lua actor world — player + multiple monkeys

**Current status: native engine actor spawning works; retail game logic is NOT playable yet.**

The game engine preserves original Lua 5.0 stage scripts. Calls to Lua
`new_ent(entityClass,{fields...})` become typed C++ `StageEntity` descriptors.
`NativeActorWorld` now converts matching entities to genuine host-native 3D
mesh instances: **one Player, multiple Monkeys and other entity classes can
all exist concurrently in one Direct3D11 world**.

## The complete code path

1. Original stage `area.lua` is executed by the embedded real host Lua 5.0
   interpreter, with `setupArea({spawn_tag=...})` and C++ `new_ent` callbacks.
2. Original scene geometry is independently loaded from original I3D assets
   converted offline to GLB through MIT ae3-sdk.
3. `NativeActorWorld::bind()` maps each Lua entity to a **private local GLB
   asset** through its literal `model_name`, unique entity name, or explicit
   owner-supplied mapping. The system does not pretend it knows which
   proprietary model represents a monkey if the mapping is missing.
4. Each actor owns **its own model and animation state**. The native engine
   preserves the world, adds actor vertices and material batches, and
   updates host CPU-skinned vertices using the same native D3D11 dynamic
   scene buffer. Actors cannot overwrite one another's meshes or clocks.
5. **Original Lua `tag_name` resolution** is now implemented. The
   recovered retail script creates Player and Monkey entities by their
   `tag_name`, often without explicit XYZ. We check `placeholder.plc`
   in the owner-private stage folder and use the exact unique matching
   placement record's XYZ (converting RH to D3D11 LH). Missing or
   ambiguous tag locations are explicitly marked **unverified** and
   are never inferred. Unsupported original PLC layouts do not prevent
   otherwise resolvable actor models from loading. Explicit synthetic
   `pos`, `position`, `xyz` or numeric `x,y,z` remain supported.
   These matching rules need validation against complete original stages.
6. The current F5 experimental native walking controller moves a bound
   `Player` entity's rendered model to its collision-tested position,
   while other actors retain separate positions and animation clocks.
7. F8 cycles animation clips on the first bound animated actor;
   F9 pauses/resumes native actor animations. This is an independent
   test interface, **not recovered original AE3 animation state logic**.

Resource safety includes rejecting absolute/path-escaping model bindings,
skipping ambiguous GLB filenames, blocking symlink traversal, limiting
actors to 64, checking combined GPU vertex/texture budgets, and returning
diagnostics for entities without assets.

## One-click actor demo — no commercial game data needed

From the updated repository, on Windows double-click:

    TEST_NATIVE_LUA_ACTORS_WINDOWS.bat

This generates a **completely original-free** test arena containing a
small procedural player proxy, two separate monkey-shaped proxies,
`area.lua`, `placeholder.plc`, and a local mapping file; compiles the
native engine and opens the example world with its Lua script.

These characters are simple independent test geometry and are **NOT**
Sony models, authentic monkey AI, the original physics controller, or
capture mechanics. The test demonstrates the integration needed for
original gameplay.

The native binary also supports two CLI positional arguments:

    Getchu3Native.exe "C:\AE3_PRIVATE\stage\glb" "C:\AE3_PRIVATE\stage\area.lua"

Or open the folder first with **Open GLB level folder**, then use
**Run original Lua 5.0 stage** to select `area.lua`.

A separate command-line check can count actors without opening a window:

    Getchu3ActorWorldProbe.exe "C:\AE3_PRIVATE\stage\glb" "C:\AE3_PRIVATE\stage\area.lua"

## Mapping your own game assets

Put an ordinary **local file** named `actor_bindings.txt` inside the
private converted GLB directory. It accepts case-insensitive plain
`name=relative_path.glb` pairs; one mapping per line. For example:

    # Give this specific player model to the Lua "player" entity
    player=player_character.glb
    # All Lua Monkey-class entities use this original monkey model
    class:Monkey=monkey_character.glb
    # A specific monkey can override that class mapping
    ape_cty_01=city_monkey_variant.glb

The examples are **filenames for illustration**, not guaranteed
names of actual retail AE3 models. Edit to match your own extracted
assets. Exact case-insensitive model-stem lookup also works when the
Lua `model_name` points uniquely to a GLB in the folder. Ambiguous
names are left unbound rather than guessing and spawning the wrong
object. No commercial model or converted original GLB is committed.

The private folder can contain original `placeholder.plc` for
authentic static-prop placement, and the original I3D_I3C can be loaded
separately using **Use original I3C collision**.

## Verified regression and limitations

A game-free native CTest executes real Lua 5.0 source that calls
`new_ent` for Player and two Monkeys, reads two synthetic GLBs,
loads an explicit private map, merges all three actors with one original-free
static geometry triangle, recovers one Monkey's exact PLC-tag spawn,
and verifies independent positions, host-native movement, material-batch
offsets, and blocked unsafe model paths.
Windows CI also generates a visually inspectable arena, invokes
`Getchu3ActorWorldProbe` against it, and publishes the legal GLB files
alongside the Windows executable.

**Not yet recovered**: the retail Sora/player model choice, real
player/monkey animation graphs, original AI/attention detection,
capture-net/gadget physics, hitboxes, camera logic, original object
lifecycle, stage triggers/progression, game menus/audio and original
save-game behavior. The Lua interface includes only some host bindings;
many original script calls may still report missing functions.

The present character/world scene is a working C++ native integration
test, NOT an authentic playable Ape Escape 3 level. The native
executable still does not link ps2-web-recomp, PCSX2, Sony's emulated
IOP or GS, or any PS2 guest machine code.

Two original-free skinned monkey test instances are also checked together
in Windows CI, verifying they own separate native skeletal animators.

Research evidence of real AE3 Lua `tag_name` for player and monkeys:
https://www.cnblogs.com/algonote/p/15678057.html
