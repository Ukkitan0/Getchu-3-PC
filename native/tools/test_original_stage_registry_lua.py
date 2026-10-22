"""Exercise the actual native executable with an owner's private retail ISO."""
import argparse
import subprocess
import unittest

EXE = ISO = None

class RegistryLuaTests(unittest.TestCase):
    def setUp(self):
        if not EXE or not ISO:
            self.skipTest('requires explicit native executable and owner-private ISO')

    def run_probe(self, *args):
        return subprocess.run([EXE, ISO, *args], capture_output=True, text=True, timeout=60)

    def test_real_stage_executes_through_owned_registry_without_guessed_spawn(self):
        result = self.run_probe('woods_c')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('ORIGINAL_STAGE_REGISTRY_LUA_CONNECTED: true', result.stdout)
        self.assertIn('ORIGINAL_GAME_LUA_CHUNK_EXECUTED: true', result.stdout)
        self.assertIn('ORIGINAL_GAME_SETUPAREA_NOT_REQUESTED', result.stdout)
        self.assertNotIn('ORIGINAL_GAME_SETUPAREA_COMPLETED', result.stdout)

    def test_nonexistent_stage_is_not_substituted(self):
        result = self.run_probe('codex_missing_stage')
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('Stage folder absent', result.stderr)
        self.assertNotIn('ORIGINAL_GAME_LUA_CHUNK_EXECUTED: true', result.stdout)

    def test_explicit_setup_reports_missing_original_engine(self):
        result = self.run_probe('woods_c', 'spawn_default')
        self.assertEqual(result.returncode, 3, result.stdout + result.stderr)
        self.assertIn('ORIGINAL_GAME_SETUPAREA_BLOCKED', result.stderr)
        self.assertNotIn('ORIGINAL_GAME_SETUPAREA_COMPLETED', result.stdout)
        self.assertIn('Stage is NOT PLAYABLE', result.stderr)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe')
    parser.add_argument('iso')
    args = parser.parse_args()
    EXE, ISO = args.exe, args.iso
    unittest.main(argv=[__file__])
