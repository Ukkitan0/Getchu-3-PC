"""Metadata-only native boot diagnostic categorization tests."""
import json
from pathlib import Path
import tempfile
import unittest

import boot_report


class NativeBootReportTests(unittest.TestCase):
    def test_no_exploration_reports_missing_evidence_not_success(self):
        with tempfile.TemporaryDirectory() as folder:
            report = boot_report.analyze(Path(folder))
            self.assertFalse(report["original_game_execution_verified"])
            self.assertEqual(report["rounds"], [])
            self.assertIn("No exploration rounds", report["prioritized_next_investigation"])

    def test_detects_iop_syscall_overlays_frames_and_vu_programs(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / "explore_summary.json").write_text(json.dumps([
                {"round": 1, "runner_exit_code": 1, "new_code_entries": 44, "new_code_images": 1},
                {"round": 2, "runner_exit_code": 1, "new_code_entries": 0, "new_code_images": 0}]))
            (root / "explore_round_01.stderr.txt").write_text(
                "[iop] missing testlib #42\n[sys] unhandled syscall 0x5a\n")
            (root / "explore_round_02.stderr.txt").write_text(
                "[ee] halted: dynamic indirect address\n"
                "[ee] stopped after 540: Exit\n"
                "WebGPU initialize failed\n")
            (root / "shots").mkdir()
            (root / "shots" / "frame0030.png").write_bytes(b"")
            (root / "vu1").mkdir()
            (root / "vu1" / "micro.bin").write_bytes(b"abc")
            (root / "owned_disc" / "overlays").mkdir(parents=True)
            (root / "owned_disc" / "overlays" / "00300000_1234ABCD.bin").write_bytes(b"")
            (root / "owned_disc" / "modules").mkdir()
            (root / "owned_disc" / "modules" / "test.snr").write_bytes(b"")
            (root / "owned_disc" / "disc_prepare.json").write_text(json.dumps({
                "overlays_dir": str(root / "owned_disc" / "overlays"),
                "modules_dir": str(root / "owned_disc" / "modules"),
                "code_windows": [{"base": "00300000", "size": 16384}]}))
            report = boot_report.analyze(root)
            self.assertEqual(report["rendered_frame_files"], 1)
            self.assertEqual(report["vu_programs_recorded"], 1)
            self.assertEqual(report["distinct_code_window_images"], 1)
            self.assertEqual(report["disc_modules"], 1)
            self.assertEqual(report["code_windows_declared"], 1)
            self.assertEqual(report["signal_counts"]["missing_iop_import"], 1)
            self.assertEqual(report["signal_counts"]["unhandled_ee_syscall"], 1)
            self.assertEqual(report["signal_counts"]["gpu_issue"], 1)
            self.assertIn("Unimplemented EE kernel", report["prioritized_next_investigation"])
            self.assertFalse(report["original_game_execution_verified"])

    def test_single_round_powershell_json_object(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / "explore_summary.json").write_text(json.dumps({
                "round": 1, "runner_exit_code": 0,
                "new_code_entries": 0, "new_code_images": 0}))
            report = boot_report.analyze(root)
            self.assertEqual(len(report["rounds"]), 1)
            self.assertIn("No new EE code", report["prioritized_next_investigation"])

    def test_rejects_corrupted_round_report(self):
        with tempfile.TemporaryDirectory() as folder:
            (Path(folder) / "explore_summary.json").write_text('"hello"')
            with self.assertRaisesRegex(ValueError,"Malformed"):
                boot_report.analyze(Path(folder))

if __name__ == "__main__":
    unittest.main()
