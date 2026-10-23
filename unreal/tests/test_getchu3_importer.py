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
sys.path.insert(0, str(FILE.parent))
import getchu3_media
import getchu3_collision
import getchu3_stage_catalog
import getchu3_stage_integrity
import getchu3_asset_registry
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
                             ['/Game/ApeEscape3/Original/Models/debug/us/stage/SourceMesh'])
    def test_unreal_import_sets_sanitized_source_name(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp)
            original = folder / "woods_c" / "Hero One.glb"
            original.parent.mkdir(parents=True)
            original.write_bytes(b"glb test fixture")
            requests = []
            class InspectManager:
                def import_asset_tasks(self, tasks):
                    for task in tasks:
                        requests.append((task.destination_path, task.destination_name))
                        task.objects = [type("OneObject", (), {
                            "get_path_name": lambda self: (
                                "/Game/ApeEscape3/Original/Models/woods_c/Hero_One")})()]
            class InspectUE(FakeUE):
                class AssetToolsHelpers:
                    @staticmethod
                    def get_asset_tools():
                        return InspectManager()
            results = module.import_glbs_in_editor(folder, InspectUE())
            self.assertEqual(requests, [
                ("/Game/ApeEscape3/Original/Models/woods_c", "Hero_One")])
            self.assertEqual(len(results), 1)

    def test_source_package_collision_detected_before_any_unreal_import(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp)
            (folder / "Hero One.glb").write_bytes(b"model A")
            (folder / "Hero-One.glb").write_bytes(b"model B")
            class MustNotImport:
                def import_asset_tasks(self, _):
                    raise AssertionError("Unreal import should not have started")
            class SilentUE(FakeUE):
                class AssetToolsHelpers:
                    @staticmethod
                    def get_asset_tools():
                        return MustNotImport()
            with self.assertRaisesRegex(RuntimeError, "identical Unreal package"):
                module.import_glbs_in_editor(folder, SilentUE())

    def test_interchange_same_uobject_path_from_distinct_sources_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp)
            (folder / "ape_a.glb").write_bytes(b"model A")
            (folder / "ape_b.glb").write_bytes(b"model B")
            with self.assertRaisesRegex(RuntimeError, "same Unreal object"):
                module.import_glbs_in_editor(folder, FakeUE())

    def test_collision_package_alias_preflight_before_import(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp)
            (folder / "floor One.obj").write_text("v 0 0 0\n")
            (folder / "floor-One.obj").write_text("v 0 0 0\n")
            class CollisionUE(FakeUE):
                StaticMesh = type("Mesh", (), {})
                CollisionTraceFlag = type("Trace", (), {})
            with self.assertRaisesRegex(RuntimeError, "identical Unreal package"):
                module.import_collision_in_editor(folder, CollisionUE())

    def test_original_animation_association_is_never_guessed(self):
        with tempfile.TemporaryDirectory() as d:
            base = Path(d)
            source = base / "vfi"; source.mkdir()
            out = base / "models"
            converter = base / "ae3.exe"
            converter.write_bytes(b"stub executable: tests only")
            seen = []
            class FakeNativeConverter:
                @staticmethod
                def export_models(*args, **kwargs):
                    seen.append(kwargs)
                    return {"results": {"converted": 1}, "original_i3d_bin_files": 1}
            with patch.object(module.importlib, 'import_module', return_value=FakeNativeConverter()):
                module.convert_original_models(source, out, base, str(converter))
            self.assertEqual(len(seen), 1)
            self.assertIs(seen[0]["animations"], False)

    def test_pck_unpack_requests_original_collision_before_conversion(self):
        with tempfile.TemporaryDirectory() as d:
            base = Path(d)
            vfi = base / "vfi"
            vfi.mkdir()
            repo = base / "repo"
            repo.mkdir()
            tool = type("FakePckModule", (), {
                "unpack": staticmethod(lambda root, include_collision=False: {
                    "original_stage_pck_archives": 2 if root == vfi and include_collision else 0,
                    "owner_private_i3c_extraction_requested": include_collision,
                    "new_original_members_written": 6,
                    "counts": {".i3d": 4, ".tm2": 2}
                })
            })
            with patch.object(module.importlib, 'import_module', return_value=tool):
                outcome = module.unpack_original_pck_for_unreal(vfi, repo)
            self.assertEqual(outcome["original_stage_pck_archives"], 2)
            self.assertTrue(outcome["owner_private_i3c_extraction_requested"])

    def test_missing_original_stage_pck_is_not_marked_as_imported(self):
        with tempfile.TemporaryDirectory() as d:
            base=Path(d)
            root=base / "vfi"; root.mkdir()
            fake=type("FakePckModule", (), {
                "unpack": staticmethod(lambda *args, **kwargs: {
                    "original_stage_pck_archives": 0,
                    "owner_private_i3c_extraction_requested": True
                })
            })
            with patch.object(module.importlib, 'import_module', return_value=fake):
                with self.assertRaisesRegex(RuntimeError, "No genuine stage PCK"):
                    module.unpack_original_pck_for_unreal(root, base)

    def test_no_glb_fails(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(RuntimeError, 'No converted'):
                module.import_glbs_in_editor(Path(d), FakeUE())
    def test_failed_hash_preflight_replaces_stale_success_status(self):
        with tempfile.TemporaryDirectory() as d:
            base = Path(d)
            disc = base / 'not_original.iso'
            disc.write_bytes(b'bad')
            saved = base / 'Saved'
            saved.mkdir()
            report_file = saved / 'Getchu3ImportStatus.json'
            report_file.write_text(json.dumps({'status': 'old_success', 'gameplay_ready': True}))
            with self.assertRaisesRegex(ValueError, 'size'):
                module.run(str(disc), ue=FakeUE(), repo=base, private_root=saved)
            status = json.loads(report_file.read_text(encoding='utf-8'))
            self.assertEqual(status['status'], 'blocked')
            self.assertEqual(status['phase'], 'verify_original_disc')
            self.assertFalse(status['gameplay_ready'])

    def test_editor_progress_cancel_fails_instead_of_partial_success(self):
        class CancelTask:
            def __init__(self, count, message):
                self.count = count
            def __enter__(self):
                return self
            def __exit__(self, exc_type, exc, tb):
                return False
            def make_dialog(self, cancellable):
                pass
            def should_cancel(self):
                return True
            def enter_progress_frame(self, *args):
                raise AssertionError('Must not import after cancellation')

        class CancelUE(FakeUE):
            ScopedSlowTask = CancelTask

        with tempfile.TemporaryDirectory() as d:
            base = Path(d)
            (base / "stage").mkdir()
            (base / "stage" / "original.glb").write_bytes(b"fake header")
            with self.assertRaisesRegex(InterruptedError, 'cancelled'):
                module.import_glbs_in_editor(base, CancelUE())

    def test_incomplete_gameplay_never_marked_ready(self):
        with tempfile.TemporaryDirectory() as d:
            base=Path(d)
            disk=base/'retail.iso'; disk.write_bytes(b'test')
            from getchu3_media import convert_textures
            with patch.object(module,'verify_disc',return_value=module.VerifiedDisc(disk,'a'*64)), \
                 patch('getchu3_media.convert_textures',return_value={
                     "converted_png_files":2, "pack_local_material_aliases":[]}), \
                 patch.object(module,'recover_original',return_value={"original_vfi_files_extracted":3994, "original_lua_scripts":168}), \
                 patch.object(module,'unpack_original_pck_for_unreal',return_value={
                     "original_stage_pck_archives": 2, "new_original_members_written": 6,
                     "counts": {".tm2": 2, ".i3d": 4}}), \
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
                 patch('getchu3_media.convert_textures',return_value={
                     "converted_png_files":2, "pack_local_material_aliases":[]}), \
                 patch('getchu3_media.convert_stream_audio',return_value={"source_exst":0,"wav_files":[]}), \
                 patch('getchu3_collision.convert_collision',return_value={"original_i3c_files":3}), \
                 patch.object(module,'import_collision_in_editor',return_value=['/Game/Original/Collision']), \
                 patch.object(module,'recover_original',return_value={"original_vfi_files_extracted":3994,"original_lua_scripts":168}), \
                 patch.object(module,'unpack_original_pck_for_unreal',return_value={
                     "original_stage_pck_archives": 2, "new_original_members_written": 6,
                     "counts": {".tm2": 2, ".i3d": 4}}), \
                 patch.object(module,'convert_original_models',return_value={"original_i3d_bin_files":12}), \
                 patch('getchu3_stage_catalog.build_original_stage_catalog',return_value={"stage_count":168,"stage_scripts":168}), \
                 patch('getchu3_stage_integrity.validate_stage_catalog',return_value={
                     "total_stage_ids":168,"verified_area_scripts":168,"gameplay_ready":False}), \
                 patch('getchu3_asset_registry.build_asset_registry',return_value={"original_stages":[{}] * 168}), \
                 patch.object(module,'import_glbs_in_editor',return_value=['/Game/Original/Mesh']):
                outcome=module.run(str(disk),ue=FakeUE(),repo=base,private_root=base/'saved')
                self.assertFalse(outcome['gameplay_ready'])
                self.assertIn('gameplay_not_implemented',outcome['status'])
                self.assertEqual(outcome['original_files'],3994)
                self.assertEqual(outcome['original_binding_evidence_status'], 'blocked')
                self.assertIn('original_binding_evidence_error', outcome)
                again=module.run(str(disk),ue=FakeUE(),repo=base,private_root=base/'saved')
                self.assertNotEqual(again['workspace'],outcome['workspace'])

if __name__=='__main__': unittest.main()
