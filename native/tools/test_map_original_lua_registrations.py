"""Synthetic MIPS call-site tests only. Never bundle the original executable."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location(
    'regscan', Path(__file__).with_name('map_original_lua_registrations.py'))
r = importlib.util.module_from_spec(spec)
spec.loader.exec_module(r)

BASE = 0x200000
LITERALS = 0x600000

def lui(reg, hi): return (15 << 26) | (reg << 16) | hi
def addiu(dst, src, value): return (9 << 26) | (src << 21) | (dst << 16) | (value & 0xFFFF)
def ori(dst, src, value): return (13 << 26) | (src << 21) | (dst << 16) | (value & 0xFFFF)
def jal(addr): return (3 << 26) | (addr >> 2 & 0x3FFFFFF)
def move(dst, src): return (src << 21) | (dst << 11) | 33

def fixture(words, name='gw_get_flag', base=BASE, ptr=LITERALS + 4):
    instructions = struct.pack('<' + 'I' * len(words), *words)
    literals = b'\x00' * 4 + name.encode('ascii') + b'\0'
    blob = instructions + literals
    segments = [(base, 0, len(instructions)),
                (LITERALS, len(instructions), len(literals))]
    return blob, segments

def scan(words, **opts):
    blob, segments = fixture(words, **opts)
    return r.scan_calls(blob, segments, BASE, len(words) * 4)

class RegistrationScanTests(unittest.TestCase):
    def test_delay_slot_signed_addiu(self):
        ptr = LITERALS + 4
        words = [lui(5, (ptr + 0x8000) >> 16),
                 jal(r.REGISTRY_HELPER), addiu(5, 5, ptr & 0xFFFF)]
        found = scan(words)
        self.assertEqual(len(found), 1)
        self.assertEqual(found[0]['name'], 'gw_get_flag')
        self.assertEqual(found[0]['name_producer_pc'], f'0x{BASE:08X}')

    def test_register_copy_and_ori(self):
        ptr = LITERALS + 4
        words = [lui(8, ptr >> 16), ori(8, 8, ptr & 0xFFFF),
                 move(5, 8), jal(r.REGISTRY_HELPER), 0]
        self.assertEqual(len(scan(words)), 1)

    def test_caller_saved_clobbered(self):
        ptr = LITERALS + 4
        words = [lui(5, ptr >> 16), ori(5, 5, ptr & 0xFFFF),
                 jal(0x340000), 0, jal(r.REGISTRY_HELPER), 0]
        self.assertEqual(scan(words), [])

    def test_branch_boundary_drops_uncertain_pointer(self):
        ptr = LITERALS + 4
        words = [lui(5, ptr >> 16), ori(5, 5, ptr & 0xFFFF),
                 (4 << 26), 0, jal(r.REGISTRY_HELPER), 0]
        self.assertEqual(scan(words), [])

    def test_unknown_helper_or_truncated_literal(self):
        ptr = LITERALS + 4
        words = [lui(5, ptr >> 16), ori(5, 5, ptr & 0xFFFF),
                 jal(r.REGISTRY_HELPER + 4), 0]
        self.assertEqual(scan(words), [])
        blob, segs = fixture(
            [lui(5, ptr >> 16), jal(r.REGISTRY_HELPER),
             ori(5, 5, ptr & 0xFFFF)], name='x' * 110)
        self.assertEqual(r.scan_calls(blob, segs, BASE, 12), [])

    def test_no_va_overread_across_segment_end(self):
        ptr = LITERALS + 4
        blob, segs = fixture(
            [lui(5, ptr >> 16), jal(r.REGISTRY_HELPER),
             addiu(5, 5, ptr & 0xFFFF)], name='X')
        self.assertEqual(len(r.scan_calls(blob, segs, BASE, 12)), 1)

    def test_published_flag_table_slots_are_raw_source_pointers_only(self):
        selected = r.FLAG_VTABLES['gw_get_string']
        pointer_values = (0x6175E8, 0, 0x100004, 0x700000, 0, 0)
        blob = struct.pack('<6I', *pointer_values)
        tables = r.flag_vtable_candidates(blob, [(selected, 0, len(blob))])
        actual = next(x for x in tables if x['name'] == 'gw_get_string')
        self.assertTrue(actual['available'])
        self.assertEqual(actual['slots'][0]['pointer'], '0x006175E8')
        self.assertTrue(actual['slots'][0]['points_into_original_text'])
        self.assertFalse(actual['slots'][3]['points_into_original_text'])
        self.assertFalse(next(x for x in tables if x['name'] == 'gw_set_flag')['available'])

    def test_clobbering_immediate_ops_cannot_reuse_prior_lui(self):
        ptr = LITERALS + 4
        for opcode in (10, 11, 12, 14, 24, 25, 32, 35, 36, 37):
            with self.subTest(opcode=opcode):
                words = [lui(5, ptr >> 16), ori(5, 5, ptr & 0xffff),
                         (opcode << 26) | (5 << 21) | (5 << 16),
                         jal(r.REGISTRY_HELPER), 0]
                self.assertEqual(scan(words), [], f'false registration after opcode {opcode}')

    def test_clobbering_special_and_coprocessor_ops(self):
        ptr = LITERALS + 4
        for word in ((5 << 11) | 32, (16 << 26), (28 << 26), (31 << 26)):
            with self.subTest(word=word):
                self.assertEqual(scan([lui(5, ptr >> 16),
                                       ori(5, 5, ptr & 0xffff), word,
                                       jal(r.REGISTRY_HELPER), 0]), [])

    def test_noncanonical_lui_cannot_preserve_an_old_pointer(self):
        ptr = LITERALS + 4
        words = [lui(5, ptr >> 16), ori(5, 5, ptr & 0xffff),
                 (15 << 26) | (3 << 21) | (5 << 16),
                 jal(r.REGISTRY_HELPER), 0]
        self.assertEqual(scan(words), [])

    def test_strict_private_elf_hash(self):
        with self.assertRaisesRegex(ValueError, 'only verified USA'):
            r.inspect(b'not-the-game')

    def test_invalid_elf_layout(self):
        with self.assertRaises(ValueError):
            r.segments_from_elf(b'wrong')
        fake = bytearray(52)
        fake[:6] = b'\x7fELF\x01\x01'
        struct.pack_into('<H', fake, 18, 8)
        struct.pack_into('<I', fake, 28, 1024)
        struct.pack_into('<HH', fake, 42, 32, 1)
        with self.assertRaises(ValueError):
            r.segments_from_elf(bytes(fake))

if __name__ == '__main__':
    unittest.main()
