#!/usr/bin/env python3
"""Concise, owner-local diagnostics from a native PS2 recompilation attempt.

Works even if the game stops at its first unimplemented syscall. Never uploads
guest RAM, game code, screenshots or the original ISO; emits metadata only.
"""
from __future__ import annotations
import argparse
from collections import Counter
import json
from pathlib import Path
import re
import sys

from frame_probe import inspect_frames

SIGNALS = (
    ("unhandled_ee_syscall", re.compile(r"\[sys\].*unhandled syscall", re.I)),
    ("missing_iop_import", re.compile(r"\[iop\].*missing .*#\d+", re.I)),
    ("guest_halt", re.compile(r"\[ee\] halted:", re.I)),
    ("guest_exit", re.compile(r"\[ee\] stopped after .*Exit", re.I)),
    ("gpu_issue", re.compile(r"(?:WebGPU|wgpu).*?(?:failed|error|unavailable)", re.I)),
    # Matches the pinned upstream dispatch.cpp stderr, not just generic wording.
    ("missing_compiled_code", re.compile(
        r"\[ee\]\s+(?:no translated code at|halted: missing code at)", re.I)),
)
MARKER_LIMIT = 12
FRAMES = re.compile(r"\[ee\] stopped after (\d+) frames,\s+(\d+) cycles:\s*(.*)")


def analyze(workspace: Path) -> dict:
    summaries_path = workspace / "explore_summary.json"
    try:
        rounds = json.loads(summaries_path.read_text(encoding="utf-8-sig"))
    except FileNotFoundError:
        rounds = []
    if isinstance(rounds, dict):
        rounds = [rounds]
    if not isinstance(rounds, list):
        raise ValueError("Malformed exploration summary JSON")
    counts = Counter()
    examples: dict[str, list[str]] = {}
    observed_frames = []
    last_halt_reason = None
    for stream in sorted(workspace.glob("explore_round_*.stderr.txt")):
        for line in stream.read_text(encoding="utf-8", errors="replace").splitlines():
            frame = FRAMES.search(line)
            if frame:
                observed_frames.append(int(frame.group(1)))
                last_halt_reason = frame.group(3).strip()
            for label, pattern in SIGNALS:
                if pattern.search(line):
                    counts[label] += 1
                    samples = examples.setdefault(label, [])
                    if len(samples) < MARKER_LIMIT:
                        samples.append(line.strip()[:350])
    prep_file = workspace / "owned_disc" / "disc_prepare.json"
    prep = json.loads(prep_file.read_text(encoding="utf-8")) if prep_file.is_file() else {}
    shots_dir = workspace / "shots"
    supported = {".ppm", ".png", ".bmp", ".jpg", ".jpeg"}
    shots = [p for p in shots_dir.rglob("*") if p.is_file() and p.suffix.lower() in supported] \
        if shots_dir.exists() else []
    visual = inspect_frames(shots_dir)
    vu = [p for p in (workspace / "vu1").glob("*.bin") if p.is_file()]         if (workspace / "vu1").exists() else []
    overlays_dir = prep.get("overlays_dir")
    modules_dir = prep.get("modules_dir")
    overlays = list(Path(overlays_dir).glob("*.bin")) if overlays_dir and Path(overlays_dir).is_dir() else []
    modules = list(Path(modules_dir).glob("*.snr")) if modules_dir and Path(modules_dir).is_dir() else []
    last = rounds[-1] if rounds else None
    if counts["unhandled_ee_syscall"]:
        hint = "Unimplemented EE kernel syscall; inspect guest syscall number/arguments."
    elif counts["missing_iop_import"]:
        hint = "Unimplemented IOP module library import; inspect module/library index."
    elif counts["missing_compiled_code"]:
        hint = "Indirect EE address has no compiled destination; inspect misses and code windows."
    elif counts["gpu_issue"]:
        hint = "WebGPU initialization/rendering failure; compare GPU and software native paths."
    elif not rounds:
        hint = "No exploration rounds recorded; inspect the build log and launch prerequisites."
    elif last and last.get("new_code_entries", 0) == 0 and last.get("new_code_images", 0) == 0:
        hint = "No new EE code found on final pass; inspect last guest halt and screenshots."
    else:
        hint = "More guest code/overlays were discovered; additional exploration may help."
    return {
        "format": "ae3-native-boot-report/v1",
        "original_game_execution_verified": False,
        "note": "Metadata-only diagnostic; passing compilation does not certify gameplay or graphics.",
        "rounds": rounds,
        "signal_counts": dict(sorted(counts.items())),
        "signal_examples": examples,
        "maximum_reported_guest_frames": max(observed_frames, default=0),
        "last_guest_halt_reason": last_halt_reason,
        "reached_guest_frame_execution": any(n > 0 for n in observed_frames),
        "rendered_frame_files": len(shots),
        "frame_visual_evidence": visual,
        "rendered_ppm_frame_files": visual["ppm_frame_files"],
        "vu_programs_recorded": len(vu),
        "distinct_code_window_images": len(overlays),
        "disc_modules": len(modules),
        "code_windows_declared": len(prep.get("code_windows", [])),
        "prioritized_next_investigation": hint,
        "log_dir": str(workspace),
    }


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--workspace", type=Path, required=True)
    a = p.parse_args(argv)
    report = analyze(a.workspace)
    target = a.workspace / "boot_report.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("AE3 NATIVE BOOT REPORT")
    print(f"rounds={len(report['rounds'])} frames={report['rendered_frame_files']} "
          f"VU1={report['vu_programs_recorded']} overlays={report['distinct_code_window_images']} "
          f"IOP-modules={report['disc_modules']}")
    for name, count in report["signal_counts"].items():
        print(f"{name}: {count}")
    print("Next investigation:", report["prioritized_next_investigation"])
    print("Saved:", target)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError) as error:
        print(f"BOOT_REPORT_ERROR: {error}", file=sys.stderr)
        sys.exit(1)
