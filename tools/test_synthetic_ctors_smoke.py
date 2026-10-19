"""Original test-code constructor table and MIPS control-flow fixture checks."""
from pathlib import Path
import importlib.util
import struct
import tempfile
import unittest

SRC = Path(__file__).with_name('synthetic_ctors_smoke.py')
spec = importlib.util.spec_from_file_location('synthetic_ctors_smoke', SRC)
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)

class ConstructorFixture(unittest.TestCase):
    def test_data_bounds_and_mips_elf(self):
        data = fixture.build_bytes()
        fixture.check(data)
        self.assertEqual(data[:4], b'\x7fELF')
        self.assertEqual(struct.unpack_from('<I', data, 24)[0], fixture.ENTRY)
        self.assertEqual(struct.unpack_from('<II', data, 52 + 16),
                         (len(data)-0x100, len(data)-0x100))
        self.assertEqual(struct.unpack_from('<I', data, fixture.FILE_OFFSET + fixture.POINTER_TABLE - fixture.ENTRY + fixture.COUNT*4)[0], 0)

    def test_143_distinct_indirect_entries(self):
        entries=fixture.callback_addresses()
        self.assertEqual(len(entries),143)
        self.assertEqual(len(set(entries)),143)
        self.assertTrue(all(x%4==0 for x in entries))
        self.assertTrue(all(x<fixture.POINTER_TABLE for x in entries))
        self.assertEqual(fixture.MAIN[6],0x0320F809)
        with tempfile.TemporaryDirectory() as t:
            file=Path(t)/'entries.txt'; fixture.entries_file(file)
            rows=file.read_text().splitlines()
            self.assertEqual(rows[0],'00100200')
            self.assertEqual(len(rows),143)

    def test_each_target_points_at_increment_and_return(self):
        data=fixture.build_bytes()
        for target in fixture.callback_addresses():
            at=fixture.FILE_OFFSET + target-fixture.ENTRY
            self.assertEqual(struct.unpack_from('<III',data,at),fixture.CALLBACK)

    def test_mutated_callback_refused(self):
        data=bytearray(fixture.build_bytes())
        data[0x100+fixture.CALLBACK_BASE-fixture.ENTRY]^=1
        with self.assertRaisesRegex(ValueError,'mismatch'):
            fixture.check(bytes(data))

    def test_branch_offsets(self):
        # at main instruction 4: (branch_pc+4)+(5*4) = done instruction 10
        self.assertEqual(4+1+(fixture.MAIN[4]&0xffff),10)
        # at main instruction 8: (branch_pc+4)-6*4 = loop instruction 3
        imm=fixture.MAIN[8]&0xffff
        self.assertEqual(8+1+(imm-0x10000),3)

if __name__=='__main__':
    unittest.main()