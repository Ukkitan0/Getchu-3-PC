#!/usr/bin/env python3
import unittest
from decode_original_navrange_vu import decode_word
class RetailVUDecoderTests(unittest.TestCase):
 def test_original_arithmetic_and_lanes(self):
  expected={0x359ea4:(0x4be10868,"VADD","xyzw"),
    0x359ea8:(0x4b4200ac,"VSUB","xz"),
    0x359eb0:(0x4be00183,"VADDw","xyzw"),
    0x359eb4:(0x4ac300ec,"VSUB","yz"),
    0x359eb8:(0x4be111bd,"VMULAy","xyzw"),
    0x359ec0:(0x4b0030bf,"VMADDAw","x"),
    0x359ec4:(0x4be1194e,"VMSUBz","xyzw"),
    0x359ec8:(0x4b84012c,"VSUB","xy")}
  for va,(word,op,mask) in expected.items():
   with self.subTest(address=hex(va)):
    actual=decode_word(va,word)
    self.assertEqual((actual["op"],actual["mask"]),(op,mask))
  r=decode_word(0x359eb8,0x4be111bd)
  self.assertEqual((r["fs"],r["ft"]),(2,1))
 def test_other_instructions_are_not_guessed(self):
  self.assertEqual(decode_word(0x441f60,0x55cffffa)["family"],"other")
  self.assertEqual(decode_word(0x359e94,0x71ce7ee8)["family"],"other")
if __name__=="__main__":unittest.main()
