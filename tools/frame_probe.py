#!/usr/bin/env python3
"""Inspect source-only metadata about native PS2 PPM captures.

The pinned PS2 runtime writes P6 binary frame_XXXXX.ppm images. These probes
count frames and quantify black/uniform/varied output without carrying ANY
pixel bytes into reports or diagnostic archives.
"""
from __future__ import annotations

import json
from pathlib import Path
import re
import sys

MAX_FRAME_BYTES = 256 * 1024 * 1024
MAX_DIMENSION = 8192
MAX_SAMPLES = 4096
HEADER = re.compile(rb"\AP6[ \t\r\n]+([0-9]{1,5})[ \t\r\n]+([0-9]{1,5})[ \t\r\n]+255\r?\n")


def inspect_ppm(path: Path) -> dict:
    if path.stat().st_size > MAX_FRAME_BYTES:
        raise ValueError("PPM exceeds safe frame-size limit")
    data = path.read_bytes()
    match = HEADER.match(data[:128])
    if not match:
        raise ValueError("Not an 8-bit binary P6 PPM image")
    width, height = int(match.group(1)), int(match.group(2))
    if not 1 <= width <= MAX_DIMENSION or not 1 <= height <= MAX_DIMENSION:
        raise ValueError("Invalid PPM frame dimensions")
    pixel_count = width * height
    start = match.end()
    expected = pixel_count * 3
    if len(data) != start + expected:
        raise ValueError("PPM pixel data length inconsistent with dimensions")
    step = max(1, (pixel_count + MAX_SAMPLES - 1) // MAX_SAMPLES)
    samples = 0
    dark = 0
    bright = 0
    distinct: set[bytes] = set()
    intensity = 0
    for pixel in range(0, pixel_count, step):
        point = data[start + pixel * 3: start + pixel * 3 + 3]
        samples += 1
        distinct.add(point)
        maximum = max(point)
        intensity += sum(point)
        if maximum <= 12:
            dark += 1
        if maximum >= 48:
            bright += 1
    dark_fraction = round(dark / samples, 4)
    bright_fraction = round(bright / samples, 4)
    mean_brightness = round(intensity / (3 * samples), 2)
    if dark_fraction >= 0.995:
        label = "mostly_dark"
    elif len(distinct) <= 3:
        label = "nearly_uniform"
    else:
        label = "varied_pixels"
    return {
        "file": path.name,
        "width": width,
        "height": height,
        "sampled_pixels": samples,
        "sampled_unique_colors": len(distinct),
        "mean_channel_intensity": mean_brightness,
        "mostly_dark_fraction": dark_fraction,
        "bright_pixel_fraction": bright_fraction,
        "classification": label,
    }


def inspect_frames(folder: Path, *, max_frames: int = 12) -> dict:
    if not folder.exists():
        return {"ppm_frame_files": 0, "examined": [], "bad_frames": 0,
                "has_nonblack_visual_evidence": False}
    candidates = sorted(folder.glob("frame_*.ppm"))
    selected = candidates[-max_frames:] if max_frames > 0 else []
    examined = []
    bad = 0
    for path in selected:
        try:
            examined.append(inspect_ppm(path))
        except (OSError, ValueError):
            bad += 1
    return {
        "ppm_frame_files": len(candidates),
        "examined": examined,
        "bad_frames": bad,
        "has_nonblack_visual_evidence": any(
            x["classification"] != "mostly_dark"
            and x["bright_pixel_fraction"] > 0.01 for x in examined),
    }


def main(argv=None):
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--frames", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args(argv)
    report = inspect_frames(args.frames)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("AE3_PPM_FRAME_EVIDENCE " + json.dumps(report))
    return 0


if __name__ == "__main__":
    sys.exit(main())
