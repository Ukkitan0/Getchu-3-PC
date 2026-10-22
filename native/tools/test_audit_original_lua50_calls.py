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


def function(reads=(), writes=(), nested=()):
    names = list(dict.fromkeys([*reads, *writes]))
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

    def test_private_report_separates_known_bindings_and_script_definitions(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'stage').mkdir()
            (root / 'stage' / 'area.luc').write_bytes(chunk(
                function(['new_ent', 'needs_retail_binding'], ['setupArea'])))
            (root / 'runtime.cpp').write_text('registerCallback("new_ent", &handler);')
            result = audit.audit(root / 'stage', root / 'runtime.cpp')
            self.assertEqual(result['unresolved_global_reads'],
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
