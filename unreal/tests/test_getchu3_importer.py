"""Offline tests for editor importer safety and mapping; never claim gameplay test."""
from __future__ import annotations
from pathlib import Path
import importlib.util
import hashlib
import json
import tempfile
import unittest
from unittest.mock import patch

FILE = (Path(__file__).resolve().parents[1] / 'Plugins/Getchu3Importer/Content/Python/getchu3_importer.py')
spec = importlib.util.spec_from_file_location('getchu3_importer', FILE)
import sys
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)

class FakeTask:
    def __init__(self):
        self.objects = []
    def get_objects(self):
        return self.objects

class FakeManager:
    def import_asset_tasks(self, tasks):
        for task in tasks:
            task.objects = [type('FakeObject', (), {'get_path_name': lambda self: task.destination_path + '/SourceMesh'})()]
        return None

class FakeUE:
    AssetImportTask = FakeTask
    class AssetToolsHelpers:
        @staticmethod
        def get_asset_tools():
            return FakeManager()

class ImporterTests(unittest.TestCase):
    def test_digest_and_size_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            disk = Path(d) / 'my.iso'
            disk.write_bytes(b'fake-disc')
            with self.assertRaisesRegex(ValueError, 'size'):
                module.verify_disc(disk)
            self.assertEqual(module.verify_disc(disk, expected_size=9,
                         expected_digest=hashlib.sha256(b'fake-disc').hexdigest()).path,disk)
            with self.assertRaisesRegex(ValueError, 'SHA-256'):
                module.verify_disc(disk, expected_size=9, expected_digest='0' * 64)
    def test_import_only_original_glbs_and_stable_path(self):
        with tempfile.TemporaryDirectory() as d:
            base=Path(d)
            (base/'debug/us/stage').mkdir(parents=True)
            (base/'debug/us/stage/Hero One.glb').write_bytes(b'glb test')
            self.assertEqual(module.import_glbs_in_editor(base, FakeUE()),
                             ['/Game/ApeEscape3/Original/debug/us/stage/SourceMesh'])
    def test_no_glb_fails(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(RuntimeError, 'No converted'):
                module.import_glbs_in_editor(Path(d), FakeUE())
    def test_incomplete_gameplay_never_marked_ready(self):
        with tempfile.TemporaryDirectory() as d:
            base=Path(d)
            disk=base/'retail.iso'; disk.write_bytes(b'test')
            with patch.object(module,'verify_disc',return_value=module.VerifiedDisc(disk,'a'*64)), \
                 patch.object(module,'recover_original',return_value={"original_vfi_files_extracted":3994, "original_lua_scripts":168}), \
                 patch.object(module,'convert_original_models',side_effect=RuntimeError('conversion unavailable')):
                with self.assertRaisesRegex(RuntimeError,'conversion unavailable'):
                    module.run(str(disk),ue=FakeUE(),repo=base,private_root=base/'saved')
            status=json.loads((base/'saved/Getchu3ImportStatus.json').read_text())
            self.assertEqual(status['status'],'blocked')
            self.assertFalse(status['gameplay_ready'])
    def test_complete_asset_import_is_not_called_playable(self):
        with tempfile.TemporaryDirectory() as d:
            base=Path(d); disk=base/'retail.iso';disk.write_bytes(b'test')
            with patch.object(module,'verify_disc',return_value=module.VerifiedDisc(disk,'b'*64)), \
                 patch.object(module,'recover_original',return_value={"original_vfi_files_extracted":3994,"original_lua_scripts":168}), \
                 patch.object(module,'convert_original_models',return_value={"original_i3d_bin_files":12}), \
                 patch.object(module,'import_glbs_in_editor',return_value=['/Game/Original/Mesh']):
                outcome=module.run(str(disk),ue=FakeUE(),repo=base,private_root=base/'saved')
                self.assertFalse(outcome['gameplay_ready'])
                self.assertIn('gameplay_not_implemented',outcome['status'])
                self.assertEqual(outcome['original_files'],3994)
                again=module.run(str(disk),ue=FakeUE(),repo=base,private_root=base/'saved')
                self.assertNotEqual(again['workspace'],outcome['workspace'])

if __name__=='__main__': unittest.main()
