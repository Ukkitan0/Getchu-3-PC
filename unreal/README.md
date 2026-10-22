# Ape Escape 3 — Unreal integration (source work; no playable build)

Unreal Engine 5.6 Windows project and Editor plugin. This folder contains **no retail game data**.

## Current scope (not a game release)

- A dockable **Ape Escape 3 — Unreal Importer** panel under Unreal Editor's Tools menu supports ISO selection and Windows file drag/drop.
- The importer verifies the original USA ISO size and full SHA-256, calls the genuine existing `native/tools/extract_retail_iso.py` source extraction code, verifies the source executable hash, and uses `native/tools/convert_models_to_glb.py` with the separately installed `ae3` CLI to convert original model geometry and skeletons. Original I3M animation-to-model associations are not guessed. Original TIM2 files are converted to PNG first (for source textures), source I3D_BIN models and rigs are converted to GLB **without speculative I3M animation pairing**, original I3D_I3C triangle geometry is converted into static OBJ collision meshes, and original EXST streams into WAVs. These supported asset types are imported through Unreal Editor AssetTools, with a private per-stage status report under `Saved/Getchu3Private`.
- An installed `ae3-sdk` command is required for model, TIM2, I3C and EXST conversion. Missing dependencies or unsupported models report failure; no replacement models are invented.
- Unported stage behavior, Lua bindings, actor AI, sound playback, cameras, UI and save systems are **not supplied**. The **Play Ape Escape 3** control is intentionally disabled. Asset conversion is not game execution.
- Imported original resources and generated Unreal assets remain owner-private. Never commit those assets or the extracted ISO, ELF or Lua.

## Requirements and usage

Windows, Unreal Engine **5.6**, supported C++ toolchain and the owner's SHA-verified USA ISO. `PythonScriptPlugin` and `Interchange` are enabled. For model import, a separately installed [pxdl/ae3-sdk](https://github.com/pxdl/ae3-sdk) CLI must provide `ae3` on PATH. Unreal's embedded Editor Python can run the extraction scripts without separate Python installation.

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
