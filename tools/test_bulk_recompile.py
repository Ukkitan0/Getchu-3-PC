"""Executable regression suite for full-game batch-entry discovery and tooling."""
from __future__ import annotations
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bulk_recompile as b


class BulkRecompileTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.words, cls.groups = b.load_inventory(b.INVENTORY)
        cls.calls = b.load_calls(b.STATIC_TARGETS, cls.words)
        cls.constructors = b.load_constructors(b.CONSTRUCTORS, cls.words)

    def test_game_inventory(self):
        self.assertEqual(len(self.words), 1351830)
        self.assertEqual(self.words[0x0041a390], 0x27bdfff0)
        self.assertEqual(sum(self.groups.values()), len(self.words))
        self.assertEqual(len(self.calls), 12228)

    def test_constructor_table_covers_every_callback(self):
        self.assertEqual(len(self.constructors), 143)
        self.assertEqual(self.constructors[0], (0x0062e89c, 0x0041a390))
        self.assertEqual(self.constructors[1], (0x0062e898, 0x00417c58))
        self.assertTrue(all(self.constructors[i][0] - 4 == self.constructors[i + 1][0]
                            for i in range(len(self.constructors) - 1)))
        self.assertEqual(len({c[1] for c in self.constructors}), 143)

    def test_entries_match_upstream_hex_schema(self):
        with tempfile.TemporaryDirectory() as tmp:
            report, path = b.write_outputs(Path(tmp), self.words, self.groups,
                                           self.calls, self.constructors, "all")
            lines = path.read_text().splitlines()
            numeric = [int(x, 16) for x in lines]
            self.assertEqual(len(numeric), 12373)
            self.assertEqual(numeric, sorted(set(numeric)))
            self.assertTrue(all(len(s) == 8 for s in lines))
            self.assertTrue(set(y for _, y in self.constructors).issubset(numeric))
            self.assertEqual(report['total_unique_seed_entries'], len(numeric))
            self.assertEqual(json.loads((Path(tmp) / 'report.json').read_text())
                             ['constructor_target_count'], 143)
            self.assertFalse((Path(tmp) / 'SCUS_975.01').exists())

    def test_default_only_seeds_indirect_callbacks_and_known_anchors(self):
        with tempfile.TemporaryDirectory() as tmp:
            report, path = b.write_outputs(Path(tmp), self.words, self.groups,
                                            self.calls, self.constructors)
            got = [int(x, 16) for x in path.read_text().split()]
            want = {*b.ANCHORS, *(t for _, t in self.constructors)}
            self.assertEqual(set(got), want)
            self.assertEqual(report['entry_profile'], 'constructors')
            self.assertEqual(report['direct_call_candidates_included_as_seeds'], 0)

    def test_common_seed_profile_is_bounded(self):
        with tempfile.TemporaryDirectory() as tmp:
            report, path = b.write_outputs(Path(tmp), self.words, self.groups,
                                            self.calls, self.constructors, 'common')
            self.assertEqual(report['direct_call_candidates_included_as_seeds'], 6442)
            self.assertEqual(len(path.read_text().split()), report['total_unique_seed_entries'])

    def test_unknown_versions_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            f = Path(tmp) / 'SCUS_975.01'
            f.write_bytes(b'wrong game data')
            with self.assertRaisesRegex(ValueError, 'Incorrect game executable SHA'):
                b.check_elf(f)

    def test_malformed_constructor_rows_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            f = Path(tmp) / 'bad.tsv'
            f.write_text('0062e89c 0041a390\n0062e89c 00417c58\n')
            with self.assertRaisesRegex(ValueError, 'Invalid constructor'):
                b.load_constructors(f, self.words)

    def test_mocked_upstream_batch_translation_command(self):
        with tempfile.TemporaryDirectory() as tmp:
            work = Path(tmp)
            elf = work / 'SCUS_975.01'
            elf.write_bytes(b'fixture')
            tool = work / 'eerecomp'
            tool.write_text('fixture')
            out = work / 'cpp'
            entries = work / 'entries'
            with patch.object(b, 'check_elf', return_value=b.EXPECTED_ELF_SHA), \
                    patch.object(b, 'compile_cached', return_value={
                        'mode': 'translated', 'units': 1, 'gameplay_verified': False
                    }) as run:
                result = b.main(['--elf', str(elf), '--eerecomp', str(tool),
                                 '--out', str(entries), '--cpp-out', str(out)])
            self.assertEqual(result, 0)
            run.assert_called_once()
            args = run.call_args.args
            self.assertEqual(args[0], tool)
            self.assertEqual(args[1], elf)
            self.assertEqual(args[2], entries / 'entries.txt')
            self.assertEqual(args[3], out)
            self.assertTrue(out.is_dir())


if __name__ == '__main__':
    unittest.main(verbosity=2)