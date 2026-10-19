#!/usr/bin/env python3
"""Content-addressed native R5900 translation cache.

The game executable, EE entry points, upstream eerecomp binary and any optional
SNR2 modules / unpacked code-window images are hashed before deciding whether
the expensive original-ELF -> C++ translation must be rerun.

Cache files and C++ outputs remain strictly inside the user's private folder.
This never treats a successful compiler exit as evidence of playable gameplay.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

CACHE_VERSION = 1


def hash_file(path: Path) -> str:
    if not path.is_file():
        raise FileNotFoundError(f"Missing native translation input: {path}")
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def directory_inputs(directory: Path | None, suffix: str) -> list[Path]:
    if directory is None:
        return []
    if not directory.is_dir():
        raise FileNotFoundError(f"Native translation input directory absent: {directory}")
    return sorted((p for p in directory.iterdir()
                   if p.is_file() and p.suffix.lower() == suffix),
                  key=lambda p: p.name.lower())


def fingerprint(compiler: Path, elf: Path, entries: Path,
                modules: Path | None = None, overlays: Path | None = None) -> str:
    digest = hashlib.sha256()
    digest.update(f"ae3-aot-cache/{CACHE_VERSION}\n".encode("ascii"))
    for kind, path in (("compiler", compiler), ("elf", elf),
                       ("entries", entries)):
        digest.update(f"{kind}\0{path.resolve()}\0{hash_file(path)}\n".encode("utf-8"))
    for kind, directory, suffix in (("module", modules, ".snr"),
                                     ("overlay", overlays, ".bin")):
        if directory is None:
            digest.update(f"{kind}=absent\n".encode("ascii"))
            continue
        digest.update(f"{kind}-folder\0{directory.resolve()}\n".encode("utf-8"))
        for file in directory_inputs(directory, suffix):
            digest.update(f"{kind}\0{file.name}\0{hash_file(file)}\n".encode("utf-8"))
    return digest.hexdigest()


def compile_cached(compiler: Path, elf: Path, entries: Path, out: Path,
                   modules: Path | None = None, overlays: Path | None = None,
                   *, force=False) -> dict:
    fingerprint_hex = fingerprint(compiler, elf, entries, modules, overlays)
    out.mkdir(parents=True, exist_ok=True)
    marker = out / ".ae3-aot-cache.json"
    previous = None
    if marker.is_file():
        try:
            previous = json.loads(marker.read_text(encoding="utf-8"))
        except (ValueError, UnicodeError):
            previous = None
    cpp_units = sorted(out.glob("*.cpp"))
    if (not force and previous and previous.get("fingerprint") == fingerprint_hex
            and cpp_units and previous.get("units") == len(cpp_units)):
        return {
            "mode": "reused",
            "fingerprint": fingerprint_hex,
            "units": len(cpp_units),
            "gameplay_verified": False,
        }
    command = [str(compiler.resolve()), str(elf.resolve()),
               "--entries", str(entries.resolve())]
    if modules is not None:
        command += ["--modules", str(modules.resolve())]
    if overlays is not None:
        command += ["--overlays", str(overlays.resolve())]
    command += ["--out", str(out.resolve())]
    subprocess.run(command, check=True)
    cpp_units = sorted(out.glob("*.cpp"))
    if not cpp_units:
        raise ValueError("eerecomp succeeded but generated no C++ translation units")
    report = {
        "mode": "translated",
        "fingerprint": fingerprint_hex,
        "units": len(cpp_units),
        "gameplay_verified": False,
    }
    # Save the state only AFTER compiler success and generated C++ validation.
    with tempfile.NamedTemporaryFile("w", dir=out, prefix=".aot-", suffix=".json",
                                     encoding="utf-8", delete=False) as stream:
        json.dump({"version": CACHE_VERSION, "fingerprint": fingerprint_hex,
                   "units": len(cpp_units)}, stream, indent=2)
        stream.write("\n")
        temp = Path(stream.name)
    try:
        os.replace(temp, marker)
    finally:
        temp.unlink(missing_ok=True)
    return report


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--compiler", type=Path, required=True)
    p.add_argument("--elf", type=Path, required=True)
    p.add_argument("--entries", type=Path, required=True)
    p.add_argument("--out", type=Path, required=True)
    p.add_argument("--modules", type=Path)
    p.add_argument("--overlays", type=Path)
    p.add_argument("--force", action="store_true")
    p.add_argument("--report", type=Path)
    a = p.parse_args(argv)
    result = compile_cached(a.compiler, a.elf, a.entries, a.out,
                            a.modules, a.overlays, force=a.force)
    if a.report is not None:
        a.report.parent.mkdir(parents=True, exist_ok=True)
        a.report.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("AE3_NATIVE_AOT_CACHE " + json.dumps(result))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print("AOT_CACHE_ERROR:", error, file=sys.stderr)
        raise SystemExit(1)
