#!/usr/bin/env python3
"""Offline native Ape Escape 3 asset-preparation bridge to Saru3VfiTool.

The converter is an OPTIONAL external GPLv3 developer tool, not a linked
component of Getchu3Native.exe. Original files remain under PRIVATE owner
workspace. This never boots or simulates any PlayStation 2 hardware.
"""
from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import subprocess
import sys

ASSET_TYPES = {".i3d", ".i3c", ".png", ".tm2", ".luc", ".lua", ".pck",
               ".sz", ".json", ".sbt", ".exdb", ".bin"}
MANIFEST_VERSION = "getchu3-native-asset-saru3-bridge/v1"


def saru_command(tool: Path) -> list[str]:
    if not tool.is_file():
        raise FileNotFoundError(f"Saru3VfiTool executable/JAR missing: {tool}")
    if tool.suffix.lower() == ".dll":
        return ["dotnet", str(tool)]
    if tool.suffix.lower() == ".exe" or tool.suffix == "":
        return [str(tool)]
    raise ValueError("Use a Saru3VfiTool .exe or .dll (not its source files)")


def prepare(game: Path, destination: Path, converter: Path, probe: Path,
            *, tim2_png=True, reuse=False, runner=subprocess.run) -> dict:
    game = game.resolve(strict=True)
    destination = destination.resolve()
    converter = converter.resolve(strict=True)
    probe = probe.resolve(strict=True)
    if not game.is_file() or not probe.is_file():
        raise FileNotFoundError("Owned game input or native asset probe is missing")
    # Keep all written paths away from game image and executable.
    if destination == game or game in destination.parents or destination == game.parent:
        raise ValueError("Choose an independent PRIVATE output directory")
    if converter == game or probe == game:
        raise ValueError("Original game data cannot be used as an executable")
    if destination.exists() and not reuse and any(destination.iterdir()):
        raise FileExistsError("Output directory already contains data; use --reuse explicitly")
    destination.mkdir(parents=True, exist_ok=True)
    databin = destination / "DATA.BIN"
    archive_dir = destination / "extracted"
    if game.suffix.casefold() == ".iso":
        if not databin.is_file():
            runner([str(probe), str(game), "--copy-databin", str(databin)], check=True)
            if not databin.is_file():
                raise RuntimeError("Native archive probe reported success without DATA.BIN")
    elif game.name.casefold() == "data.bin":
        databin = game
    else:
        raise ValueError("Supply a locally owned ISO or DATA.BIN")
    saru = saru_command(converter)
    if not (reuse and (archive_dir / "databin.manifest.json").is_file()):
        archive_dir.mkdir(exist_ok=True)
        command = saru + ["extract"]
        if tim2_png:
            command.append("--tim2")
        command += [str(databin), str(archive_dir)]
        runner(command, check=True)
        if not (archive_dir / "databin.manifest.json").is_file():
            raise RuntimeError("Saru3VfiTool did not produce a DATA.BIN manifest")

    counts: Counter[str] = Counter()
    sampled: dict[str, list[str]] = {}
    script_bytes = 0
    for file in archive_dir.rglob("*"):
        if not file.is_file() or file.is_symlink():
            continue
        suffix = file.suffix.casefold()
        counts[suffix or "(none)"] += 1
        if suffix in {".luc", ".i3d", ".i3c", ".tm2", ".png"}:
            relative = file.relative_to(archive_dir).as_posix()
            examples = sampled.setdefault(suffix, [])
            if len(examples) < 12:
                examples.append(relative)
        if suffix == ".luc":
            script_bytes += file.stat().st_size
    manifest = {
        "schema": MANIFEST_VERSION,
        "stage": "asset-preparation-only-not-a-game-boot",
        "native_runtime_depends_on_saru": False,
        "requires_ps2_hardware_emulation": False,
        "asset_types": dict(sorted(counts.items())),
        "sample_assets": sampled,
        "lua_bytecode_bytes": script_bytes,
        "raw_databin": str(databin),
        "extracted_root": str(archive_dir),
        "original_game_data_uploaded": False
    }
    (destination / "native_asset_inventory.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--game", type=Path, required=True, help="Owner's ISO or DATA.BIN")
    p.add_argument("--out", type=Path, required=True,
                   help="Private output directory outside game ISO")
    p.add_argument("--saru", type=Path, required=True,
                   help="Saru3VfiTool.exe or Saru3VfiTool.dll")
    p.add_argument("--probe", type=Path, default=Path("native/build/Release/Getchu3AssetProbe.exe"),
                   help="Native archive probe built from Getchu3Assets")
    p.add_argument("--keep-tim2", action="store_true",
                   help="Do not generate PNG and TIM2 JSON sidecars")
    p.add_argument("--reuse", action="store_true",
                   help="Explicitly reuse existing extraction/manifest")
    args = p.parse_args(argv)
    result = prepare(args.game, args.out, args.saru, args.probe,
                     tim2_png=not args.keep_tim2, reuse=args.reuse)
    print(json.dumps(result, indent=2))
    print("\nThis is a native asset preparation step, NOT playable gameplay.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as exc:
        print("NATIVE_ASSET_PREPARATION_FAILED:", exc, file=sys.stderr)
        raise SystemExit(1)
