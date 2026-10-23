"""Verify owner-extracted AE3 stage bytecode against its own ISO import catalog.

No scripts are interpreted, generated, copied or distributed here. The importer
must have already SHA-verified the original USA ISO and executable.
"""
from __future__ import annotations

import hashlib
from pathlib import Path, PurePosixPath
import re

STAGE_ID = re.compile(r"[A-Za-z0-9_]{1,64}\\Z")
HEX64 = re.compile(r"[a-fA-F0-9]{64}\\Z")
LUA_MAGIC = b"\\x1bLua\\x50"
MAX_SCRIPT_BYTES = 16 * 1024 * 1024


def source_path(stage_id: str, relative: str) -> PurePosixPath:
    """Accept a stage's authentic area.luc path; reject path traversal."""
    if not STAGE_ID.fullmatch(stage_id):
        raise ValueError("Invalid original stage identifier")
    if not isinstance(relative, str) or "\\\\" in relative or "\\x00" in relative:
        raise ValueError("Unsafe original source script path")
    path = PurePosixPath(relative)
    if (path.is_absolute() or ".." in path.parts or "." in relative.split("/") or
            "" in relative.split("/") or len(path.parts) != 5 or
            tuple(part.casefold() for part in path.parts[:4]) !=
            ("debug", "us", "stage", stage_id.casefold()) or
            path.name.casefold() != "area.luc"):
        raise ValueError("Source Lua file must be original stage-local area.luc")
    return path


def validate_stage_catalog(extracted_vfi: Path, catalog: dict) -> dict:
    """Validate every stage ID and its single original script SHA-256.

    This does not make Lua bindings, actor managers or gameplay operational.
    """
    extracted_vfi = Path(extracted_vfi).resolve(strict=True)
    if not extracted_vfi.is_dir():
        raise NotADirectoryError("Original extracted VFI stage source missing")
    if catalog.get("schema") != "getchu3-original-stage-catalog/v1":
        raise ValueError("Not the original verified stage catalog schema")
    if catalog.get("gameplay_ready") is not False:
        raise ValueError("Catalog falsely claims completed game")
    stages = catalog.get("stages")
    if not isinstance(stages, list) or not stages or len(stages) > 1000:
        raise ValueError("Original stage table missing or outside budget")
    seen: set[str] = set()
    verified: list[dict] = []
    for stage in stages:
        if not isinstance(stage, dict):
            raise ValueError("Malformed original stage record")
        stage_id = stage.get("stage")
        if not isinstance(stage_id, str) or not STAGE_ID.fullmatch(stage_id):
            raise ValueError("Original stage identifier malformed")
        key = stage_id.casefold()
        if key in seen:
            raise ValueError("Duplicate original stage identifier")
        seen.add(key)
        scripts = stage.get("area_lua_sources", [])
        if not isinstance(scripts, list) or len(scripts) > 1:
            raise ValueError("Ambiguous original area.luc sources")
        if not scripts:
            verified.append({"stage": stage_id, "source": None, "verified": False})
            continue
        script = scripts[0]
        if not isinstance(script, dict):
            raise ValueError("Malformed original script record")
        relative = source_path(stage_id, script.get("original_path"))
        expected = script.get("sha256")
        if not isinstance(expected, str) or not HEX64.fullmatch(expected):
            raise ValueError("Original stage script SHA-256 absent or malformed")
        path = extracted_vfi.joinpath(*relative.parts)
        if path.is_symlink() or not path.is_file():
            raise FileNotFoundError("Owner-original source Lua stage file unavailable")
        length = path.stat().st_size
        if length < 14 or length > MAX_SCRIPT_BYTES:
            raise ValueError("Original stage Lua size invalid")
        with path.open("rb") as handle:
            magic = handle.read(5)
            handle.seek(0)
            digest = hashlib.file_digest(handle, "sha256").hexdigest()
        if magic != LUA_MAGIC or digest.casefold() != expected.casefold():
            raise ValueError("Owner-original Lua bytes mismatch verified stage catalog")
        verified.append({"stage": stage_id, "source": relative.as_posix(),
                         "sha256": digest, "verified": True})
    return {"schema": "getchu3-verified-original-stage-index/v1",
            "total_stage_ids": len(verified),
            "verified_area_scripts": sum(bool(r["verified"]) for r in verified),
            "gameplay_ready": False, "stages": verified}
