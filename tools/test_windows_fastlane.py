"""Pure offline checks for Windows upstream compatibility and round-trip discovery.

Mocks upstream anchors from the literal reviewed patch contracts; no ISO or
Sony code is required. It does not claim an upstream Windows compilation.
"""
import ast
import gzip
import json
from pathlib import Path
import tempfile
import unittest

import native_explore
import windows_upstream_patch


def literal(node):
    if isinstance(node, ast.Constant) and isinstance(node.value, str):
        return node.value
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add):
        return literal(node.left) + literal(node.right)
    raise ValueError(f'Nonliteral patch anchor: {ast.dump(node)[:140]}')


def pinned_anchors():
    source = Path(windows_upstream_patch.__file__).read_text(encoding='utf-8')
    syntax = ast.parse(source)
    result = []
    for x in ast.walk(syntax):
        if isinstance(x, ast.Call) and isinstance(x.func, ast.Name) and x.func.id == 'transform':
            if len(x.args) != 4:
                raise AssertionError('Patch call schema changed')
            result.append((literal(x.args[0]), literal(x.args[1]), literal(x.args[2]), literal(x.args[3])))
    return result


class Win32Overlay(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.anchors = pinned_anchors()
        self.assertGreaterEqual(len(self.anchors), 10)
        self.paths = {}
        for path, old, new, description in self.anchors:
            self.paths.setdefault(path, []).append(old)
        for path, contents in self.paths.items():
            fp = self.root / path
            fp.parent.mkdir(parents=True, exist_ok=True)
            fp.write_text('\n\n'.join(contents), encoding='utf-8')

    def tearDown(self):
        self.temp.cleanup()

    def test_patch_every_anchor(self):
        result = windows_upstream_patch.patch_sources(self.root)
        self.assertEqual(len(result), len(self.anchors))
        self.assertTrue(all(x['changed'] for x in result))
        for path, old, new, _ in self.anchors:
            text = (self.root / path).read_text()
            if old not in new:
                self.assertNotIn(old, text)
            self.assertIn(new, text)
        self.assertIn('WGPUSurfaceSourceWindowsHWND', (self.root/'runtime/src/gs/gpu/gpu.cpp').read_text())
        self.assertIn('GetProcessMemoryInfo', (self.root/'host/window.cpp').read_text())

    def test_idempotent(self):
        windows_upstream_patch.patch_sources(self.root)
        old = {p: (self.root / p).read_bytes() for p in self.paths}
        again = windows_upstream_patch.patch_sources(self.root)
        self.assertFalse(any(x['changed'] for x in again))
        self.assertEqual(old, {p: (self.root / p).read_bytes() for p in self.paths})

    def test_dry_run_does_not_change_file(self):
        before = {p: (self.root / p).read_bytes() for p in self.paths}
        r = windows_upstream_patch.patch_sources(self.root, dry_run=True)
        self.assertTrue(all(x['changed'] for x in r))
        self.assertEqual(before, {p: (self.root / p).read_bytes() for p in self.paths})

    def test_preflight_atomic_failure(self):
        victim = self.root/'CMakeLists.txt'
        victim.write_text('intentionally divergent upstream code', encoding='utf-8')
        before = {p: (self.root / p).read_bytes() for p in self.paths}
        with self.assertRaisesRegex(ValueError, 'expected exactly one original anchor'):
            windows_upstream_patch.patch_sources(self.root)
        self.assertEqual(before, {p: (self.root / p).read_bytes() for p in self.paths})

    def test_missing_source_preflight(self):
        (self.root/'runtime/src/gs/gpu/gpu.hpp').unlink()
        with self.assertRaises(FileNotFoundError):
            windows_upstream_patch.patch_sources(self.root)

    def test_double_original_anchor_rejected(self):
        p = self.root/'runtime/include/ps2/system.hpp'
        text = p.read_text()
        p.write_text(text + '\n' + text, encoding='utf-8')
        with self.assertRaisesRegex(ValueError, 'found 2'):
            windows_upstream_patch.patch_sources(self.root)


class MissingCodeDiscovery(unittest.TestCase):
    def setUp(self):
        self.t = tempfile.TemporaryDirectory()
        self.dir = Path(self.t.name)
        self.entries = self.dir/'entries.txt'
        self.misses = self.dir/'misses.txt'
        self.inventory = self.dir/'inventory.gz'
        with gzip.open(self.inventory, 'wt', encoding='ascii') as out:
            out.write('address\tword\n')
            for addr in range(0x100100,0x100120,4):
                out.write(f'{addr:08x}\t24020001\n')
        self.entries.write_text('00100100\n00100104\n')

    def tearDown(self):
        self.t.cleanup()

    def test_monotonic_merge_and_dedupe(self):
        self.misses.write_text('0x00100108\n0010010C\n00100108\n')
        r = native_explore.merge_entries(self.entries, self.misses, self.inventory)
        self.assertEqual(r['new_entries'], 2)
        self.assertEqual(r['total_entries'], 4)
        self.assertTrue(r['changed'])
        r2 = native_explore.merge_entries(self.entries, self.misses, self.inventory)
        self.assertEqual(r2['new_entries'], 0)
        self.assertFalse(r2['changed'])

    def test_missing_file_is_no_change(self):
        self.misses.unlink(missing_ok=True)
        self.assertFalse(native_explore.merge_entries(self.entries, self.misses, self.inventory)['changed'])

    def test_invalid_address_atomic(self):
        original = self.entries.read_bytes()
        self.misses.write_text('00ff0000\n')
        with self.assertRaisesRegex(ValueError, 'outside validated'):
            native_explore.merge_entries(self.entries, self.misses, self.inventory)
        self.assertEqual(original, self.entries.read_bytes())

    def test_unaligned_address_atomic(self):
        old = self.entries.read_bytes()
        self.misses.write_text('00100103\n')
        with self.assertRaisesRegex(ValueError, 'Unaligned'):
            native_explore.merge_entries(self.entries, self.misses, self.inventory)
        self.assertEqual(old, self.entries.read_bytes())

    def test_module_entry_preserved(self):
        self.misses.write_text('TEST.IRX:00001000\n')
        result=native_explore.merge_entries(self.entries, self.misses, self.inventory)
        self.assertEqual(result['module_entries'],1)
        self.assertIn('TEST.IRX:00001000',self.entries.read_text())

    def test_massive_discovery_guard(self):
        self.misses.write_text('00100108\n0010010c\n')
        with self.assertRaisesRegex(ValueError,'exceed safety budget'):
            native_explore.merge_entries(self.entries,self.misses,self.inventory,max_added=1)

    def test_malformed_token(self):
        self.misses.write_text('use this instruction now\n')
        with self.assertRaisesRegex(ValueError, 'Invalid indirect entry'):
            native_explore.merge_entries(self.entries,self.misses,self.inventory)


class LauncherContract(unittest.TestCase):
    def test_launcher_preserves_source_and_requires_native_sdl(self):
        ps1=(Path(__file__).resolve().parent.parent/'RUN_BULK_NATIVE_WINDOWS.ps1').read_text()
        self.assertIn('windows_upstream_patch.py', ps1)
        self.assertIn('-DAE3_REQUIRE_SDL2=ON', ps1)
        self.assertIn('ClangCL', ps1)
        self.assertIn('native_explore.py', ps1)
        self.assertIn("'vurecomp'", ps1)
        self.assertIn("'--window'", ps1)
        self.assertNotIn('wsl.exe', ps1.lower())


if __name__=='__main__':
    unittest.main(verbosity=2)