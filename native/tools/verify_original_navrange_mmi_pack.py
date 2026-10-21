#!/usr/bin/env python3
"""Audit authentic retail SCUS_975.01 MMI quaternion matrix against native C++ port."""
import argparse,hashlib,struct
from pathlib import Path
SOURCE_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOW_SHA="fb74790cea6a3d4a4f4658f5eee097f975a9073f2fe54b467d11e278d09a9d85"
START,STOP=0x359e74,0x359ef8
# Exact source instruction words pin every decoded op without distributing the ELF.
WORDS={
0x359e74:0,0x359e78:0,0x359e7c:0,0x359e80:0x04190004,
0x359e84:0x70a56ee8,0x359e88:0x700577a9,0x359e8c:0x71ce76e8,
0x359e90:0x700e77a9,0x359e94:0x71ce7ee8,0x359e98:0x48a50800,
0x359e9c:0x0080102d,0x359ea0:0x48ae1000,0x359ea4:0x4be10868,
0x359ea8:0x4b4200ac,0x359eac:0x48ad1800,0x359eb0:0x4be00183,
0x359eb4:0x4ac300ec,0x359eb8:0x4be111bd,0x359ebc:0x48af2000,
0x359ec0:0x4b0030bf,0x359ec4:0x4be1194e,0x359ec8:0x4b84012c,
0x359ecc:0xf8800030,0x359ed0:0xf8850000,
0x359ed4:0x4be121be,0x359ed8:0x4a8030bf,0x359edc:0x4be1108c,
0x359ee0:0x4be119bc,0x359ee4:0x4a4030bf,0x359ee8:0xf8820010,
0x359eec:0x4be1204d,0x359ef0:0x03e00008,0x359ef4:0xf8810020}
def verify(path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=SOURCE_SHA:
  raise ValueError("Wrong original USA retail executable")
 if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
  raise ValueError("Expected original MIPS ELF32")
 ph=struct.unpack_from("<I",data,28)[0];step,count=struct.unpack_from("<HH",data,42)
 window=None
 for i in range(count):
  typ,off,va,_,size,_,_,_=struct.unpack_from("<8I",data,ph+i*step)
  if typ==1 and va<=START and STOP<=va+size:
   window=data[off+START-va:off+STOP-va];break
 if window is None or hashlib.sha256(window).hexdigest()!=WINDOW_SHA:
  raise ValueError("Original 33-word matrix function region differs")
 def word(a):return struct.unpack_from("<I",window,a-START)[0]
 for addr,expected in WORDS.items():
  if word(addr)!=expected:raise ValueError("Unexpected original opcode "+hex(addr))
 mtsah=word(0x359e80)
 assert mtsah>>26==1 and (mtsah>>16)&31==25 and (mtsah>>21)&31==0 and (mtsah&65535)==4
 # MMI decode: sa 27=QFSRV, sa 30=PEXCW; compare ORIGINAL rd/rs/rt.
 for addr,sa,rd,rs,rt in (
    (0x359e84,27,13,5,5),(0x359e88,30,14,0,5),
    (0x359e8c,27,14,14,14),(0x359e90,30,14,0,14),
    (0x359e94,27,15,14,14)):
  w=word(addr)
  assert w>>26==0x1c and (w&63)==(0x28 if sa==27 else 0x29)
  assert ((w>>6)&31,(w>>11)&31,(w>>21)&31,(w>>16)&31)==(sa,rd,rs,rt)
 # Actual QMTC2 input register to original VU0 register mapping.
 for a,gpr,vf in ((0x359e98,5,1),(0x359ea0,14,2),(0x359eac,13,3),(0x359ebc,15,4)):
  w=word(a)
  assert w>>26==0x12 and (w>>21)&31==5 and ((w>>16)&31,(w>>11)&31)==(gpr,vf)
 # Final six source vector arithmetic opcodes, complete operand+lane masks.
 for a,fn,fd,fs,ft,mask,index in (
    (0x359ed4,62,6,4,1,15,26), (0x359ed8,63,2,6,0,4,11),
    (0x359edc,12,2,2,1,15,12),(0x359ee0,60,6,3,1,15,24),
    (0x359ee4,63,2,6,0,2,11),(0x359eec,13,1,4,1,15,13)):
  w=word(a)
  i=((w&3)|((w>>4)&0x7c)) if fn>=60 else fn
  assert w>>26==0x12 and (w&63)==fn and i==index
  assert ((w>>6)&31,(w>>11)&31,(w>>16)&31,(w>>21)&15)==(fd,fs,ft,mask)
 # Source SQC2 stores VF5/VF2/VF1/VF0 at +0/+16/+32/+48.
 for a,vf,offset in ((0x359ecc,0,48),(0x359ed0,5,0),
                     (0x359ee8,2,16),(0x359ef4,1,32)):
  w=word(a)
  assert w>>26==0x3e and (w>>21)&31==4 and ((w>>16)&31,w&65535)==(vf,offset)
 assert word(0x359ef0)==0x03e00008
 print("PASS: 33 source words, MTSAH, 5 MMI, 4 QMTC2, 6 VU and 4 SQC2 stores")
 print("PASS: original 0x359E74..0x359EF4 quaternion-to-matrix function verified")
 print("LIMIT: caller eligibility/source input provenance remains incomplete")
if __name__=="__main__":
 p=argparse.ArgumentParser();p.add_argument("elf",type=Path)
 verify(p.parse_args().elf)
