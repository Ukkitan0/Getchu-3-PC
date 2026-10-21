# Ape Escape 3 USA — original actor route-following update 0x57C7CC

## Owner-original inputs, never published

Private retail original:
- `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
- `woods_c/navigation.pck` SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`

Source ELF is the US original, not rebuilt emulator code. MIPS Virtual Addresses are original executable addresses. Runtime route pointers are translated to stable native addresses in actual decoded original `OriginalSpatialRecord70` objects, rather than fabricating guest memory.

## Recovered exact route traversal

This source routine was discovered by following the actual original actor controller's call from `0x57BF28` into `0x57C7CC`. Its 72 retail instructions (`0x57C7CC..0x57C8E8`) are SHA-pinned.

1. `0x57C7F4..0x57C800`: load actor+0x04->+0x30 stage polygon, and original controller byte +0x1C.
2. If original `controller[0x1C]` is **zero**, `0x57C8D8..0x57C8E0` writes the stage polygon directly to controller+0x50, calls `0x57C8EC` exactly once, returns.
3. If the multi-polygon flag is set and a controller target already exists at +0x50, `0x57C804..0x57C828` calls the original `0x4C1B3C` polygon projection. An equal pointer returns without any target update.
4. Otherwise `0x57C82C..0x57C844` selects **route[1]** (NOT route[0]), writes it to controller+0x50, and applies `0x57C8EC` without first projecting route[1].
5. Starting at index **2**, `0x57C848..0x57C8B4` applies each subsequent source polygon using `0x57C8EC`, then calls `0x4C1B3C`; **only if the projection output equals the candidate** does it advance controller+0x50. On the first mismatch it stops, retaining the last successfully matched target. The projection is performed AFTER the candidate's source target calculation.

This control flow is now C++ in `originalActorAdvanceRoute_57c7cc`, with native record identities and bounds checks around original array indexing. It does **not** create any navigation route, hypothetical movement target, or artificial stepping algorithm.

## Still unresolved, with explicit call interfaces

Source `0x4C1B3C` uses VU0 and original `0x21BD3C` to project actor motion into source polygon geometry; it is **not** the same as simply comparing the actor's raw position with the destination.

Source `0x57C8EC` computes the current target vector using original `0x21B788`, `0x21B458`, and `0x57CA68` depending on controller variant and polygon geometry.

The newly added native route-following routine demands original callback implementations for both. Tests inject deterministic callback outcomes **only to validate source branching and pointer behavior**, not to claim the callbacks or full actor motion are reconstructed. Do not replace these calculations with heuristic stand-ins.

## Verification

`native/tools/verify_original_actor_route_follower.py` checks 206 original retail instruction words across three whole SHA-pinned regions, 33 individual opcode anchors, eight original call edges, and seven branch destinations. It also validates both genuine `woods_c` spatial graphs.

`native/tests/original_actor_route_follower_test.cpp` covers the following actual source cases:

- Single-polygon path uses actor stage polygon and does not project.
- The previous target remains unchanged when projection matches.
- The candidate is applied *before* projection; a mismatch retains the last confirmed polygon and ends the scan.
- All later route entries can be accepted when each source projection matches.
- Route arrays with only two source polygons do not project after the first target selection.
- Host bounds/owner identity validation.
- **All 137 original edges** from both true `woods_c` graphs, including all **15 special type-2 links**, when the owner-private original PCK path is configured in CMake.

Tested locally with GCC, Clang, ASan and UBSan. No original assets/ELF are checked in or redistributed. No GitHub Actions or emulation requested or triggered.

The complete **native Windows** actor update loop is not wired/booted. The remaining original `0x4C1B3C` and `0x57C8EC` vector implementations are blocking authentic actor travel; please pursue them next.
