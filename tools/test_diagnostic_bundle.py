"""Game-free regression suite for private native AE3 diagnostics archives."""
from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
import zipfile

import diagnostic_bundle as diag


class DiagnosticZipTests(unittest.TestCase):
    def test_archive_contains_only_generated_sanitized_json_not_game_data(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            workspace = root / "private"
            workspace.mkdir()
            # These are fake proprietary-looking files. Absolutely none may
            # enter the diagnostic archive, including screens, RAM and C++.
            for filename in ("SCUS_975.01", "Ape Escape 3.iso", "explore_ram.bin",
                             "captured.png", "generated.cpp", "save.ps2"):
                (workspace / filename).write_bytes(("SECRET:" + filename).encode())
            (workspace / "build.log").write_text(
                "INFO: compiling\n"
                "FAILED to open C:\\Users\\Example\\PrivateGame\\game.iso\n"
                "error: missing code at 0x00428AE8\n",
                encoding="utf-8")
            (workspace / "boot_report.json").write_text(
                json.dumps({
                    "disc": "C:\\Users\\Example\\PrivateGame\\game.iso",
                    "rounds": [{"stderr": "C:\\Users\\Example\\PrivateGame\\log.txt",
                                "new_code_entries": 7}],
                    "original_game_execution_verified": False
                }), encoding="utf-8")
            archive = root / "diagnostics.zip"
            report = diag.diagnostic_archive(workspace, archive)
            self.assertFalse(report["contains_game_data"])
            with zipfile.ZipFile(archive) as zipf:
                self.assertEqual(zipf.namelist(), ["AE3_NATIVE_DIAGNOSTICS.json"])
                raw = zipf.read("AE3_NATIVE_DIAGNOSTICS.json")
                payload = json.loads(raw)
                self.assertEqual(payload["format"], "ae3-native-diagnostics/v1")
                self.assertEqual(payload["reports"]["boot_report.json"]["rounds"][0]
                                 ["new_code_entries"], 7)
                self.assertNotIn(b"SECRET", raw)
                self.assertNotIn(b"Example", raw)
                self.assertNotIn(b"PrivateGame", raw)
                self.assertNotIn(b"stderr", raw)
                self.assertIn(b"<LOCAL_PATH>", raw)

    def test_missing_reports_and_logs_are_harmless(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder)
            diag.diagnostic_archive(base, base / "report.zip")
            with zipfile.ZipFile(base / "report.zip") as z:
                payload = json.loads(z.read(z.namelist()[0]))
            self.assertEqual(payload["reports"], {})
            self.assertEqual(payload["error_excerpts"], {})

    def test_truncates_errors_and_rejects_raw_unfiltered_lines(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder)
            with (base / "build.log").open("w", encoding="utf-8") as stream:
                for i in range(250):
                    stream.write("error: " + ("x" * 1000) + "\n")
                stream.write("NOISE user-owned iso secret game code\n")
            diag.diagnostic_archive(base, base / "out.zip")
            with zipfile.ZipFile(base / "out.zip") as z:
                content = json.loads(z.read(z.namelist()[0]))
            logs = content["error_excerpts"]["build.log"]
            self.assertEqual(len(logs), diag.MAX_EXCERPT_LINES)
            self.assertTrue(all(len(s) <= diag.MAX_LINE_CHARS for s in logs))
            self.assertNotIn("NOISE", str(content))

    def test_nonexisting_workspace_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(FileNotFoundError):
                diag.diagnostic_archive(Path(directory) / "absent",
                                        Path(directory) / "missing.zip")


if __name__ == "__main__":
    unittest.main()
