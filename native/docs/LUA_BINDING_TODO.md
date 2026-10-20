# Original retail Lua — native engine binding recovery

**Status: research TODO.** These are names from original-owner bytecode
analysis and the pinned USA ELF. They are **not claims of working gameplay**.

## Most frequent Lua global reads in 168 original area.luc scripts

| Global | Read sites | Current known handling |
|---|---:|---|
| `c_ctrl` | 7,018 | Records a controller command table, not the game behavior |
| `new_ent` | 6,572 | Captures a native entity descriptor; unknown types still need engine implementations |
| `c_order` | 1,893 | Native timeline schedules ordered commands; effects largely not implemented |
| `lua_param` | 1,460 | Retains script parameters |
| `gw_get_flag` | 191 | Partial native flag map; persistence/default semantics need retail evidence |
| `area_get_flag` | 64 | Partial native stage state map |
| `get_capture_flag` | 56 | Reads a verified recovered capture-flag structure when available |
| `createTutorialExit` | 31 | Original registration known; behavior not implemented |
| `LocaleInfo_getID` | 19 | Original registration known; platform/locale semantics need investigation |
| `createFeatureLauncher` | 15 | Original registration known; behavior not implemented |
| `stage_var_get_int` | 13 | Explicitly unsupported for retail chunks; no invented value |
| `createWarpGateLauncher` | 1 | Original registration known; behavior not implemented |

Counts are static GETGLOBAL sites, not proven runtime invocation counts.
Names can be registered via **other** ELF paths; the known 17-site
`0x255BD8` registration list is not exhaustive.

## Next investigation steps

1. Execute one **real retail** stage on the host-native Lua 5.0 runtime
   with a call-order logger for unsupported callbacks; log arguments and
   stop when an unknown callback needs a return value.
2. Enumerate `new_ent` entity class names and `c_ctrl` commands from all
   original stage scripts; prioritize most frequently used ones.
3. For each callback, record ELF function addresses and observed argument
   types in `native/docs/ELF_NOTES.md`, with exact evidence.
4. Implement only well-supported native semantics; test against original
   state expectations and stage data. Preserve the Lua layer unchanged.

Reference evidence: `native/reference/verified_original_retail_api_sites.json`.
No original copyrighted Lua or ELF bytes should be committed.
