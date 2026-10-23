"""Convert genuine owner-extracted AE3 TIM2 and EXST resources for Unreal.

The original game mixer/event scheduler is NOT recreated by these WAV files.
Conversion requires installed ae3tools (pxdl/ae3-sdk) and owner-private files.
"""
from __future__ import annotations

from pathlib import Path
import hashlib
import json
import shutil
import subprocess
from typing import Callable

MAX_COUNT = 10000
MAX_FILE = 512 * 1024 * 1024


def _candidates(root: Path, suffix: str, magic: bytes) -> list[Path]:
    root = root.resolve(strict=True)
    result = []
    names: set[str] = set()
    for file in sorted(root.rglob("*")):
        if file.is_symlink() or not file.is_file() or file.suffix.lower() != suffix:
            continue
        rel = file.relative_to(root)
        if file.stat().st_size < len(magic) or file.stat().st_size > MAX_FILE:
            raise ValueError(f"Invalid original {suffix} size: {rel}")
        with file.open("rb") as src:
            if src.read(len(magic)) != magic:
                raise ValueError(f"Invalid original {suffix} magic: {rel}")
        output_key = rel.with_suffix("").as_posix().casefold()
        if output_key in names:
            raise ValueError(f"Ambiguous case-insensitive original resource path: {rel}")
        names.add(output_key)
        result.append(file)
        if len(result) > MAX_COUNT:
            raise ValueError(f"Too many original {suffix} files")
    return result


def _cli(ae3: str, *, runner: Callable = subprocess.run) -> str:
    if not ae3 or not (shutil.which(ae3) or Path(ae3).is_file()):
        raise FileNotFoundError("ae3-sdk command missing; original media cannot be converted")
    return ae3


def convert_textures(source: Path, dest: Path, ae3: str, *,
                     material_alias_root: Path | None = None,
                     runner: Callable = subprocess.run) -> dict:
    source = Path(source).resolve(strict=True)
    dest = Path(dest).resolve()
    if dest == source or source in dest.parents or dest in source.parents:
        raise ValueError("Converted textures must live in a separate private directory")
    if material_alias_root is not None:
        material_alias_root = Path(material_alias_root).resolve()
        if (material_alias_root == source or source in material_alias_root.parents or
            material_alias_root == dest or dest in material_alias_root.parents or
            material_alias_root in dest.parents):
            raise ValueError("Pack material aliases require a separate owner-private destination")
    files = _candidates(source, ".tm2", b"TIM2")
    if not files:
        raise RuntimeError("No original TIM2 textures available")
    tool = _cli(ae3, runner=runner)
    converted = []
    aliases = []
    dest.mkdir(parents=True, exist_ok=True)
    for file in files:
        rel = file.relative_to(source)
        folder = dest / rel.parent
        folder.mkdir(parents=True, exist_ok=True)
        # One file at a time makes ae3's dest_paths preserve the input hierarchy.
        cmd = [tool, "tm2", str(file), "--out", str(folder), "--quiet"]
        result = runner(cmd, capture_output=True, text=True, timeout=120, check=False)
        outputs = sorted(folder.glob(file.stem + ".png"))
        outputs += sorted(folder.glob(file.stem + "_[0-9]*.png"))
        if result.returncode or not outputs or any(p.stat().st_size == 0 for p in outputs):
            raise RuntimeError(f"Original TIM2 conversion failed: {rel}; "
                               f"exit={result.returncode}; stderr={result.stderr[-400:]}")
        for png in outputs:
            converted.append(png.relative_to(dest).as_posix())
            # The genuine PCK exporter records index and original name as
            # 0003_material.tm2. ae3 gltf asks for material.png, not
            # 0003_material.png. Emit a pack-local alias only, so a matching
            # material from another pack or stage is never chosen by glob.
            if (material_alias_root is not None and
                "pck_assets" in rel.parts and len(file.stem) > 5 and
                file.stem[:4].isdigit() and file.stem[4] == "_"):
                suffix = png.name[len(file.stem):]
                name = file.stem[5:] + suffix
                destination = material_alias_root / rel.parent / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                if destination.exists():
                    if destination.read_bytes() != png.read_bytes():
                        raise RuntimeError(
                            f"Ambiguous original texture material within one PCK: {destination}")
                else:
                    shutil.copyfile(png, destination)
                aliases.append(destination.relative_to(material_alias_root).as_posix())
    result = {"source_files": len(files), "converted_png_files": len(converted),
              "files": converted, "pack_local_material_aliases": aliases,
              "source": "genuine TIM2", "gameplay_ready": False}
    (dest / "getchu3_textures.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    return result


def convert_stream_audio(source: Path, dest: Path, ae3: str, *,
                         runner: Callable = subprocess.run) -> dict:
    source = Path(source).resolve(strict=True)
    dest = Path(dest).resolve()
    if dest == source or source in dest.parents or dest in source.parents:
        raise ValueError("Converted streams must live in a separate private directory")
    # Some original .x files are not EXST: report but don't invent WAVs for them.
    streams = []
    not_exst = []
    for path in sorted(source.rglob("*")):
        if path.is_symlink() or not path.is_file() or path.suffix.lower() != ".x":
            continue
        with path.open("rb") as f:
            magic = f.read(4)
        if magic == b"EXST":
            streams.append(path)
        else:
            not_exst.append(path.relative_to(source).as_posix())
    if len(streams) > MAX_COUNT:
        raise ValueError("Unreasonably many retail EXST streams")
    if not streams:
        return {"source_exst": 0, "wav_files": [], "other_x": not_exst,
                "gameplay_ready": False}
    tool = _cli(ae3, runner=runner)
    converted = []
    dest.mkdir(parents=True, exist_ok=True)
    for file in streams:
        rel = file.relative_to(source)
        if file.stat().st_size > MAX_FILE:
            raise ValueError(f"Original EXST exceeds conversion budget: {rel}")
        output = (dest / rel).with_suffix(".wav")
        output.parent.mkdir(parents=True, exist_ok=True)
        proc = runner([tool, "exst", str(file), "--decode", "-o", str(output)],
                      capture_output=True, text=True, timeout=120, check=False)
        if proc.returncode or not output.is_file() or output.stat().st_size < 44:
            raise RuntimeError(f"Original EXST decode failed: {rel}; "
                               f"exit={proc.returncode}; stderr={proc.stderr[-400:]}")
        converted.append(output.relative_to(dest).as_posix())
    report = {"source_exst": len(streams), "wav_files": converted,
              "other_x": not_exst, "gameplay_ready": False,
              "warning": "Decoded streams are not the original dynamic audio engine"}
    (dest / "getchu3_audio.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report
