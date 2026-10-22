"""Static original-Lua evidence reconciliation tests; synthetic reports only."""
import unittest
import reconcile_original_lua_bindings as bindings


def lua_report():
    return {
        'schema': 'getchu3-original-lua50-global-dependencies/v1',
        'scripts': [{'path': 'area.luc'}],
        'global_writes': ['setupArea', 'localStageHelper'],
        'global_reads': ['new_ent', 'gw_get_string', 'localStageHelper',
                         'missingOtherHelper', 'assert'],
        'direct_global_calls': ['new_ent', 'gw_get_string',
                                'localStageHelper', 'missingOtherHelper'],
    }


def elf_report():
    return {
        'original_verified_elf_sha256': bindings.elf.US_SHA256,
        'registrations': [
            {'name': 'new_ent', 'register_jal': '0x00200000'},
            {'name': 'gw_get_string', 'register_jal': '0x00200020'},
            {'name': 'gw_get_string', 'register_jal': '0x00200044'},
            {'name': 'unusedGameFunction', 'register_jal': '0x00200050'},
        ],
    }


class CrossReferenceTests(unittest.TestCase):
    def test_report_prioritizes_verified_original_function_without_host(self):
        result = bindings.reconcile(lua_report(), elf_report(), ['new_ent', 'assert'])
        rows = {x['name']: x for x in result['rows']}
        self.assertEqual(result['retail_scripts_count'], 1)
        self.assertEqual(rows['gw_get_string']['priority'],
                         'original_direct_call_missing_native_host')
        self.assertEqual(rows['gw_get_string']['original_register_sites'],
                         ['0x00200020', '0x00200044'])
        self.assertEqual(rows['missingOtherHelper']['priority'],
                         'unresolved_direct_call_requires_additional_source_proof')
        self.assertNotEqual(rows['localStageHelper']['priority'],
                            'original_direct_call_missing_native_host')
        self.assertEqual(rows['unusedGameFunction']['priority'],
                         'retail_registration_not_yet_in_native_host')
        self.assertEqual(rows['new_ent']['priority'],
                         'present_name_semantics_not_verified')

    def test_refuses_fake_executable_provenance(self):
        bad = elf_report()
        bad['original_verified_elf_sha256'] = '00' * 32
        with self.assertRaises(ValueError):
            bindings.reconcile(lua_report(), bad, ['new_ent'])

    def test_refuses_fabricated_direct_call_evidence(self):
        bad = lua_report()
        bad['direct_global_calls'].append('never_loaded')
        with self.assertRaises(ValueError):
            bindings.reconcile(bad, elf_report(), [])

    def test_bad_format_is_not_misread(self):
        bad = lua_report()
        bad['schema'] = 'fake'
        with self.assertRaises(ValueError):
            bindings.reconcile(bad, elf_report(), [])


if __name__ == '__main__':
    unittest.main()
