"""Native source-only screenshot probes for P6 output from pinned PS2 runtime."""
from __future__ import annotations

from pathlib import Path
import tempfile
import unittest

import frame_probe
import boot_report


def write_ppm(path: Path, width: int, height: int, pixels: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(f"P6\n{width} {height}\n255\n".encode() + pixels)


class NativePpmImageEvidence(unittest.TestCase):
    def test_detects_uniform_black_real_ppm_frame(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / "shots"
            path = root / "frame_00300.ppm"
            write_ppm(path, 4, 4, bytes([0, 0, 0]) * 16)
            item = frame_probe.inspect_ppm(path)
            self.assertEqual((item["width"], item["height"]), (4, 4))
            self.assertEqual(item["classification"], "mostly_dark")
            self.assertEqual(item["mostly_dark_fraction"], 1)
            result = frame_probe.inspect_frames(root)
            self.assertEqual(result["ppm_frame_files"], 1)
            self.assertFalse(result["has_nonblack_visual_evidence"])

    def test_varied_scene_detected_and_included_in_boot_report(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            shots = root / "shots"
            frame = shots / "frame_00600.ppm"
            pixels = bytes([0, 0, 0, 255, 50, 0, 0, 250, 0, 0, 0, 250] * 4)
            write_ppm(frame, 4, 4, pixels)
            found = frame_probe.inspect_frames(shots)
            self.assertEqual(found["ppm_frame_files"], 1)
            self.assertTrue(found["has_nonblack_visual_evidence"])
            self.assertEqual(found["examined"][0]["classification"], "varied_pixels")
            report = boot_report.analyze(root)
            self.assertEqual(report["rendered_frame_files"], 1)
            self.assertEqual(report["rendered_ppm_frame_files"], 1)
            self.assertTrue(report["frame_visual_evidence"]["has_nonblack_visual_evidence"])
            self.assertFalse(report["original_game_execution_verified"])

    def test_binary_pixel_whitespace_does_not_extend_ppm_header(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "frame_00001.ppm"
            # First pixel starts with ASCII newline; still raw pixel data.
            write_ppm(path, 2, 1, b"\n\x20\x09\x40\x40\x40")
            found = frame_probe.inspect_ppm(path)
            self.assertEqual(found["sampled_pixels"], 2)
            self.assertEqual(found["sampled_unique_colors"], 2)

    def test_invalid_and_truncated_frames_are_counted_not_crashes(self):
        with tempfile.TemporaryDirectory() as tmp:
            shots = Path(tmp)
            write_ppm(shots / "frame_00001.ppm", 10, 10, b"bad")
            write_ppm(shots / "frame_00002.ppm", 1, 1, b"\x20\x20\x20")
            result = frame_probe.inspect_frames(shots)
            self.assertEqual(result["ppm_frame_files"], 2)
            self.assertEqual(result["bad_frames"], 1)
            self.assertEqual(len(result["examined"]), 1)

    def test_no_images_and_unsupported_file_types(self):
        with tempfile.TemporaryDirectory() as tmp:
            shots = Path(tmp)
            (shots / "screen.png").write_bytes(b"\x00")
            self.assertEqual(frame_probe.inspect_frames(shots)["ppm_frame_files"], 0)
            self.assertFalse(frame_probe.inspect_frames(shots)["has_nonblack_visual_evidence"])

    def test_large_frame_dimensions_and_bad_header_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "frame_00010.ppm"
            path.write_bytes(b"P6\n99999 99999\n255\n")
            with self.assertRaises(ValueError):
                frame_probe.inspect_ppm(path)
            path.write_bytes(b"P3\n1 1\n255\n0 0 0")
            with self.assertRaisesRegex(ValueError, "Not an 8-bit"):
                frame_probe.inspect_ppm(path)


if __name__ == "__main__":
    unittest.main(verbosity=2)
