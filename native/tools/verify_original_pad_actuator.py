#!/usr/bin/env python3
"""Verify original USA AE3 pad actuator control and initialization from owner ELF.

The script commits no source assets, sound banks, executable words or game data.
"""
import argparse
import hashlib
import struct
from pathlib import Path

SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
DELTA=0xff000
SECTIONS={
    (0x51790c,0x5179b0):'c2f91a6626d6ef6cf5c98e5f43b19e8809a3612c368d70dae991467daea50cd4',
    (0x517c1c,0x517d38):'a00d4406b5b9f5ffe4bfd88cfe407de7fa54103140a110c5922f9e1f08d4a58f',
}
WORDS={
  0x517910:0x922f0000, # source mode byte
  0x517924:0x46000832, # C.EQ.S saved vs requested intensity
  0x517940:0x8e2f0000, # requested mode
  0x51794c:0xae0f0014, # store previous mode +0x14
  0x517970:0x8e2f0004, # requested intensity
  0x517974:0xae0f0018, # store intensity +0x18
  0x51797c:0x46010002, # MUL.S
  0x517980:0x46000064, # CVT.W.S
  0x517984:0x440f0800, # MFC1
  0x517988:0x000f7040, # SLL 1
  0x51798c:0x000f79c3, # SRA 7
  0x517994:0xa3af0021, # high byte output
  0x517998:0x35cf0001, # low byte OR 1
  0x51799c:0x01ed700b, # MOVN on mode byte
  0x5179a4:0xa3ae0020, # low byte output in JAL delay slot
  0x517c20:0xa0800014, # constructor mode initialization
  0x517c40:0xae000018, # constructor intensity initialization
}

def verify(path:Path)->None:
 b=path.read_bytes()
 assert hashlib.sha256(b).hexdigest()==SHA,'ELF SHA mismatch'
 def word(va:int)->int:return struct.unpack_from('<I',b,va-DELTA)[0]
 for (a,z),digest in SECTIONS.items():
  assert hashlib.sha256(b[a-DELTA:z-DELTA]).hexdigest()==digest,f'source function bytes changed: {a:x}'
 assert struct.unpack_from('<f',b,0x6ed0c4-DELTA)[0]==255.0,'source float scale !=255'
 for va,expected in WORDS.items():
  assert word(va)==expected,f'source instruction changed @ {va:x}'
 call=word(0x5179a0)
 assert call>>26==3 and (call&0x3ffffff)<<2==0x405e30,'source pad actuator callback changed'
 print('ORIGINAL_RETAIL_ACTUATOR_ELF_AUDIT_PASSED',
       f'functions={len(SECTIONS)} instructions={len(WORDS)} source=0x51790C..0x5179A8')

if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('original_usa_elf',type=Path)
 verify(ap.parse_args().original_usa_elf)
