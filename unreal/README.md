# Ape Escape 3 — Unreal integration (source work; no playable build)

Unreal Engine 5.6 Windows project and Editor plugin. This folder contains **no retail game data**.

## Recovered Lua 5.0 native runtime integration (new)

This source branch now has a **real Unreal game target**, a packaged-runtime
Getchu3OriginalLua plugin module and a versioned native C ABI v3 DLL compiled
from the existing original-game Lua 5.0 interpreter and retail bytecode
transcoder. See [original Lua engine bridge](docs/ORIGINAL_LUA_NATIVE_RUNTIME.md).

An Unreal GameInstanceSubsystem can pass an owner-extracted original
area.luc file and spawn tag into the source VM and forward calls to an
explicit Unreal original-manager provider. The provider refuses absent
original game functionality rather than guessing the game's mechanics.
A source-backed, exactly 1033-byte PlayerProgress snapshot may be supplied
by the future original save/game-state manager.

**Windows build workflow:** double-click [BuildAndOpen.bat](BuildAndOpen.bat)
with Unreal Engine 5.6, CMake, Visual Studio 2022 and Python 3 installed.
It installs pinned original asset conversion tools privately, builds the
Lua DLL and its contract tests, stages the native library, and builds/opens
the Unreal project. The original game ISO is still selected locally through
the importer.

The real actor manager, stage parameter manager, player/monkey update
dispatcher, gadgets, live sound, collision responses, menus and progression
are **still unimplemented**, and the game remains **not playable**.
No successful Unreal Editor build or original-game execution is claimed.

## Current scope (not a game release)

- A dockable **Ape Escape 3 — Unreal Importer** panel under Unreal Editor's Tools menu supports ISO selection and Windows file drag/drop.
- The importer verifies the original USA ISO size and full SHA-256, calls the genuine existing `native/tools/extract_retail_iso.py` source extraction code, verifies the source executable hash, and uses `native/tools/convert_models_to_glb.py` with the separately installed `ae3` CLI to convert original model geometry and skeletons. Original I3M animation-to-model associations are not guessed. Original TIM2 files are converted to PNG first (for source textures), source I3D_BIN models and rigs are converted to GLB **without speculative I3M animation pairing**, original I3D_I3C triangle geometry is converted into static OBJ collision meshes, and original EXST streams into WAVs. These supported asset types are imported through Unreal Editor AssetTools, with a private per-stage status report under `Saved/Getchu3Private`.
- An installed `ae3-sdk` command is required for model, TIM2, I3C and EXST conversion. Missing dependencies or unsupported models report failure; no replacement models are invented.
- Unported stage behavior, Lua bindings, actor AI, sound playback, cameras, UI and save systems are **not supplied**. The **Play Ape Escape 3** control is intentionally disabled. Asset conversion is not game execution.
- Imported original resources and generated Unreal assets remain owner-private. Never commit those assets or the extracted ISO, ELF or Lua.

## Requirements and usage

Windows, Unreal Engine **5.6**, supported C++ toolchain, **Python 3.10 or later**, and the owner's SHA-verified USA ISO. `PythonScriptPlugin` and `Interchange` are enabled. For model import, a separately installed [pxdl/ae3-sdk](https://github.com/pxdl/ae3-sdk) CLI must provide `ae3` on PATH. Unreal's embedded Editor Python can run the extraction scripts without separate Python installation.

Open `Getchu3.uproject`, build the Editor plugin if the editor prompts, then select Tools → **Ape Escape 3 — Unreal Importer**. Drag/drop or browse for an owned ISO; choose **Import Original ISO**. The editor remains occupied while assets are imported. Each attempt uses a new private workspace so retries do not overwrite original data.

## Source and validation

Existing native work, integration PRs and all specialist branches remain unchanged. Import status is in `Saved/Getchu3Private/Getchu3ImportStatus.json`. Offline unit tests: `python -m unittest discover -s unreal/tests -v`. Focused local Python media/collision regressions covering thirteen cases were run against equivalent committed conversion logic; this does not substitute for a Windows editor run. These tests do not prove Unreal Editor compilation, retail model coverage, visual parity, or game playability. A genuine Windows editor + owner ISO run is required before presenting this as an end-user importer.

**Do not offer a game executable from this source until original gameplay, rendering, input, audio, menus and progression have been ported and end-to-end verified.**

## Original engine integrations still missing

The current source branch converts a subset of authentic assets from the verified USA disc; it is not a full engine importer. It does **not** register original Lua game callbacks with Unreal, place actual stage game objects, implement original actor-specific collision response, run original Player/Monkey updates, or reproduce gadget/capture and stage progression. Stream WAVs are source conversions, not the original interactive sound mix. The original game is not playable. A successful asset import must never flip `gameplay_ready` to true.

No model/animation correctness or full source corpus acceptance has yet been verified inside Unreal 5.6. The original file and all generated binary media remain in private ignored workspace.

## Original stage and Unreal source-asset registries

The importer now writes two additional owner-private reports for every verified original ISO import:

- `Saved/Getchu3Private/<import-id>/getchu3_original_stages.json` records genuine `area.luc` identities and SHA-256 values, the source-proven PLC translation fields (raw original XYZ), and unambiguous model-name candidates. Original rotation/scale, dynamic actor creation and source coordinate-conversion semantics are **not** represented as finished.
- `Saved/Getchu3Private/<import-id>/getchu3_source_asset_registry.json` maps **each original converted model, texture, EXST stream and I3C mesh** to its actual imported Unreal UObject paths. Stage PLC links are resolved only when the original association is unambiguous and the matching model was imported. No synthetic actors or scenes are created. Unknown placements stay unresolved and remain explicit.

Both outputs remain private; neither is bundled with a game or stored in Git. Failed ISO hash validation now records the current error rather than displaying a previous successful import. Unreal Editor import operations display a cancellable progress dialog; cancellation leaves the attempt **blocked** and may leave already-imported assets, never a false success. The editor's **Play Ape Escape 3** button remains disabled.

## Validation boundary

The source tree includes unit/regression tests for PLC record parsing, original Lua source inventory, source-to-Unreal object identity, path collisions, cancellation, and SHA validation. They should be run on a clean checkout with `python -m unittest discover -s unreal/tests -v`. The newest stage/registry changes have **not** been executed in Unreal or in the owner-ISO pipeline here. The project does not yet compile into a verified playable Windows game.

### Fidelity gate: original animation binding

The ae3-sdk command supports a heuristic `--anims` auto-pairing mode, but original retail model-to-animation ownership has not been recovered sufficiently for frame-accurate game use. This importer deliberately does **not** enable heuristic pairing. It retains the original I3M files privately for source-backed association and later precise animation import. This is an intentional correctness restriction, not an animation implementation.

## Original stage PCK extraction (implemented in this branch)

The owner-ISO importer now unpacks the genuine stage `.pck` member index **before** attempting texture, model and collision conversions. This fixes a major source-data gap: genuine I3D models, I3M animation sources, TIM2 textures and I3D_I3C collision are often members of the PCK, not direct standalone VFI files. The existing `native/tools/unpack_original_stage_pck.py` is reused, and the Unreal pipeline explicitly requests `include_collision=True` to expose the source collision bytes for I3C-to-OBJ conversion.

Unreal's opt-in collision path writes the original `I3D_I3C\x00` bytes into private `.i3d` members, preserving their magic identity. The preexisting native runtime's default PCK behavior **still leaves I3C inside PCK**: the new option does not change that path. All extracted and converted files stay in `Saved/Getchu3Private` and are excluded from source control. A missing stage PCK reports a blocked source-data import, never an invented model or collision mesh.

The integrated Windows build now runs both `unreal/tests` and the original PCK extractor regressions. Locally, equivalent structural PCK extraction logic passed six synthetic-fixture checks for identity preservation, collision opt-in/default behavior, repeated extraction, upgrade from the native mode and corrupt input rejection. The full repository test suite and genuine owner ISO-to-Unreal import have **not** yet run here.

### Original material names from PCK

The pinned `ae3-sdk gltf` converter resolves image materials by their original TIM2 basename (for example `fur.png`), while the true PCK extractor names members with an archive index (`0003_fur.tm2`). The importer now creates **owner-private, source-pack-local** material aliases after TIM2 decoding. A model from `pck_assets/bg` can only resolve those aliases under its own original `pck_assets/bg` directory, not a different archive or another stage. A duplicate original material name in the same pack with different pixels is explicitly rejected. If a PCK model has no verified pack-local material, its conversion records this limitation instead of borrowing a plausible texture. No original game source file is renamed or changed.

The `material_aliases` directory remains owner-private and is used by the source GLB converter; imported PNG assets continue to retain their original indexed identities.

## Imported stage selection without manual Lua paths

The ISO import pipeline now hashes and validates every extracted source
`area.luc` referenced in the original stage catalog. It writes an
owner-private `getchu3_verified_stage_index.json` and records verified script
counts in `Getchu3ImportStatus.json`. The Unreal original Lua subsystem now
offers `StartImportedOriginalStage(StageId, SpawnTag, OriginalManager)`, which
resolves the exact stage from the verified import and checks SHA-256 again
before executing the actual Lua 5.0 bytecode. Direct arbitrary file-path
launch is not exposed to Blueprints.

This removes manual source script browsing. The original actor, parameters,
AI, gadget, collision and progression managers remain unfinished, so it
**does not enable gameplay or the disabled Play button**. The new integrity
tests passed locally with synthetic structural scripts; no genuine retail
ISO or Windows Unreal build has been verified in this environment.

## Original native callback gaps — actionable per-stage report

After owner-ISO import, if the verified original `SCUS_975.01` Lua
registration audit succeeds, the importer writes the private file
`getchu3_original_native_stage_readiness.json` under the current
`Saved/Getchu3Private/<import-id>/` workspace.

The Unreal import panel exposes **View Original Gameplay Binding Gaps**,
which opens the owner-private report. The source comparison distinguishes:

- Direct Lua function calls missing from recovered native host registrations;
- Original USA ELF CreateBind registrations that still have no host callback;
- Functions already registered by name but whose **original behavior is not
  verified**;
- Original script-defined functions, original actor class literals and
  controller commands per stage; and
- Source-verified PLC-to-imported-model associations versus unresolved
  placements.

Each result is stage-specific; information from another stage is not
silently assigned to it. The parser uses the existing original USA executable
and Lua bytecode audit tools. It never reports a stage playable based on a
function name match, imported mesh, or recovered address.

If the ELF/source audit is unavailable, the importer records an explicit
blocked diagnostics status while leaving independent owner-asset conversion
available. The report-opening control stays disabled until a verified
owner-private report exists. No original game data is included in Git.
New synthetic evidence regressions cover missing callbacks, duplicate
original registrations, unknown stage scripts, conflicting source-asset
inventories, and stage isolation.

## Isolated recovered Lua build verification

The separate `native/unreal_bridge` CMake project now uses a pinned,
SHA-256-verified official Lua 5.0.3 source archive. Modern
`FetchContent_MakeAvailable` is used correctly for a source archive
without its own CMakeLists.txt; the archive layout is checked before
compiling. On Linux CMake 3.31, the no-CMake upstream source handling was
tested with a local source override; two structural CTest targets passed
using synthetic stub sources. **Those tests do not establish that the
real native Lua engine or the Unreal project compile or run.**

## Recovered original player health routine

A native source-backed gameplay operation is now available:
`UGetchu3OriginalLuaSubsystem::ApplyOriginalHealthDelta`. It invokes the
recovered original `ChangeHP` routine on the game's packed
**1,033-byte PlayerProgress** structure (life at byte offset 8),
clamping health to the original 0–100 range. Unreal itself does **not**
invent a second health component.

The caller must have successfully started a source-backed stage with an
actual Player manager capable of reading and atomically committing the entire
original PlayerProgress record. The default
`ReadOriginalPlayerProgress` / `WriteOriginalPlayerProgress` methods
return false, so **without that original game manager, the operation
correctly refuses to change player health**. The native C ABI has been
versioned to v4 and the one-click Windows build runs the added
`OriginalChangeHPSourceStateReadWrite` regression. It verifies health
damage/healing, bounds, preservation of unrelated original capture/shop
bytes, and failed/no-op writes. As always, tests and actual UE5.6 Win64
build remain unverified here.
