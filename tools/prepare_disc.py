#!/usr/bin/env python3
"""Prepare an owner-supplied PS2 ISO for a native, source-only Windows build.

Uses the EXACT pinned upstream ISO reader and code-window implementation.
Only the boot executable and needed SNR2 modules are privately extracted.
No commercial game bytes are committed or uploaded by this tool.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path, PurePosixPath
import re
import sys

BOOT2 = re.compile(r"(?im)^\s*BOOT2\s*=\s*cdrom0:\s*[\\/]*(.+?)\s*$")


def load_module(path: Path, name: str):
    if not path.is_file():
        raise FileNotFoundError(f"Missing pinned upstream script: {path}")
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise ValueError(f"Cannot load required upstream script: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_boot2(system_cnf: bytes) -> str:
    # BOOT2 should name an ISO9660 path such as cdrom0:\\SCUS_975.01;1.
    text = system_cnf.decode("ascii", errors="replace")
    match = BOOT2.search(text)
    if not match:
        raise ValueError("ISO SYSTEM.CNF has no BOOT2=cdrom0: executable")
    path = match.group(1).strip().replace("\\", "/").split(";", 1)[0].strip()
    parsed = PurePosixPath(path)
    if (not path or path.startswith("/") or any(
            part in ("..", ".", "") for part in path.split("/"))
            or ":" in path or "\\x00" in path or not parsed.name):
        raise ValueError(f"Unsafe BOOT2 path in SYSTEM.CNF: {path!r}")
    return path


def prepare(disc: Path, upstream: Path, out: Path, expected_sha: str,
            supplied_elf: Path | None = None) -> dict:
    if not disc.is_file():
        raise FileNotFoundError(f"Disc ISO not found: {disc}")
    iso = load_module(upstream / "scripts" / "disc.py", "ps2_recomp_disc_reader")
    windows = load_module(upstream / "scripts" / "windows.py", "ps2_recomp_code_windows")
    image = iso.Image(str(disc.resolve()))
    try:
        config = image.find("SYSTEM.CNF")
        if config is None:
            raise ValueError("Disc ISO has no SYSTEM.CNF")
        image.file.seek(config[2] * iso.SECTOR)
        boot_path = parse_boot2(image.file.read(config[3]))
        boot_entry = image.find(boot_path)
        if boot_entry is None:
            raise ValueError(f"ISO BOOT2 executable missing: {boot_path}")
        # Do not trust path components from an unvalidated image.
        executable = out / "disc" / boot_path
        executable.parent.mkdir(parents=True, exist_ok=True)
        image.copy(boot_entry, str(executable))
        actual_hash = sha256(executable)
        if actual_hash.lower() != expected_sha.lower():
            executable.unlink(missing_ok=True)
            raise ValueError(f"Unexpected ISO boot ELF SHA256 {actual_hash}; expected {expected_sha}")
        if supplied_elf is not None:
            if not supplied_elf.is_file() or sha256(supplied_elf) != actual_hash:
                raise ValueError("Provided -Elf does not match exact ISO BOOT2 executable")
        elf_bytes = executable.read_bytes()
        window_list = windows.windows(elf_bytes)
        overlays_dir = None
        if window_list:
            overlay_folder = out / "overlays"
            overlay_folder.mkdir(parents=True, exist_ok=True)
            overlays_dir = str(overlay_folder.resolve())
        modules_dir = None
        if b"SNR2" in elf_bytes:
            module_folder = out / "modules"
            iso.modules(image, str(module_folder))
            if any(module_folder.glob("*.snr")):
                modules_dir = str(module_folder.resolve())
        result = {
            "format": "ae3-private-disc-prep/v1",
            "disc": str(disc.resolve()), "boot_program": boot_path,
            "executable": str(executable.resolve()), "sha256": actual_hash,
            "modules_dir": modules_dir, "overlays_dir": overlays_dir,
            "code_windows": [{"base": f"{base:08x}", "size": size}
                             for base, size in window_list],
            "retail_assets_uploaded": False
        }
        out.mkdir(parents=True, exist_ok=True)
        (out / "disc_prepare.json").write_text(
            json.dumps(result, indent=2) + "\n", encoding="utf-8")
        return result
    finally:
        image.file.close()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--disc", type=Path, required=True)
    parser.add_argument("--upstream", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--expected-sha", required=True)
    parser.add_argument("--elf", type=Path)
    args = parser.parse_args(argv)
    report = prepare(args.disc, args.upstream, args.out,
                     args.expected_sha, args.elf)
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, IndexError, EOFError) as error:
        print(f"DISC_PREP_ERROR: {error}", file=sys.stderr)
        sys.exit(1)
