# Ape Escape 3 — Unreal integration (source work; no playable build)

Unreal Engine 5.6 Windows project and Editor plugin. This folder contains **no retail game data**.

## Current scope (not a game release)

- A dockable **Ape Escape 3 — Unreal Importer** panel under Unreal Editor's Tools menu supports ISO selection and Windows file drag/drop.
- The importer verifies the original USA ISO size and full SHA-256, calls the genuine existing `native/tools/extract_retail_iso.py` source extraction code, verifies the source executable hash, and uses `native/tools/convert_models_to_glb.py` with the separately installed `ae3` CLI to convert original model and animation resources. Original TIM2 files are converted to PNG first (for source textures), source I3D_BIN models are converted to GLB, original I3D_I3C triangle geometry is converted into static OBJ collision meshes, and original EXST streams into WAVs. These supported asset types are imported through Unreal Editor AssetTools, with a private per-stage status report under `Saved/Getchu3Private`.
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
