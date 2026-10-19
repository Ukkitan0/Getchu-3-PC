"""Independent code-window ELF and MIPS opcode emission checks (no game data)."""
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

SOURCE=Path(__file__).with_name('synthetic_overlay_smoke.py')
spec=importlib.util.spec_from_file_location('synthetic_overlay_smoke',SOURCE)
o=importlib.util.module_from_spec(spec)
spec.loader.exec_module(o)

class CodeWindowFixture(unittest.TestCase):
    def test_two_executable_load_segments_with_empty_runtime_window(self):
        b=o.build_bytes()
        o.inspect(b)
        self.assertEqual(b[:7],b'\x7fELF\x01\x01\x01')
        self.assertEqual(struct.unpack_from('<H',b,44)[0],2)
        ph0=struct.unpack_from('<8I',b,52)
        ph1=struct.unpack_from('<8I',b,84)
        self.assertEqual(ph0[:3],(1,o.OFFSET,o.ENTRY))
        self.assertEqual(ph1[:4],(1,0,o.WINDOW,o.WINDOW))
        self.assertEqual(ph1[4:7],(0,0x4000,5))

    def test_guest_writes_exact_callback_and_indirectly_executes(self):
        b=o.build_bytes()
        words=struct.unpack_from('<'+'I'*len(o.WORDS),b,o.OFFSET)
        self.assertEqual(words[0:4],(0x3C080011,0x3C092402,0x3529008F,0xAD090000))
        self.assertEqual(words[4:9],(0x3C0903E0,0x35290008,0xAD090004,0x0100F809,0))
        self.assertEqual(words[9:14],(0x3C0A0001,0x354A2000,0xAD420000,0x24030004,12))
        self.assertEqual((0x2402<<16)|0x8f,0x2402008f)
        self.assertEqual((0x03e0<<16)|8,0x03e00008)
        self.assertEqual(o.TARGET,0x12000)

    def test_fixture_mutation_detected(self):
        b=bytearray(o.build_bytes())
        b[-3]^=1
        with self.assertRaisesRegex(ValueError,'mismatch'):
            o.inspect(bytes(b))

if __name__=='__main__':
    unittest.main()
