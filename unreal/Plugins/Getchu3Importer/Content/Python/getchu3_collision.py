"""Convert authentic I3D_I3C collision triangles into owner-private OBJ assets.

Source geometry comes from the original USA game's I3C BVH, via pxdl/ae3-sdk.
Imported meshes are static geometry; original collision RESPONSE rules and player
movement remain separate reverse-engineering work.
"""
from __future__ import annotations
import json
import shutil
import subprocess
from pathlib import Path
from typing import Callable

I3C_MAGIC = b"I3D_I3C\x00"
MAX_COLLISION_FILES = 2048
MAX_I3C_BYTES = 512 * 1024 * 1024


def convert_collision(source: Path, destination: Path, ae3: str,
                      *, runner: Callable = subprocess.run) -> dict:
    source = Path(source).resolve(strict=True)
    destination = Path(destination).resolve()
    if not source.is_dir() or source == destination or source in destination.parents or destination in source.parents:
        raise ValueError("Original I3C and exported geometry must reside in separate directories")
    inputs = []
    names = set()
    for f in sorted(source.rglob("*")):
        if f.is_symlink() or not f.is_file() or f.suffix.lower() != ".i3d":
            continue
        if f.stat().st_size < len(I3C_MAGIC) or f.stat().st_size > MAX_I3C_BYTES:
            raise ValueError(f"Malformed I3D candidate size: {f}")
        with f.open("rb") as stream:
            is_collision = stream.read(len(I3C_MAGIC)) == I3C_MAGIC
        if not is_collision:
            continue  # Genuine model I3D_BIN files are handled by model exporter.
        relative = f.relative_to(source)
        key = relative.with_suffix(".obj").as_posix().casefold()
        if key in names:
            raise ValueError(f"Collision file naming conflict: {relative}")
        names.add(key)
        inputs.append(f)
        if len(inputs) > MAX_COLLISION_FILES:
            raise ValueError("Original I3C file count exceeds bounded importer capacity")
    if not inputs:
        raise RuntimeError("Original archive contains no I3D_I3C collision files")
    if not ae3 or not (shutil.which(ae3) or Path(ae3).is_file()):
        raise FileNotFoundError("ae3-sdk command missing; cannot decode authentic I3C collision data")
    destination.mkdir(parents=True, exist_ok=True)
    objects = []
    for f in inputs:
        relative = f.relative_to(source)
        folder = destination / relative.parent
        folder.mkdir(parents=True, exist_ok=True)
        cmd = [ae3, "i3c", str(f), "--out", str(folder), "--format", "obj"]
        proc = runner(cmd, capture_output=True, text=True, timeout=120, check=False)
        obj = folder / (f.stem + ".obj")
        if proc.returncode or not obj.is_file() or obj.stat().st_size == 0:
            raise RuntimeError(f"Source I3C collision conversion failed for {relative}; "
                               f"exit={proc.returncode}; error={proc.stderr[-400:]}")
        # Reject empty geometry instead of treating a comment-only OBJ as collision.
        with obj.open("r", encoding="utf-8") as handle:
            nv = nf = 0
            for line in handle:
                if line.startswith("v "): nv += 1
                if line.startswith("f "): nf += 1
            if nv < 3 or nf < 1:
                raise RuntimeError(f"I3C collision output has no triangles: {relative}")
        objects.append({"original": relative.as_posix(), "exported": obj.relative_to(destination).as_posix(),
                        "vertices": nv, "faces": nf})
    report = {"original_i3c_files": len(inputs), "collision_objects": objects,
              "source_geometry": True, "native_collision_behavior_verified": False,
              "gameplay_ready": False}
    (destination / "getchu3_collision.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report
