"""Build a lossless source-to-Unreal asset reference registry for AE3.

Original stage identifiers, Lua scripts, PLC positions and their source-derived
model asset paths are preserved. This module makes NO scene or gameplay claims.
"""
from __future__ import annotations

from pathlib import Path, PurePosixPath
import json

KINDS = ("models", "textures", "streams", "collision")
SCHEMA = "getchu3-source-asset-registry/v1"


def normalized_relative(path: str) -> str:
    if not path or "\x00" in path or "\\" in path:
        raise ValueError("Original resource must use a safe POSIX relative path")
    value = PurePosixPath(path)
    if value.is_absolute() or not value.parts or any(p in (".", "..", "") for p in value.parts):
        raise ValueError(f"Invalid original source-relative path: {path!r}")
    return value.as_posix()


def _map_one_kind(kind: str, entries: list[dict]) -> dict[str, dict]:
    output = {}
    for entry in entries:
        name = normalized_relative(entry["source_relative"])
        objects = entry.get("unreal_object_paths", [])
        if not objects or not all(isinstance(p, str) and p.startswith("/Game/") and " " not in p
                                  for p in objects):
            raise ValueError(f"{kind}/{name}: Unreal import returned no usable object paths")
        key = name.casefold()
        if key in output:
            raise ValueError(f"{kind}/{name}: duplicate or case-colliding source asset")
        output[key] = {
            "source_relative": name,
            "unreal_object_paths": sorted(set(objects)),
        }
    return output


def build_asset_registry(catalog: dict, imported: dict[str, list[dict]],
                         output: Path) -> dict:
    """Link imported UObjects to exact source records, not guessed names.

    An ambiguous PLC model match remains ambiguous even when all candidate
    files were imported; we must not arbitrarily pick one for a game actor.
    """
    if catalog.get("schema") != "getchu3-original-stage-catalog/v1":
        raise ValueError("Only source-validated original stage catalogs are supported")
    if catalog.get("gameplay_ready") is not False:
        raise ValueError("Source catalog cannot certify original gameplay")
    if set(imported) - set(KINDS):
        raise ValueError("Unsupported original importer source category")
    maps = {kind: _map_one_kind(kind, imported.get(kind, [])) for kind in KINDS}
    records = []
    missing = []
    ambiguous = []
    for stage in catalog["stages"]:
        resolved = []
        for placement in stage["translation_only_associations"]:
            candidates = placement["matching_models"]
            if placement["resolution"] == "unique" and len(candidates) == 1:
                path = normalized_relative(candidates[0])
                model = maps["models"].get(path.casefold())
                if model is None:
                    missing.append({"stage": stage["stage"], "source_model": path})
                    objects = []
                    resolution = "unimported"
                else:
                    objects = model["unreal_object_paths"]
                    resolution = "linked_original_model_source"
            elif placement["resolution"] == "ambiguous":
                ambiguous.append({"stage": stage["stage"],
                                  "source_record": placement["source_record"],
                                  "matching_models": candidates})
                objects = []
                resolution = "ambiguous_unresolved"
            else:
                objects = []
                resolution = "missing_original_model_source"
            resolved.append({
                "source_plc": placement["plc"],
                "source_record": placement["source_record"],
                "original_entity_name": placement["original_entity_name"],
                "original_xyz": placement["original_xyz"],
                "resolution": resolution,
                "source_original_gameplay": False,
                "unreal_imported_objects": objects,
                "original_rotation_scale_verified": False,
            })
        records.append({
            "stage": stage["stage"],
            "area_lua_sources": stage["area_lua_sources"],
            "resolved_source_placements": resolved,
            "can_construct_original_stage": False,
        })
    if missing:
        raise RuntimeError("A unique original PLC model was not imported; "
                           f"first source: {missing[0]!r}")
    report = {
        "schema": SCHEMA,
        "status": "unreal_source_assets_indexed_original_gameplay_unavailable",
        "gameplay_ready": False,
        "imported_assets": {
            kind: [maps[kind][key] for key in sorted(maps[kind])]
            for kind in KINDS
        },
        "original_stages": records,
        "ambiguous_placement_matches": ambiguous,
        "original_lua_vm_attached": False,
        "original_scene_placement_implemented": False,
    }
    output = Path(output).resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")
    return report
