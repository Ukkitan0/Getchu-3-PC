#!/usr/bin/env python3
"""Verify original AE3 USA fallback receives two distinct source vectors."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
ANCHORS={0x57c970:0x8e0f0004,0x57c974:0x7a060020,0x57c978:0x8def0014,
 0x57c97c:0x0c15f29a,0x57c980:0x79e50040,
 0x57ca78:0x7005bca9,0x57ca88:0x7006b4a9}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Original owner-USA ELF changed')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if kind==1 and off+size<=len(b):segments.append((va,va+size,off))
 def word(a):
  for lo,hi,off in segments:
   if lo<=a and a+4<=hi:return struct.unpack_from('<I',b,off+a-lo)[0]
  raise ValueError('Source virtual address unmapped '+hex(a))
 for a,w in ANCHORS.items():
  if word(a)!=w:raise ValueError('Original source argument opcode changed '+hex(a))
 first,second=word(0x57c974),word(0x57c980)
 if (first>>26,(first>>21)&31,(first>>16)&31,first&65535)!=(30,16,6,0x20):
  raise ValueError('Original $a2 is not controller+0x20')
 if (second>>26,(second>>21)&31,(second>>16)&31,second&65535)!=(30,15,5,0x40):
  raise ValueError('Original $a1 is not owner+0x40')
 own=word(0x57c978)
 if (own>>26,(own>>21)&31,(own>>16)&31,own&65535)!=(35,15,15,0x14):
  raise ValueError('Original owner pointer hop +0x14 changed')
 call=word(0x57c97c)
 if call>>26!=3 or (((0x57c980)&0xf0000000)|((call&0x3ffffff)<<2))!=0x57ca68:
  raise ValueError('Original JAL to 0x57CA68 changed')
 return {'original_ELF_SHA256':SHA,'source_words_verified':len(ANCHORS),
   'input_a1':'owner chain +0x04 -> +0x14 -> packed +0x40',
   'input_a2':'original controller packed +0x20',
   'source_inputs_distinct':True,
   'callee_MMI_upper_half_extractions':['0x57CA78 from a1','0x57CA88 from a2'],
   'external_VU0_Q_still_needed':True,'game_booted_on_Windows':False}
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('original_USA_ELF',type=Path)
 args=parser.parse_args()
 print(json.dumps(verify(args.original_USA_ELF),indent=2))
