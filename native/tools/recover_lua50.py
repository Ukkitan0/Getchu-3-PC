#!/usr/bin/env python3
"""Recover owned Ape Escape 3 level Lua 5.0 source for native-port analysis.

This is an *offline* build-time extraction step. The finished Windows game
must not depend on Java, a PS2 emulator, or a PS2 runtime. Uses a user-supplied
unluac JAR (Lua 5.0-aware). Original game scripts and recovered source remain
in a user-selected LOCAL output directory; never commit or upload them.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

LUA50 = b"\x1bLua\x50"
MAX_SCRIPT = 16 * 1024 * 1024
MAX_FILES = 8192
MAX_OUTPUT = 32 * 1024 * 1024


def candidates(source: Path, all_scripts=False) -> list[Path]:
    if not source.is_dir():
        raise ValueError("Input must be a locally extracted Ape Escape 3 asset directory")
    paths = []
    for path in source.rglob("*"):
        if path.is_symlink() or not path.is_file():
            continue
        if path.suffix.casefold() != ".luc":
            continue
        if not all_scripts and path.name.casefold() != "area.luc":
            continue
        paths.append(path)
        if len(paths) > MAX_FILES:
            raise ValueError("Unexpectedly many Lua scripts in source tree")
    return sorted(paths)


def recover(source: Path, out: Path, jar: Path, java: str = "java",
            all_scripts=False, *, runner=subprocess.run) -> dict:
    if not jar.is_file():
        raise FileNotFoundError("Provide a local unluac JAR that supports Lua 5.0")
    if not source.is_dir():
        raise FileNotFoundError(f"Extracted asset directory not found: {source}")
    source = source.resolve()
    out = out.resolve()
    if out == source or source in out.parents:
        raise ValueError("Output must be outside the original extracted asset tree")
    out.mkdir(parents=True, exist_ok=True)
    counts = {"recovered": 0, "invalid_or_different_version": 0,
              "failed": 0, "skipped_large": 0}
    reports = []
    files = candidates(source, all_scripts)
    for path in files:
        relative = path.relative_to(source)
        record = {"asset": relative.as_posix()}
        if path.stat().st_size > MAX_SCRIPT:
            counts["skipped_large"] += 1
            record["status"] = "skipped_large"
        else:
            with path.open("rb") as stream:
                header = stream.read(5)
            if header != LUA50:
                counts["invalid_or_different_version"] += 1
                record["status"] = "not_standard_lua50"
                record["header_version"] = header[4] if header.startswith(b"\x1bLua") else None
            else:
                dest = out / relative.with_suffix(".lua")
                dest.parent.mkdir(parents=True, exist_ok=True)
                try:
                    # User-provided Java unluac is an offline decompiler tool,
                    # never distributed with or invoked by Getchu3Native.exe.
                    result = runner([java, "-jar", str(jar), str(path)],
                                    capture_output=True, timeout=120, check=False)
                    if result.returncode != 0 or not result.stdout or len(result.stdout) > MAX_OUTPUT:
                        raise ValueError("Lua 5.0 decompiler rejected script or output exceeded size limit")
                    with tempfile.NamedTemporaryFile("wb", dir=dest.parent,
                                                     prefix=".lua-recovered-",
                                                     delete=False) as stream:
                        stream.write(result.stdout)
                        temporary = Path(stream.name)
                    try:
                        temporary.replace(dest)
                    finally:
                        temporary.unlink(missing_ok=True)
                    counts["recovered"] += 1
                    record["status"] = "recovered"
                    record["source"] = str(dest.relative_to(out).as_posix())
                except (ValueError, OSError, subprocess.TimeoutExpired) as exc:
                    counts["failed"] += 1
                    record["status"] = "unluac_failed"
                    record["reason"] = type(exc).__name__
        reports.append(record)
    report = {"format": "getchu3-local-lua50-recovery/v1",
              "native_runtime_java_dependency": False,
              "requires_original_owned_assets": True,
              "script_files_found": len(files),
              "counters": counts,
              "files": reports}
    (out / "lua_recovery_manifest.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--extracted", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--unluac", type=Path, required=True)
    parser.add_argument("--java", default="java")
    parser.add_argument("--all-scripts", action="store_true")
    args = parser.parse_args(argv)
    report = recover(args.extracted, args.out, args.unluac,
                     args.java, args.all_scripts)
    print(json.dumps({k: v for k, v in report.items() if k != "files"}, indent=2))
    if not report["script_files_found"]:
        print("No area.luc files found. Extract DATA.BIN's nested SZ/PCK assets "
              "using Saru3VfiTool first.", file=sys.stderr)
        return 2
    return 1 if report["counters"]["failed"] else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError) as exc:
        print("LUA50_RECOVERY_ERROR:", exc, file=sys.stderr)
        sys.exit(1)
