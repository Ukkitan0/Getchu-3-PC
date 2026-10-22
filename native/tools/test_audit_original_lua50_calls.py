"""Original Lua 5.0 bytecode dependency audit tests (synthetic bytes only)."""
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

MODULE = Path(__file__).with_name('audit_original_lua50_calls.py')
spec = importlib.util.spec_from_file_location('audit_original_lua50_calls', MODULE)
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def u32(v):
    return struct.pack('<I', v)


def string(value):
    raw = value.encode('ascii') + b'\0'
    return u32(len(raw)) + raw


def function(reads=(), writes=(), nested=(), calls=(), alias_calls=(), barrier_calls=(),
             literal_calls=(), literal_alias_calls=(), overwritten_calls=(),
             skipped_calls=(), no_arg_calls=()):
    names = list(dict.fromkeys([*reads, *writes, *calls, *alias_calls, *barrier_calls,
        *(x for pair in (*literal_calls, *literal_alias_calls, *overwritten_calls,
                         *skipped_calls, *no_arg_calls) for x in pair)]))
    result = bytearray()
    result += string('synthetic_unit_test') + u32(1) + b'\0\0\0\x02'
    result += u32(0) + u32(0) + u32(0)
    result += u32(len(names))
    for name in names:
        result += b'\x04' + string(name)
    result += u32(len(nested))
    for child in nested:
        result += child
    ins = [((names.index(name) << 6) | 5) for name in reads]
    ins += [((names.index(name) << 6) | 7) for name in writes]
    # Lua 5.0 OP_GETGLOBAL/OP_CALL use A at bit 24, Bx at bit 6.
    for name in calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                (5 << 24) | (names.index(name) << 6) | 1,
                (4 << 24) | (1 << 6) | 25]
    for name in alias_calls:
        ins += [(3 << 24) | (names.index(name) << 6) | 5,
                (5 << 24) | (3 << 15) | 0,
                (5 << 24) | (1 << 6) | 25]
    for name in barrier_calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                20,  # OP_JMP: do not invent a direct call across flow joins
                (4 << 24) | (1 << 6) | 25]
    for name, literal in literal_calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                (5 << 24) | (names.index(literal) << 6) | 1,
                (4 << 24) | (3 << 15) | (1 << 6) | 25]
    for name, literal in literal_alias_calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                (6 << 24) | (names.index(literal) << 6) | 1,
                (5 << 24) | (6 << 15) | 0,
                (4 << 24) | (2 << 15) | (1 << 6) | 25]
    for name, literal in overwritten_calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                (5 << 24) | (names.index(literal) << 6) | 1,
                (5 << 24) | 2, # LOADBOOL overwrites first argument register
                (4 << 24) | (2 << 15) | (1 << 6) | 25]
    for name, literal in skipped_calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                (5 << 24) | (names.index(literal) << 6) | 1,
                20, # branch: no provenance beyond this point
                (4 << 24) | (2 << 15) | (1 << 6) | 25]
    for name, literal in no_arg_calls:
        ins += [(4 << 24) | (names.index(name) << 6) | 5,
                (5 << 24) | (names.index(literal) << 6) | 1,
                (4 << 24) | (1 << 15) | (1 << 6) | 25]
    result += u32(len(ins))
    for op in ins:
        result += u32(op)
    return bytes(result)


def chunk(body):
    return audit.HEADER + struct.pack('<f', 31415926.0) + body


class RetailLua50AuditTests(unittest.TestCase):
    def test_real_lua50_instruction_format_and_nested_prototypes(self):
        blob = chunk(function(['new_ent', 'gw_set_flag'],
                              ['setupArea'], [function(['get_capture_flag'])]))
        result = audit.inspect(blob)
        self.assertEqual(result['prototypes'], 2)
        self.assertEqual(result['instructions'], 4)
        self.assertEqual(result['global_reads'],
                         ['get_capture_flag', 'gw_set_flag', 'new_ent'])
        self.assertEqual(result['global_writes'], ['setupArea'])

    def test_lua50_call_provenance_and_branch_safety(self):
        blob = chunk(function(['new_ent'], ['setupArea'],
                              calls=['new_ent'], alias_calls=['lua_param'],
                              barrier_calls=['branch_only']))
        parsed = audit.inspect(blob)
        self.assertEqual(parsed['direct_global_calls'], ['lua_param', 'new_ent'])
        self.assertEqual(parsed['prototype_calls'][0]['line_defined'], 1)
        self.assertIn('branch_only', parsed['global_reads'])
        self.assertNotIn('branch_only', parsed['direct_global_calls'])

    def test_actual_lua50_abx_layout_uses_bits_6_to_23(self):
        # The previous auditor incorrectly decoded Lua 5.1's bit-14 Bx.
        # A nonzero register A in bits 24..31 must not contaminate Bx.
        body = function(['first', 'second', 'third'])
        blob = chunk(body)
        parsed = audit.inspect(blob)
        self.assertEqual(parsed['global_reads'], ['first', 'second', 'third'])
        self.assertEqual(parsed['instructions'], 3)

    def test_closure_opcode_35_with_nested_stage_callback(self):
        # Original Lua 5.0.3 defines OP_CLOSURE=35. Stage scripts define
        # setupArea as a nested function, so this opcode is not optional.
        nested = function(['new_ent'], [], calls=['new_ent'])
        original = function([], ['setupArea'], nested=[nested])
        source = chunk(original)
        # Replace first function's final SETGLOBAL with CLOSURE R(0), proto 0
        # and append SETGLOBAL to preserve both emitted instructions.
        instruction = u32(35) + u32((0 << 24) | (0 << 6) | 7)
        orig_inst = u32(7)
        self.assertTrue(source.endswith(orig_inst))
        modified = source[:-8] + u32(2) + instruction
        report = audit.inspect(modified)
        self.assertEqual(report['prototypes'], 2)
        self.assertEqual(report['direct_global_calls'], ['new_ent'])
        self.assertEqual(report['global_writes'], ['setupArea'])
        invalid = modified[:-8] + u32(63) + modified[-4:]
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(invalid)

    def test_literal_first_arguments_recover_original_actor_class_and_controller(self):
        src = chunk(function(literal_calls=[('new_ent', 'Monkey'),
                                            ('c_ctrl', 'trig_wait')],
                             literal_alias_calls=[('set_player_weapon', 'net')]))
        report = audit.inspect(src)
        self.assertEqual(report['literal_first_arguments'], {
            'c_ctrl': ['trig_wait'], 'new_ent': ['Monkey'],
            'set_player_weapon': ['net']})
        by_name = {site['name']: site for site in report['direct_call_sites']}
        self.assertEqual(by_name['new_ent']['literal_first_string_argument'], 'Monkey')
        self.assertEqual(by_name['new_ent']['line_defined'], 1)
        self.assertEqual(by_name['set_player_weapon']['pc'], 9)

    def test_no_fabricated_argument_after_clobber_branch_or_no_arguments(self):
        report = audit.inspect(chunk(function(
            overwritten_calls=[('new_ent', 'OldName')],
            skipped_calls=[('gw_set_flag', 'branch_symbol')],
            no_arg_calls=[('lua_param', 'ghost_arg')])))
        self.assertEqual(report['literal_first_arguments'], {})
        sites = report['direct_call_sites']
        self.assertEqual(len(sites), 2)
        self.assertTrue(all(site['literal_first_string_argument'] is None for site in sites))

    def test_private_report_aggregates_proven_literal_values_without_scripts(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'area.luc').write_bytes(chunk(function(
                literal_calls=[('new_ent', 'Monkey'), ('new_ent', 'Player')])))
            result = audit.audit(root/'area.luc')
            self.assertEqual(result['literal_first_arguments'],
                             {'new_ent': ['Monkey', 'Player']})
            self.assertIn('direct_call_sites', result['scripts'][0])

    def test_private_report_separates_known_bindings_and_script_definitions(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'stage').mkdir()
            (root / 'stage' / 'area.luc').write_bytes(chunk(
                function(['new_ent', 'needs_retail_binding'], ['setupArea'],
                         calls=['needs_retail_binding'])))
            (root / 'runtime.cpp').write_text('registerCallback("new_ent", &handler);')
            result = audit.audit(root / 'stage', root / 'runtime.cpp')
            self.assertEqual(result['unresolved_global_reads'],
                             ['needs_retail_binding'])
            self.assertEqual(result['unresolved_direct_global_calls'],
                             ['needs_retail_binding'])
            self.assertEqual(result['scripts'][0]['path'], 'area.luc')

    def test_corruption_and_opcode_index_rejected(self):
        body = function(['new_ent'])
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(chunk(body)[:-2])
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(chunk(body) + b'junk')
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(b'\x1bLua\x51' + chunk(body)[5:])
        invalid = bytearray(chunk(body))
        invalid[-4:] = u32((2 << 6) | 5)
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(bytes(invalid))

    def test_float_marker_and_string_validation(self):
        fake = bytearray(chunk(function(['x'])))
        fake[14:18] = struct.pack('<f', 1.0)
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(bytes(fake))
        fake = bytearray(chunk(function(['x'])))
        fake[18 + 4 + len('synthetic_unit_test')] = ord('!')
        with self.assertRaises(audit.InvalidChunk):
            audit.inspect(bytes(fake))


if __name__ == '__main__':
    unittest.main()
