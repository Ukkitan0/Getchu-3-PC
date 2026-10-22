"""Owner-local original AE3 ISO -> Unreal Editor asset import.

This editor-only module intentionally refuses to label asset extraction as gameplay.
No commercial files are stored in the source repository or embedded in the plugin.
"""
from __future__ import annotations

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


def convert_original_models(source: Path, destination: Path, repo: Path,
                            ae3_cli: str) -> dict[str, Any]:
    if not ae3_cli or (not shutil.which(ae3_cli) and not Path(ae3_cli).is_file()):
        raise FileNotFoundError("The ae3-sdk 'ae3' model converter is not installed; refusing to invent models")
    tools = repo / "native" / "tools"
    sys.path.insert(0, str(tools))
    try:
        module = importlib.import_module("convert_models_to_glb")
        outcome = module.export_models(source, destination, ae3_cli, animations=True)
    finally:
        sys.path.remove(str(tools))
    failures = outcome.get("results", {}).get("failed", 0)
    if failures or outcome.get("original_i3d_bin_files", 0) == 0:
        raise RuntimeError(f"Original 3D model conversion incomplete: failed={failures}; "
                           f"detected={outcome.get('original_i3d_bin_files', 0)}")
    return outcome


def import_glbs_in_editor(folder: Path, ue: Any, destination: str = "/Game/ApeEscape3/Original") -> list[str]:
    glbs = sorted(folder.rglob("*.glb"))
    if not glbs:
        raise RuntimeError("No converted original GLB files were found")
    imported: list[str] = []
    destinations: set[str] = set()
    manager = ue.AssetToolsHelpers.get_asset_tools()
    for glb in glbs:
        relative = glb.relative_to(folder)
        # Unreal package segments are restricted; mapping is stable and case-preserving.
        parts = ["".join(c if c.isascii() and (c.isalnum() or c == "_") else "_"
                         for c in part) for part in relative.parent.parts if part not in (".", "")]
        target = destination + ("/" + "/".join(parts) if parts else "")
        key = (target + "/" + glb.stem).casefold()
        if key in destinations:
            raise RuntimeError(f"Different original source models map to the same Unreal asset: {relative}")
        destinations.add(key)
        task = ue.AssetImportTask()
        task.filename = str(glb)
        task.destination_path = target
        task.automated = True
        task.async_ = False
        task.replace_existing = True
        task.save = True
        result = manager.import_asset_tasks([task])
        paths = list(task.imported_object_paths)
        if not paths:
            raise RuntimeError(f"Unreal failed to import original GLB {relative}; result={result}")
        imported.extend(str(path) for path in paths)
    return imported


def run(iso_file: str, *, ae3_cli: str = "ae3", ue: Any = None,
        repo: Path | None = None, private_root: Path | None = None) -> dict[str, Any]:
    """Run editor import. A successful result means imported assets, NOT playable gameplay.

    Import failure is explicit; a successful run never enables the Play button.
    All original game bytes stay under Unreal Saved/Getchu3Private.
    """
    if ue is None:
        import unreal as ue  # Only available in the actual Unreal Editor.
    source = verify_disc(Path(iso_file))
    repo = Path(repo) if repo else locate_repo()
    saved = Path(private_root) if private_root else Path(ue.Paths.project_saved_dir()) / "Getchu3Private"
    saved = saved.expanduser().resolve()
    if saved == source.path.parent or saved in source.path.parents:
        raise ValueError("Private destination cannot be inside the source ISO path")
    saved.mkdir(parents=True, exist_ok=True)
    # Unique per attempt; a failed extraction never poisons a later retry.
    folder = Path(tempfile.mkdtemp(prefix=source.digest[:16] + "-", dir=saved))
    report_file = saved / "Getchu3ImportStatus.json"
    outcome: dict[str, Any] = {"schema": "ae3-unreal-owner-import/v1", "disc_sha256": source.digest,
                               "status": "running", "gameplay_ready": False, "workspace": str(folder)}
    report_file.write_text(json.dumps(outcome, indent=2), encoding="utf-8")
    try:
        extracted = folder / "original"
        recovery = recover_original(source.path, extracted, repo)
        outcome["original_files"] = recovery["original_vfi_files_extracted"]
        outcome["original_lua_scripts"] = recovery["original_lua_scripts"]
        converted = folder / "converted"
        models = convert_original_models(extracted / "vfi", converted, repo, ae3_cli)
        outcome["models_detected"] = models["original_i3d_bin_files"]
        assets = import_glbs_in_editor(converted, ue)
        outcome["unreal_asset_paths"] = assets
        outcome["status"] = "original_assets_imported_gameplay_not_implemented"
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
