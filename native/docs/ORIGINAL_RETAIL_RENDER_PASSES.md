# Original Ape Escape 3 USA graphics render traversal recovery — pass 2

**ELF:** owner-private retail USA `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
**Native target:** genuine source-driven graphics **control flow**, no PS2 emulator,
no replacement stage geometry, no guessed game camera or GPU draw implementation.

## Recovered original render-owner scheduling

Original `0x00280B7C`, reached from `calc render sub` path, operates on an
owner object with these fields verified in the original instructions:

| Original field | Meaning / source |
| --- | --- |
| owner `+0x34` | pointer to begin/end vector of records, stride **0x90** (`0x280BC4,0x280BD4,0x280C0C`) |
| owner `+0x38` | rendering schedule mode, special handling when **exactly 1** (`0x280C20–0x280C28`) |
| owner `+0x3C` | signed period, clamped to at least 1 (`0x280BC0–0x280BD8`) |
| owner `+0x40` | **64-bit** frame counter, incremented and written back every call (`0x280BB0–0x280BC0`) |
| record `+0x04` | when zero, calls `0x51CDAC` validator (`0x280BE8–0x280BF8`); then clears the byte |
| record `+0x05` | after validation, only nonzero records continue into render scheduling (`0x280BFC–0x280C04`) |

When mode is 1, the retail code calls `0x4233F8` with the 64-bit frame
counter and clamped period; `0x4233F8` wraps `0x423418`, returning the
64-bit remainder stored through its output pointer. The caller compares
`frame % period` to the **active-record ordinal** (not array index).
Inactive records do not advance this ordinal. Mode other than 1 schedules all
activated entries. Both the skipped and selected paths advance the ordinal.

New source-owned `original_render_owner_schedule.hpp/.cpp` encodes this
scheduler over decoded C++ owner/record values, returning scheduled original
IDs in the original vector order. Original `0x51CDAC` body remains
unrecovered: a callback must be bound when the record requires it; never
synthesize activation or treat arbitrary source bytes as models.

## Recovered primary render pass

`original_render_pass.hpp/.cpp` implements control flow and original call
order from `0x280810–0x280968`, plus the separate flag-33 false source branch
`0x280A90–0x280AB8`.

1. Object+0x20 AND owner+0x58, both **64-bit**; skip when zero.
2. Object bytes +0x30 and +0x35 must both be set.
3. When object+0x32 is set, call `0x26C9B8` and skip when it rejects.
4. Actor+0x110 resource wrapper `0x26B550` optionally enables original selector `0x26B57C`. When selected: call `0x2802D4` and `0x3B76F8(resource,savedSlot)`.
5. Object byte +0x33 true calls `0x2803B4`; false calls `0x3B76E8(resource,savedState)`.
6. Query `0x26B3D0` (actor resource+0x08 via `0x26A974`); if true call `0x2804F4`.
7. When object+0x34 is zero, call `0x3B76F8(resource,0)`.
8. Call `0x26831C(object,resource,mode=1)`.
9. **Re-query** original actor resource+0x08; if nonzero OR object+0x34==0, call `0x3B76F8(resource,savedSlot)`.

The two queries of `0x26B3D0` are deliberately separate: the intervening
original operations may change the resource's active byte. This bridge calls
the provided accessor each time, rather than caching a potentially stale flag.

## Recovered secondary render pass

Separate loop `0x280E20–0x280EB8` tests object+0x30 and +0x35 but
**not** the 64-bit group mask and **not** +0x32. It unconditionally calls
`0x26C9B8` for eligible objects. For an accepted object, +0x33 true calls
`0x2803B4`; false calls `0x3B76E8(resource,savedSlot)`. It then calls
`0x26831C(object,resource,mode=0)`. This is not interchangeable with the
primary pass, even when the same original actor occurs in both.

## Integration with native authentic meshes

`OriginalRetailPassTrace::acceptedIds` returns accepted source actor IDs in
their native traversal order, separate from ordered original routine events.
`OriginalRenderWorld::composeFrameInOriginalOrder(acceptedIds)` takes these
IDs and assembles their **actual owner-extracted GLB meshes**, vertex colors,
UVs, textures, animated host CPU poses, and per-actor world transforms into
the existing `StaticScene` D3D11-compatible upload layout **without host
re-sorting**. A skipped/invisible object contributes no geometry; duplicate or
missing actor IDs are explicit errors.

This is an integration seam for real game models, **not the original draw
microcode or complete original shader/matrix/effects pipeline**.
The source `0x2803B4`, `0x26831C`, `0x3B76F8`, `0x3B76E8`,
`0x2804F4` and bounding call `0x26C9B8` still need native GPU/VU
recovery and GPU binding; the trace identifies exactly where each belongs.

Illustrative integrator-side flow (must be placed only in Windows-owned code):

```cpp
auto tick=advanceOriginalRetailRenderOwner(realOwner, originalSlotInit);
auto first=traceOriginalPrimaryRenderPass(realStageActors, realMask58, providers);
auto second=traceOriginalSecondaryRenderPass(realStageActors, providers);
// Submit per-original-pass actors from the tick's genuine scheduled owner:
// derive original list/bucket membership from genuine stage objects before call.
auto stage=world.composeFrameInOriginalOrder(first.acceptedIds);
renderBackend.displayScene(stage.scene, true);
// Restore original render states / draw effects at emitted call boundaries;
// simple scene submission alone is NOT a visually correct retail render pass.
```

## Private retail instruction fingerprints

These exact source address ranges were SHA-256 checked against the owner's ELF
and added to `native/tools/audit_original_graphics.py` (end-exclusive):

- `0x280810–0x280968`: `2e5208f2d3a5011fb14547dd136a3024c2f5ae7d1cf88e0cf87d48725f4e5dc3`
- `0x280E20–0x280EB8`: `4a792023553cf070bf97e992aa560185e6992b88e4b1fd83fae4cff60ec9c9e0`
- `0x26B3D0–0x26B3FC`: `764c6db6a386166eab9621a815e42c60738affbeeee7d903e82b41bd4a187198`
- `0x26A968–0x26A980`: `ecbb590540e60a82dc670dfd606d0408498bc2d236b514168aa155e5bc34522b`
- `0x280A90–0x280ABC`: `09728d913e02ed6e44f0cadb6aea8a436931d3e70ec0407a2bc10da601565f61`
- `0x2802D4–0x2802F8`: `df96d533136361022447536b0e381b5ce9337cd0472f2f195b070785680e78ef`
- `0x26C9B8–0x26C9D4`: `c08bbe94a2e307a3200b51de2522332b0c72805da4effe414a0eec93ef6ceaa8`
- `0x280B7C–0x280C80`: `69dc32f2de97a12e4ee3a7a0c0cd4d35f38990c7434ba26d7c1d95f75289b190`
- `0x4233F8–0x423418`: `a70c622eefdea82e7f22aa193ffa23caa2fe4a23e6aff2dde2e04cb011ff0e2c`
- `0x26A974–0x26A980`: `e1370cb89f6e17b9fc6f89479d8d1eb39fb7129817893a16fd6de4b85116b75e`

## Tests and remaining limits

Tests: `original_render_pass_test.cpp`, `original_render_owner_schedule_test.cpp`,
extended `original_render_instances_test.cpp`, with existing retail gate/selector
tests. Five source-owned native test suites run with GCC -O2 -Werror, plus
Clang -O1 -fsanitize=address,undefined -Werror; **10/10 test executions pass**.

No original level was rendered, no full Windows executable boot occurred,
and the updated GLB/cgltf tests and complete CMake/CTest remain unrun here.
The user-owned ELF is private and is never shipped. Future work must
recover `0x26C4B0` VU0 spatial rejection, genuine actor+0x110 resource
binding, stage bucket ownership, original matrices/camera, render materials,
effect dispatch, and authentic asset+animation mappings before visual claims.
