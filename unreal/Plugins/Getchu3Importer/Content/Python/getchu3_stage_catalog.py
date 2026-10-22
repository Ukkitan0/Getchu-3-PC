"""Source-faithful Ape Escape 3 stage registry and PLC placement inventory.

Keeps original stage scripts and PLC transform data as evidence for the future
game runtime. It deliberately does not spawn actors, guess rotation/scale, or
label converted GLBs as a playable scene. No game bytes are published.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
import math
from pathlib import Path
import re
import struct

PLC_HEADER = 0xC0
PLC_RECORD = 0x80
PLC_LIMIT = 64 * 1024 * 1024
PLC_MAX_RECORDS = 100_000
STAGE_ID = re.compile(r"[a-zA-Z0-9_]+")


@dataclass(frozen=True)
class OriginalPlacement:
    source_record: int
    name: str
    xyz: tuple[float, float, float]


def read_original_plc(data: bytes) -> dict:
    """Read only original fields recovered in native/src/level_scene.cpp.

    This does NOT recover original rotation/scale, entity classes or spawn tags.
    Malformed PLC records remain counted rather than being silently treated
    as valid instances. Truncated PLC tables are rejected entirely.
    """
    if not PLC_HEADER <= len(data) <= PLC_LIMIT:
        raise ValueError("Original PLC size outside allowed bounds")
    count = struct.unpack_from("<I", data, 0)[0]
    if count > PLC_MAX_RECORDS or count > (len(data) - PLC_HEADER) // PLC_RECORD:
        raise ValueError("Original PLC record table is truncated or excessive")
    placements: list[OriginalPlacement] = []
    rejected: list[int] = []
    for index in range(count):
        offset = PLC_HEADER + index * PLC_RECORD
        field = data[offset:offset + 32].split(b"\0", 1)[0]
        name = ""
        if field and all(32 <= b <= 126 for b in field):
            name = field.decode("ascii")
        xyz = struct.unpack_from("<fff", data, offset + 0x40)
        if not name or any(not math.isfinite(v) or abs(v) >= 100_000 for v in xyz):
            rejected.append(index)
            continue
        placements.append(OriginalPlacement(index, name, xyz))
    return {
        "declared_records": count,
        "placements": [
            {"source_record": p.source_record, "name": p.name, "original_xyz": list(p.xyz)}
            for p in placements
        ],
        "rejected_record_indices": rejected,
        "all_record_transforms_recovered": False,
        "original_units_and_axes": True,
        "rotation_and_scale_not_recovered": True
    }


def stage_from_path(path: Path) -> str | None:
    """Resolve retail stage/<name> using original path structure, no guesses."""
    parts = path.parts
    for i in range(len(parts) - 1):
        if parts[i].casefold() == "stage" and STAGE_ID.fullmatch(parts[i + 1]):
            return parts[i + 1].casefold()
    return None


def matches_original_placement(model_stem: str, placement: str) -> bool:
    # Matches the native recoverer's exact name-or-suffix rule.
    a, b = model_stem.casefold(), placement.casefold()
    return a == b or (len(a) > len(b) + 1 and a.endswith("_" + b))


def build_original_stage_catalog(vfi: Path, converted_models: Path, output: Path) -> dict:
    """Inventory original scripts, placements and unambiguous model associations.

    Scripts are copied nowhere; only source-relative paths and hashes are
    placed in a private JSON report. Ambiguity and missing models are exposed.
    """
    vfi = Path(vfi).resolve(strict=True)
    converted_models = Path(converted_models).resolve(strict=True)
    output = Path(output).resolve()
    if not vfi.is_dir() or not converted_models.is_dir():
        raise NotADirectoryError("Original extracted resource and model folders required")
    if output == vfi or output == converted_models or output in vfi.parents or output in converted_models.parents:
        raise ValueError("Stage registry must be written outside original source trees")
    all_scripts: dict[str, list[dict]] = {}
    all_plc: dict[str, list[dict]] = {}
    all_models: dict[str, list[dict]] = {}
    for path in sorted(vfi.rglob("*")):
        if path.is_symlink() or not path.is_file():
            continue
        relative = path.relative_to(vfi)
        stage = stage_from_path(relative)
        if stage is None:
            continue
        extension = path.suffix.casefold()
        if extension == ".luc" and path.name.casefold() == "area.luc":
            if path.stat().st_size > 8 * 1024 * 1024:
                raise ValueError(f"Original script exceeds 8 MiB limit: {relative}")
            payload = path.read_bytes()
            if not payload.startswith(b"\x1bLua\x50"):
                raise ValueError(f"Stage area.luc is not original Lua 5.0: {relative}")
            all_scripts.setdefault(stage, []).append({
                "original_path": relative.as_posix(),
                "sha256": hashlib.sha256(payload).hexdigest(),
                "source_bytes": len(payload),
                "game_bindings_connected": False
            })
        elif extension == ".plc":
            if path.stat().st_size > PLC_LIMIT:
                raise ValueError(f"Original PLC exceeds 64 MiB limit: {relative}")
            content = read_original_plc(path.read_bytes())
            all_plc.setdefault(stage, []).append({
                "original_path": relative.as_posix(), **content
            })

    for path in sorted(converted_models.rglob("*.glb")):
        if path.is_symlink() or not path.is_file():
            continue
        relative = path.relative_to(converted_models)
        stage = stage_from_path(relative)
        if stage is None:
            continue
        all_models.setdefault(stage, []).append({
            "source_derived_glb": relative.as_posix(),
            "model_stem": path.stem
        })

    stage_names = sorted(set(all_scripts) | set(all_plc) | set(all_models))
    stages = []
    for name in stage_names:
        scripts = all_scripts.get(name, [])
        plcs = all_plc.get(name, [])
        models = all_models.get(name, [])
        associations = []
        for plc in plcs:
            for placement in plc["placements"]:
                possible = [m["source_derived_glb"] for m in models
                            if matches_original_placement(m["model_stem"], placement["name"])]
                associations.append({
                    "plc": plc["original_path"],
                    "source_record": placement["source_record"],
                    "original_entity_name": placement["name"],
                    "original_xyz": placement["original_xyz"],
                    "matching_models": possible,
                    "resolution": "unique" if len(possible) == 1 else
                                  ("missing" if not possible else "ambiguous"),
                    "original_rotation_scale_verified": False
                })
        stages.append({
            "stage": name, "area_lua_sources": scripts,
            "plc_sources": plcs, "converted_model_sources": models,
            "translation_only_associations": associations,
            "unmatched_placements": sum(x["resolution"] == "missing" for x in associations),
            "ambiguous_placements": sum(x["resolution"] == "ambiguous" for x in associations),
            "original_gameplay_ready": False,
            "authentic_level_transform_complete": False
        })

    if not all_scripts:
        raise RuntimeError("No original stage area.luc scripts found in extracted source")
    manifest = {
        "schema": "getchu3-original-stage-catalog/v1",
        "stage_count": len(stages),
        "stage_scripts": sum(len(s["area_lua_sources"]) for s in stages),
        "status": "original_source_stage_inventory_only",
        "gameplay_ready": False,
        "rotation_scale_camera_ai_unverified": True,
        "stages": stages
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(manifest, indent=2, ensure_ascii=True) + "\n",
                      encoding="utf-8")
    return manifest
