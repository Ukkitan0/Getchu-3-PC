#!/usr/bin/env python3
"""Create a privacy-conscious native AE3 diagnostic archive, without game data.

The ZIP includes structured status and bounded sanitized text excerpts only.
No ISO, ELF, RAM dumps, screenshots, code images, save files, generated C++,
or game content is ever included, even if the user stores them in the workspace.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys
import zipfile

SCHEMA = "ae3-native-diagnostics/v1"
MAX_EXCERPT_LINES = 160
MAX_LINE_CHARS = 600
WINDOWS_PATH = re.compile(r"(?i)(?:[a-z]:\\(?:[^\s'\"<>|]+\\?)+)")
POSIX_PATH = re.compile(r"(?<!\w)/(?:home|Users|mnt|tmp)/[^\s'\"<>|]+")
ERROR_LINE = re.compile(r"(?:error|fatal|failed|exception|missing code|unhandled|"
                        r"cannot |stopped at|halted|no translated|vulkan|WebGPU)", re.I)
REPORTS = ("boot_report.json", "explore_summary.json",
           "windows_port_report.json")


def redact(value: str) -> str:
    value = WINDOWS_PATH.sub("<LOCAL_PATH>", value)
    value = POSIX_PATH.sub("<LOCAL_PATH>", value)
    return value[:MAX_LINE_CHARS]


def safe_value(value):
    if isinstance(value, dict):
        return {key: safe_value(item) for key, item in value.items()
                if key not in ("disc", "executable", "trace", "stderr",
                               "stdout", "log_dir", "modules_dir",
                               "overlays_dir", "elf")}
    if isinstance(value, list):
        return [safe_value(item) for item in value[:2000]]
    if isinstance(value, str):
        return redact(value)
    return value


def diagnostic_archive(workspace: Path, destination: Path) -> dict:
    if not workspace.is_dir():
        raise FileNotFoundError(f"Native workspace does not exist: {workspace}")
    reports = {}
    for name in REPORTS:
        path = workspace / name
        if not path.is_file():
            continue
        # Json metadata only, never copy raw supplied files into ZIP.
        if path.stat().st_size > 8 * 1024 * 1024:
            continue
        try:
            reports[name] = safe_value(json.loads(path.read_text(encoding="utf-8-sig")))
        except (ValueError, UnicodeError, OSError):
            reports[name] = {"error": "malformed local report"}

    excerpt = {}
    candidates = [workspace / "build.log", *sorted(workspace.glob("explore_round_*.stderr.txt"))]
    for path in candidates[:80]:
        if not path.is_file():
            continue
        # Work with real error strings but never include arbitrarily dumped
        # guest instruction bytes, binary blobs, complete private build logs,
        # or full local account/installation paths.
        lines = []
        try:
            with path.open("r", encoding="utf-8", errors="replace") as stream:
                for i, line in enumerate(stream):
                    if i >= 350000:
                        break
                    if ERROR_LINE.search(line):
                        lines.append(redact(line.strip()))
                        if len(lines) >= MAX_EXCERPT_LINES:
                            break
        except OSError:
            continue
        excerpt[path.name] = lines

    payload = {
        "format": SCHEMA,
        "contains_retail_assets": False,
        "contains_elf_or_iso": False,
        "contains_screenshots": False,
        "status": "diagnostic evidence only; this does not certify a playable game",
        "reports": reports,
        "error_excerpts": excerpt,
    }
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Explicitly write just this one generated JSON member: never wildcard
    # a private folder into an archive.
    with zipfile.ZipFile(destination, mode="w", compression=zipfile.ZIP_DEFLATED) as out:
        out.writestr("AE3_NATIVE_DIAGNOSTICS.json", json.dumps(payload, indent=2) + "\n")
    return {"archive": str(destination), "members": ["AE3_NATIVE_DIAGNOSTICS.json"],
            "contains_game_data": False, "reports": len(reports),
            "error_log_files": len(excerpt)}


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--workspace", type=Path, required=True)
    p.add_argument("--out", type=Path)
    a = p.parse_args(argv)
    target = a.out or a.workspace / "AE3_NATIVE_DIAGNOSTICS.zip"
    result = diagnostic_archive(a.workspace, target)
    print("AE3_NATIVE_DIAGNOSTICS " + json.dumps(result))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print("DIAGNOSTIC_ARCHIVE_ERROR:", exc, file=sys.stderr)
        raise SystemExit(1)
