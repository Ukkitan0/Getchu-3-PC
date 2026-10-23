"""Owner-local original AE3 ISO -> Unreal Editor asset import.

This editor-only module intentionally refuses to label asset extraction as gameplay.
No commercial files are stored in the source repository or embedded in the plugin.
"""
from __future__ import annotations

from contextlib import nullcontext
from dataclasses import dataclass
from pathlib import Path
import hashlib
import importlib
import json
import shutil
import sys
import tempfile
from typing import Any

USA_ISO_SHA256 = "5b4c6272c9daf190b3f72081f124edd7ea2af3cc0dafeaef206ded97a022fd09"
USA_ISO_BYTES = 2_174_091_264
USA_ELF_SHA256 = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"


@dataclass(frozen=True)
class VerifiedDisc:
    path: Path
    digest: str


def verify_disc(iso: Path, *, expected_digest: str = USA_ISO_SHA256,
                expected_size: int = USA_ISO_BYTES) -> VerifiedDisc:
    iso = Path(iso).expanduser().resolve(strict=True)
    if not iso.is_file() or iso.suffix.casefold() != ".iso":
        raise ValueError("Select an original Ape Escape 3 USA ISO file")
    if iso.stat().st_size != expected_size:
        raise ValueError("Disc size does not match the verified Ape Escape 3 USA release")
    checksum = hashlib.sha256()
    with iso.open("rb") as stream:
        for chunk in iter(lambda: stream.read(8 * 1024 * 1024), b""):
            checksum.update(chunk)
    if checksum.hexdigest() != expected_digest:
        raise ValueError("Disc SHA-256 does not match the verified Ape Escape 3 USA ISO")
    return VerifiedDisc(iso, checksum.hexdigest())


def locate_repo() -> Path:
    for folder in Path(__file__).resolve().parents:
        if (folder / "native/tools/extract_retail_iso.py").is_file():
            return folder
    raise RuntimeError("Native ISO recovery tools are missing from this project checkout")


def recover_original(iso: Path, output: Path, repo: Path) -> dict[str, Any]:
    """Calls actual preexisting original-disc extraction; never synthetic assets."""
    tools = repo / "native" / "tools"
    sys.path.insert(0, str(tools))
    try:
        module = importlib.import_module("extract_retail_iso")
        result = module.extract(iso, output, all_resources=True, include_elf=True)
    finally:
        sys.path.remove(str(tools))
    if result.get("original_vfi_files_extracted", 0) < 1:
        raise RuntimeError("Original ISO produced no game files")
    executable = output / "SCUS_975.01"
    if hashlib.sha256(executable.read_bytes()).hexdigest() != USA_ELF_SHA256:
        raise RuntimeError("Original SCUS_975.01 hash mismatch")
    return result


def unpack_original_pck_for_unreal(source: Path, repo: Path) -> dict[str, Any]:
    """Expose genuine stage members BEFORE model/texture/collision conversion.

    The existing native source extractor keeps I3C in PCK by default; Unreal
    explicitly requests those bytes so the original I3C geometry can be
    imported without emulation or invented collision.
    """
    tools = repo / "native" / "tools"
    sys.path.insert(0, str(tools))
    try:
        module = importlib.import_module("unpack_original_stage_pck")
        result = module.unpack(source, include_collision=True)
    finally:
        sys.path.remove(str(tools))
    if result.get("original_stage_pck_archives", 0) < 1:
        raise RuntimeError("No genuine stage PCK archives found in owner-original ISO")
    if result.get("owner_private_i3c_extraction_requested") is not True:
        raise RuntimeError("Original I3C triangle members were not requested from stage PCK")
    return result


def convert_original_models(source: Path, destination: Path, repo: Path,
                            ae3_cli: str, *, textures: Path | None = None,
                            pack_texture_aliases: Path | None = None) -> dict[str, Any]:
    if not ae3_cli or (not shutil.which(ae3_cli) and not Path(ae3_cli).is_file()):
        raise FileNotFoundError("The ae3-sdk 'ae3' model converter is not installed; refusing to invent models")
    tools = repo / "native" / "tools"
    sys.path.insert(0, str(tools))
    try:
        module = importlib.import_module("convert_models_to_glb")
        # The ae3-sdk --anims auto-pair path is heuristic, NOT original
        # model-to-clip ownership. Avoid silently attaching wrong animations.
        outcome = module.export_models(source, destination, ae3_cli, animations=False,
                                       textures=textures,
                                       pack_texture_aliases=pack_texture_aliases)
    finally:
        sys.path.remove(str(tools))
    failures = outcome.get("results", {}).get("failed", 0)
    if failures or outcome.get("original_i3d_bin_files", 0) == 0:
        raise RuntimeError(f"Original 3D model conversion incomplete: failed={failures}; "
                           f"detected={outcome.get('original_i3d_bin_files', 0)}")
    return outcome


def _unreal_name(text: str) -> str:
    """Deterministically sanitize a source package segment without losing identity."""
    if not text or text in (".", ".."):
        raise ValueError("Original source asset has an invalid Unreal name")
    cleaned = "".join(
        c if c.isascii() and (c.isalnum() or c == "_") else "_"
        for c in text)
    if not cleaned.strip("_") or len(cleaned) > 128:
        raise ValueError(f"Original source asset name cannot safely map to Unreal: {text!r}")
    return cleaned


def _planned_imports(folder: Path, files: list[Path],
                     destination: str) -> list[tuple[Path, Path, str, str]]:
    """Check all source-to-package collisions BEFORE the first Unreal import."""
    plans = []
    packages = set()
    for file in files:
        if file.is_symlink() or not file.is_file():
            raise ValueError(f"Source asset cannot be a symlink: {file}")
        relative = file.relative_to(folder)
        parent = [_unreal_name(part) for part in relative.parent.parts
                  if part not in (".", "")]
        target = destination + ("/" + "/".join(parent) if parent else "")
        stem = _unreal_name(file.stem)
        key = (target + "/" + stem).casefold()
        if key in packages:
            raise RuntimeError(
                f"Original sources map to an identical Unreal package: {relative}")
        packages.add(key)
        plans.append((file, relative, target, stem))
    return plans


def import_assets_in_editor(folder: Path, ue: Any, suffix: str,
                            destination: str, *,
                            source_records: list[dict] | None = None) -> list[str]:
    files = sorted(p for p in folder.rglob("*") if p.is_file() and p.suffix.lower() == suffix)
    if not files and suffix == ".glb":
        raise RuntimeError("No converted original GLB files were found")
    plans = _planned_imports(folder, files, destination)
    imported: list[str] = []
    observed_objects: set[str] = set()
    manager = ue.AssetToolsHelpers.get_asset_tools()
    progress = ue.ScopedSlowTask(len(plans), "Importing original Ape Escape 3 assets") if hasattr(ue, "ScopedSlowTask") and plans else None
    with (progress if progress is not None else nullcontext()):
        if progress is not None:
            progress.make_dialog(True)
        for file, relative, target, clean_stem in plans:
            if progress is not None:
                if progress.should_cancel():
                    raise InterruptedError("Original assets import cancelled; prior imported assets may remain")
                progress.enter_progress_frame(1, f"Importing assets: {relative}")
            task = ue.AssetImportTask()
            task.filename = str(file)
            task.destination_path = target
            task.destination_name = clean_stem
            task.automated = True
            task.async_ = False
            task.replace_existing = True
            task.save = True
            manager.import_asset_tasks([task])
            objects = list(task.get_objects())
            if not objects:
                raise RuntimeError(f"Unreal failed to import original {suffix} {relative}")
            paths = [str(obj.get_path_name()) for obj in objects]
            # Interchange may ignore task.destination_name. An actual returned
            # UObject path is the only reliable import identity.
            if len(paths) != len(set(p.casefold() for p in paths)):
                raise RuntimeError(f"One original source generated duplicate Unreal objects: {relative}")
            collisions = observed_objects.intersection(p.casefold() for p in paths)
            if collisions:
                raise RuntimeError(f"Distinct original sources produced the same Unreal object: {sorted(collisions)!r}")
            observed_objects.update(p.casefold() for p in paths)
            imported.extend(paths)
            if source_records is not None:
                source_records.append({"source_relative": relative.as_posix(),
                                       "unreal_object_paths": paths})
    return imported


def import_collision_in_editor(folder: Path, ue: Any, *,
                               source_records: list[dict] | None = None) -> list[str]:
    """Import real I3C mesh triangles; static physics may collide per polygon.

    Dynamic player rules, stage placement, slopes and capture/gameplay are NOT reconstructed.
    """
    files = sorted(folder.rglob("*.obj"))
    if not files:
        raise RuntimeError("No original I3C collision meshes available to import")
    if not hasattr(ue, "StaticMesh") or not hasattr(ue, "CollisionTraceFlag"):
        raise RuntimeError("Unreal StaticMesh collision editing is unavailable")
    # Fail before modifying editor packages on any source filename collision.
    plans = _planned_imports(folder, files, "/Game/ApeEscape3/Original/Collision")
    manager = ue.AssetToolsHelpers.get_asset_tools()
    result = []
    observed_objects: set[str] = set()
    progress = ue.ScopedSlowTask(len(plans), "Importing Ape Escape 3 original collision assets") if hasattr(ue, "ScopedSlowTask") and plans else None
    with (progress if progress is not None else nullcontext()):
        if progress is not None:
            progress.make_dialog(True)
        for source, relative, dest, name in plans:
            if progress is not None:
                if progress.should_cancel():
                    raise InterruptedError("Original collision import cancelled; partial assets may remain")
                progress.enter_progress_frame(1, f"Importing collision: {relative}")
            task = ue.AssetImportTask()
            task.filename = str(source)
            task.destination_path = dest
            task.destination_name = name
            task.automated = True
            task.async_ = False
            task.replace_existing = True
            task.save = True
            manager.import_asset_tasks([task])
            original_meshes = [obj for obj in task.get_objects() if isinstance(obj, ue.StaticMesh)]
            if not original_meshes:
                raise RuntimeError(f"No Unreal StaticMesh was created from original I3C mesh {relative}")
            imported_meshes = []
            for obj in original_meshes:
                object_path = str(obj.get_path_name())
                folded = object_path.casefold()
                if folded in observed_objects:
                    raise RuntimeError(
                        f"Two original I3C collision files map to the same UObject: {object_path}")
                observed_objects.add(folded)
                body = obj.get_editor_property("body_setup")
                if body is None:
                    raise RuntimeError(f"Original collision StaticMesh has no BodySetup: {relative}")
                body.set_editor_property("collision_trace_flag",
                                         ue.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
                if not ue.EditorAssetLibrary.save_loaded_asset(obj):
                    raise RuntimeError(f"Could not save original I3C collision asset: {relative}")
                item_path = str(obj.get_path_name())
                result.append(item_path)
                imported_meshes.append(item_path)
            if source_records is not None:
                source_records.append({"source_relative": relative.as_posix(),
                                       "unreal_object_paths": imported_meshes})
    return result


def import_glbs_in_editor(folder: Path, ue: Any,
                          destination: str = "/Game/ApeEscape3/Original/Models", *,
                          source_records: list[dict] | None = None) -> list[str]:
    return import_assets_in_editor(folder, ue, ".glb", destination,
                                   source_records=source_records)


def run(iso_file: str, *, ae3_cli: str = "ae3", ue: Any = None,
        repo: Path | None = None, private_root: Path | None = None) -> dict[str, Any]:
    """Run editor import. A successful result means imported assets, NOT playable gameplay.

    Import failure is explicit; a successful run never enables the Play button.
    All original game bytes stay under Unreal Saved/Getchu3Private.
    """
    if ue is None:
        import unreal as ue  # Only available in the actual Unreal Editor.
    saved = Path(private_root) if private_root else Path(ue.Paths.project_saved_dir()) / "Getchu3Private"
    saved = saved.expanduser().resolve()
    iso_path = Path(iso_file).expanduser().resolve()
    if saved == iso_path or saved == iso_path.parent or saved in iso_path.parents:
        raise ValueError("Private destination cannot overwrite or contain the source disc")
    saved.mkdir(parents=True, exist_ok=True)
    report_file = saved / "Getchu3ImportStatus.json"
    outcome: dict[str, Any] = {"schema": "ae3-unreal-owner-import/v1", "disc_sha256": None,
                               "status": "running", "phase": "verify_original_disc",
                               "gameplay_ready": False,
                               "animation_parity_verified": False,
                               "original_collision_imported": False,
                               "original_lua_bound_to_runtime": False,
                               "sound_engine_verified": False}
    report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
    try:
        source = verify_disc(iso_path)
        repo = Path(repo) if repo else locate_repo()
        # Unique owner-private staging on each verified attempt.
        folder = Path(tempfile.mkdtemp(prefix=source.digest[:16] + "-", dir=saved))
        outcome.update({"disc_sha256": source.digest, "workspace": str(folder),
                        "phase": "extract_source"})
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        extracted = folder / "original"
        recovery = recover_original(source.path, extracted, repo)
        outcome["original_files"] = recovery["original_vfi_files_extracted"]
        outcome["original_lua_scripts"] = recovery["original_lua_scripts"]
        outcome["phase"] = "unpack_original_stage_pck"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        pck = unpack_original_pck_for_unreal(extracted / "vfi", repo)
        outcome["original_stage_pck_archives"] = pck["original_stage_pck_archives"]
        outcome["original_pck_members_written"] = pck["new_original_members_written"]
        outcome["original_pck_source_types"] = pck["counts"]
        outcome["phase"] = "decode_tim2"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        converted = folder / "converted"
        from getchu3_media import convert_textures, convert_stream_audio
        textures = convert_textures(extracted / "vfi", converted / "textures", ae3_cli,
                                    material_alias_root=converted / "material_aliases")
        outcome["textures_converted"] = textures["converted_png_files"]
        outcome["original_pack_local_material_aliases"] = len(
            textures["pack_local_material_aliases"])
        outcome["phase"] = "decode_models"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        models = convert_original_models(extracted / "vfi", converted / "models",
                                         repo, ae3_cli, textures=converted / "textures",
                                         pack_texture_aliases=converted / "material_aliases")
        outcome["models_detected"] = models["original_i3d_bin_files"]
        outcome["phase"] = "original_stage_catalog"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        from getchu3_stage_catalog import build_original_stage_catalog
        stages = build_original_stage_catalog(extracted / "vfi", converted / "models",
                                              folder / "getchu3_original_stages.json")
        outcome["original_stage_count"] = stages["stage_count"]
        outcome["original_stage_script_count"] = stages["stage_scripts"]
        outcome["original_stage_catalog"] = str(folder / "getchu3_original_stages.json")
        outcome["phase"] = "verify_original_stage_bytecode_hashes"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        from getchu3_stage_integrity import validate_stage_catalog
        verified_stages = validate_stage_catalog(extracted / "vfi", stages)
        stage_index_path = folder / "getchu3_verified_stage_index.json"
        stage_index_path.write_text(
            json.dumps(verified_stages, indent=2) + "\n", encoding="utf-8")
        outcome["source_sha256_verified_area_scripts"] = verified_stages["verified_area_scripts"]
        outcome["verified_original_stage_index"] = str(stage_index_path)
        outcome["phase"] = "audit_original_lua_manager_bindings"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        from getchu3_original_bindings import create_original_binding_report
        try:
            native_evidence = create_original_binding_report(
                extracted, repo, folder / "getchu3_original_binding_evidence.json")
            outcome["original_binding_evidence_status"] = "source_checked"
            outcome["unresolved_original_binding_candidates"] = len(
                native_evidence["unresolved_original_bindings"])
            outcome["original_actor_class_literal_count"] = len(
                native_evidence["source_original_actor_class_literals"])
        except (ImportError, OSError, ValueError, RuntimeError) as exc:
            # A failure in the static recovery audit is not a failed asset
            # decode. Report it explicitly, without hiding missing gameplay.
            outcome["original_binding_evidence_status"] = "blocked"
            outcome["original_binding_evidence_error"] = (
                f"{type(exc).__name__}: {exc}")
        outcome["phase"] = "decode_streams"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        streams = convert_stream_audio(extracted / "vfi", converted / "audio", ae3_cli)
        outcome["streams_decoded"] = streams["source_exst"]
        outcome["phase"] = "decode_original_i3c"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        from getchu3_collision import convert_collision
        collision = convert_collision(extracted / "vfi", converted / "collision", ae3_cli)
        outcome["original_i3c_files"] = collision["original_i3c_files"]
        outcome["phase"] = "unreal_editor_asset_import"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        base = "/Game/ApeEscape3/Original"
        imported_sources: dict[str, list[dict]] = {
            "models": [], "textures": [], "streams": [], "collision": []}
        assets = import_glbs_in_editor(converted / "models", ue,
                                       destination=base + "/Models",
                                       source_records=imported_sources["models"])
        pngs = import_assets_in_editor(converted / "textures", ue, ".png",
                                       base + "/Textures",
                                       source_records=imported_sources["textures"])
        wavs = import_assets_in_editor(converted / "audio", ue, ".wav",
                                       base + "/Audio",
                                       source_records=imported_sources["streams"]) if streams["wav_files"] else []
        outcome["unreal_model_assets"] = assets
        outcome["unreal_texture_assets"] = pngs
        outcome["unreal_stream_audio_assets"] = wavs
        outcome["unreal_collision_mesh_assets"] = import_collision_in_editor(
            converted / "collision", ue, source_records=imported_sources["collision"])
        outcome["source_collision_mesh_assets_imported"] = True
        outcome["phase"] = "link_original_stage_source_objects"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        from getchu3_asset_registry import build_asset_registry
        registry_path = folder / "getchu3_source_asset_registry.json"
        source_registry = build_asset_registry(stages, imported_sources, registry_path)
        outcome["original_source_registry"] = str(registry_path)
        outcome["source_linked_stage_count"] = len(source_registry["original_stages"])
        outcome["phase"] = "audit_per_stage_native_gameplay_dependencies"
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
        if outcome.get("original_binding_evidence_status") == "source_checked":
            try:
                from getchu3_native_readiness import build_original_native_readiness
                evidence_path = folder / "getchu3_original_binding_evidence.json"
                evidence = json.loads(evidence_path.read_text(encoding="utf-8"))
                native_report_path = folder / "getchu3_original_native_stage_readiness.json"
                native_report = build_original_native_readiness(
                    stages, evidence, source_registry, native_report_path)
                outcome["native_stage_readiness_status"] = "source_evidence_only"
                outcome["native_stage_readiness_report"] = str(native_report_path)
                outcome["stages_with_unresolved_native_calls"] = (
                    native_report["stages_with_unresolved_calls"])
            except (ImportError, OSError, ValueError, TypeError, KeyError, RuntimeError) as exc:
                outcome["native_stage_readiness_status"] = "blocked"
                outcome["native_stage_readiness_error"] = (
                    f"{type(exc).__name__}: {exc}")
        else:
            outcome["native_stage_readiness_status"] = (
                "blocked_original_lua_elf_evidence_unavailable")
        outcome["original_collision_imported"] = False
        outcome["original_lua_bound_to_runtime"] = False
        outcome["phase"] = "asset_import_finished"
        outcome["status"] = "original_models_textures_streams_i3c_imported_gameplay_not_implemented"
        outcome["remaining"] = [
            "original actor and Lua callback runtime", "player and monkey simulation",
            "camera and animation runtime", "gadgets and capture gameplay",
            "audio event execution", "menus, progression and saves"]
        # Save is explicit, but it never certifies a playable game.
        return outcome
    except Exception as exc:
        outcome["status"] = "blocked"
        outcome["error"] = f"{type(exc).__name__}: {exc}"
        raise
    finally:
        report_file.write_text(json.dumps(outcome, indent=2) + "\n", encoding="utf-8")
