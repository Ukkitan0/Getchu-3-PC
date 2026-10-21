#!/usr/bin/env python3
"""Table/operand sanity tests for the static R5900 MMI source decoder."""
import unittest
from audit_original_navrange_mmi_basis import decode_mmi
class DecodeTests(unittest.TestCase):
    def test_original_shuffles(self):
        cases={
          0x21effc:(0x06180000,'MTSAB',16),
          0x21f000:(0x700d7ee8,'QFSRV',0),
          0x21f008:(0x724f74c8,'PPACW',18),
          0x21f010:(0x726e74c8,'PPACW',19),
          0x21f0c0:(0x700074a9,'POR',0),
          0x21f10c:(0x71c074e9,'PNOR',14),
          0x21f110:(0x708e2489,'PAND',4),
          0x21f114:(0x708f24a9,'POR',4),
        }
        for addr,(opcode,name,rs) in cases.items():
            with self.subTest(addr=hex(addr)):
                row=decode_mmi(addr,opcode)
                self.assertIsNotNone(row)
                self.assertEqual(row['op'],name)
                self.assertEqual(row['rs'],rs)
    def test_no_misclassified_instructions(self):
        self.assertIsNone(decode_mmi(0x21f020,0x4be3086c))
        self.assertIsNone(decode_mmi(0x21f03c,0xfba20040))
if __name__=='__main__':unittest.main()
