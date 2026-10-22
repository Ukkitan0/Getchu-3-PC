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

    def test_published_vtable_only_is_separate_evidence_not_confirmed_callback(self):
        report = elf_report()
        report['registrations'] = [r for r in report['registrations']
                                   if r['name'] != 'gw_get_string']
        report['published_flag_vtable_slot_candidates'] = [{
            'name': 'gw_get_string', 'vtable_address': '0x006B70C0',
            'available': True,
            'slots': [
                {'index': 0, 'pointer': '0x006175E8',
                 'points_into_original_text': True},
                {'index': 1, 'pointer': '0x00000000',
                 'points_into_original_text': False},
            ],
        }]
        result = bindings.reconcile(lua_report(), report, ['new_ent'])
        row = next(r for r in result['rows'] if r['name'] == 'gw_get_string')
        self.assertFalse(row['registered_at_scanned_original_CreateBind'])
        self.assertEqual(row['priority'],
                         'original_direct_call_has_published_vtable_not_bound')
        self.assertEqual(row['published_original_vtable']['address'], '0x006B70C0')
        self.assertEqual(row['published_original_vtable']['candidate_text_pointers'],
                         ['0x006175E8'])
        self.assertEqual(result['published_vtable_names'], 1)
        self.assertIn('NOT identified native Lua callback', result['critical_caveat'])

    def test_rejects_duplicate_published_vtable_identity(self):
        report = elf_report()
        record = {'name': 'gw_get_string', 'vtable_address': '0x006B70C0',
                  'available': False, 'slots': []}
        report['published_flag_vtable_slot_candidates'] = [record, record]
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            bindings.reconcile(lua_report(), report, [])

    def test_six_published_original_launcher_names_stay_distinct_from_elf_calls(self):
        report = elf_report()
        report['published_launcher_vtable_slot_candidates'] = [
            {'name': 'createFeatureLauncher', 'vtable_address': None,
             'published_symbol': 's_SetSelectFortune',
             'available': False, 'slots': []},
            {'name': 'createMGSGenerator', 'vtable_address': '0x006ADF00',
             'published_symbol': None, 'available': False, 'slots': []},
        ]
        result = bindings.reconcile(lua_report(), report, [])
        rows = {row['name']: row for row in result['rows']}
        self.assertFalse(rows['createFeatureLauncher']['registered_at_scanned_original_CreateBind'])
        self.assertIsNone(rows['createFeatureLauncher']['published_original_vtable']['address'])
        self.assertEqual(rows['createFeatureLauncher']['published_original_vtable']['published_symbol'],
                         's_SetSelectFortune')
        self.assertEqual(rows['createMGSGenerator']['published_original_vtable']['address'],
                         '0x006ADF00')

    def test_actor_and_controller_literals_form_original_stage_requirements(self):
        script = lua_report()
        script['scripts'] = [
            {'path': 'one/area.luc', 'literal_first_arguments': {
                'new_ent': ['Monkey', 'Player'], 'c_ctrl': ['trig_wait']},
             'direct_global_calls': ['new_ent', 'c_ctrl']},
            {'path': 'two/area.luc', 'literal_first_arguments': {
                'new_ent': ['Monkey']},
             'direct_global_calls': ['new_ent']},
        ]
        script['literal_first_arguments'] = {
            'new_ent': ['Monkey', 'Player'], 'c_ctrl': ['trig_wait'],
            'gw_get_flag': ['intro_completed'],
            'gw_set_flag': ['intro_completed', 'stage_complete']}
        report = bindings.reconcile(script, elf_report(), [])
        self.assertEqual(report['observed_actor_class_literals'], ['Monkey', 'Player'])
        self.assertEqual(report['observed_controller_command_literals'], ['trig_wait'])
        self.assertEqual(report['observed_global_flag_key_literals'],
                         ['intro_completed', 'stage_complete'])
        self.assertEqual(report['original_stage_inputs'][1]['path'], 'two/area.luc')
        self.assertEqual(report['original_stage_inputs'][1]['observed_actor_class_literals'],
                         ['Monkey'])

    def test_malformed_literal_evidence_cannot_be_silently_accepted(self):
        script = lua_report()
        script['literal_first_arguments'] = {'new_ent': [42]}
        with self.assertRaisesRegex(ValueError, 'literal argument'):
            bindings.reconcile(script, elf_report(), [])

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
