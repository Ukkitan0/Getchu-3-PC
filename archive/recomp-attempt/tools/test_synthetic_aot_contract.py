"""Game-free integration contract for the native Win32 pipeline smoke script."""
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parent

class AotContract(unittest.TestCase):
    def test_true_end_to_end_recompiler_and_runner(self):
        source = (ROOT/'run_synthetic_aot_windows.ps1').read_text()
        for required in ('--entries', '--out', '-DPS2_GENERATED=',
                         "'ps2run'", "'--target'", "'synthetic_aot_smoke.py'",
                         "'00012000,4'", '00001234', 'Exit'):
            self.assertIn(required, source)

    def test_no_commercial_assets_or_external_binaries_embedded(self):
        fixture = (ROOT/'synthetic_aot_smoke.py').read_text()
        self.assertNotIn('SCUS_975', fixture)
        self.assertNotIn('5a8a73cf', fixture)
        self.assertNotIn('https:', fixture)
        self.assertIn('0x0320F809', fixture)  # JALR dynamic callback

    def test_143_callback_fixture_is_supported(self):
        source = (ROOT/'run_synthetic_aot_windows.ps1').read_text()
        for needle in ("'constructors'", 'synthetic_ctors_smoke.py',
                       "'0000008f'", '$ExpectedSeeds = 143',
                       '$ExpectedRam'):
            self.assertIn(needle, source)

    def test_nonlocal_page_fixture_is_wired_to_native_runner(self):
        source = (ROOT/'run_synthetic_aot_windows.ps1').read_text()
        pipeline = (ROOT.parent/'.github/workflows/windows-fastlane-smoke.yml').read_text()
        for needle in ("'pages'", 'synthetic_pages_smoke.py', 'ExpectedSeeds = 143'):
            self.assertIn(needle,source)
        self.assertIn('-FixtureMode pages',pipeline)
        self.assertIn('ae3-aot-pages/aot-smoke-result.json',pipeline)

    def test_failure_and_timeout_handling(self):
        source = (ROOT/'run_synthetic_aot_windows.ps1').read_text()
        for required in ('Stop-Process', 'WaitForExit', 'finally',
                         '$process.ExitCode', 'eerecomp emitted zero',
                         'AOT ps2run exited', 'Get-Content -LiteralPath $log'):
            self.assertIn(required, source)

if __name__ == '__main__':
    unittest.main()