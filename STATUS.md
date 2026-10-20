# Getchu-3-PC — native Windows progress

**Updated October 20, 2026. NOT PLAYABLE.** No complete retail Ape Escape 3 stage has been verified running on Windows. The current engine is developmental, not a game release.

| Milestone | Status | Verified result / remaining gate |
|---|---|---|
| 0. Preserve and separate old recompilation work | In progress | 275 legacy files moved intact to `archive/recomp-attempt/` on a separate branch; Windows CI not yet rerun |
| 1. Extract owner-owned retail data | Done (offline) | Prior session recovered seven ISO parts, 2,174,091,264 bytes; 3,994 VFI entries and 168 original Lua chunks |
| 2. Mod-priority loose-file VFS | Not started | Original archive loading needs a validated loose-file override path |
| 3. Display recognizable real retail stage | Not verified | Asset extraction, GLB conversion and renderer exist; no Windows retail-level screenshot/run |
| 4. Trace original Lua bindings | In progress | Lua 5.0 interpreter and partial game bindings exist; all-stage call trace and complete engine callback semantics missing |
| 5. Bind original entities/models | In progress | Lua `new_ent` descriptors and native actor binding code exist; full original type/model mapping missing |
| 6. Authentic player/camera | Not started | Experimental walking test only; no original player behavior reconstructed |
| 7. Gadgets/capture/monkey AI | Not started | Original game logic still requires reverse engineering |
| 8. Title/menus/progression/save/audio | Not started | No complete original game-flow implementation |
| 9. End-to-end playable Windows game | Not started | Requires all prior gates and validation on actual retail content |

## Verified source evidence

The earlier private ISO analysis reported the original USA `SCUS_975.01` ELF as 6,488,148 bytes with SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. All 168 original `area.luc` scripts parsed offline (805 functions, 135,820 instructions). Seventeen game Lua registration sites were mapped through one ELF helper; they are **not all engine APIs**. See `native/reference/verified_original_retail_api_sites.json`. The prior private Woods C PCK sample contained authentic collision, model, animation and texture resources.

**Limit:** Neither the extracted original game assets nor the retail ELF is kept in this public repo. The latest code has not passed a fresh native Windows build against original retail assets. Synthetic test success is not gameplay success.

## Near-term work

1. Validate this cleanup with manual-only Windows CI; preserve it as a branch until verification.
2. Implement an owner-local mod overlay path without changing the original Lua runtime.
3. Run one authentic retail stage, log every missing Lua callback without inventing results, and map original entity assets.
4. Recover original player, camera, gadget and monkey behaviors from ELF evidence, then implement them as native C++.

No PS2 emulation, ELF static recompilation, or invented gameplay behavior is part of the active project.
