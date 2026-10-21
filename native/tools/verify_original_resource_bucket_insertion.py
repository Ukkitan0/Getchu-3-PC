#!/usr/bin/env python3
"""Audit owner-private Ape Escape 3 USA resource-bucket insertion and hash."""
import argparse,hashlib,struct,json
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
 (0x36cfd0,0x36d008):'2d2174a52e995ba42ec1ba5d2716d17fd79bf7417a08440f07eaef4e40f7d546',
 (0x36d610,0x36d6f4):'c3b70a57ebe9d7a204fae920bd629a1199c0ce9f2a7738dc78efa42488af7f67',
 (0x36d844,0x36d8c8):'65b245dea211fe48df41374ad3188052d3610e2077a25bc238203c2ea30c3a20'}
MMI={
 0x36cfd0:(8,26,14,4,14),0x36cfd4:(40,26,15,4,15),
 0x36cfd8:(8,8,14,15,15),0x36cfdc:(8,22,14,15,14),
 0x36cfe0:(40,22,15,15,15),0x36cfe4:(8,4,14,15,15),
 0x36cfe8:(8,18,14,15,14),0x36cfec:(40,18,15,15,15),
 0x36cff0:(8,0,14,15,14),0x36cff4:(41,14,14,0,15),
 0x36cff8:(8,0,15,14,15)}
ANCHORS={
 0x36d614:0x78af0010,0x36d62c:0x78ae0000,
 0x36d63c:0x0c0dbc7b,0x36d644:0x10400029,
 0x36d65c:0xac500020,0x36d660:0x7c4f0000,
 0x36d664:0xac400024,0x36d66c:0x7c4f0010,
 0x36d67c:0x700f24a9,0x36d688:0x0c0db3f4,
 0x36d690:0x7ba40050,0x36d694:0x0c0db3f4,
 0x36d69c:0x02028021,0x36d6a0:0x3210007f,
 0x36d6a4:0x00108080,0x36d6a8:0x02122021,
 0x36d6ac:0x8c8f0000,0x36d6b8:0x8dcf0024,
 0x36d6bc:0x51e00003,0x36d6c0:0xadd10024,
 0x36d6c4:0x1000fffc,0x36d6c8:0x8dce0024,
 0x36d6e8:0xac910000,0x36d004:0x304200ff}

# Source-fixed analysis of the two 16-byte reductions at 0x36CFD0.
# This is a private verification aid, not a game-time instruction interpreter.
def interleave(rs,rt,width,first):
 n=width//8
 def lanes(a):return [int.from_bytes(a[i:i+n],'little') for i in range(0,16,n)]
 a,b=lanes(rs),lanes(rt)
 return bytes().join(x.to_bytes(n,'little') for i in range(first,first+8//n)
                      for x in (b[i],a[i]))
def packed_add(a,b,width):
 n=width//8;mask=(1<<width)-1
 return bytes().join(((int.from_bytes(a[i:i+n],'little')+
                         int.from_bytes(b[i:i+n],'little'))&mask).to_bytes(n,'little')
                      for i in range(0,16,n))
def fold(t6,t7,a0):
 t6=interleave(t6,a0,8,0)             # PEXTLB
 t7=interleave(t7,a0,8,8)             # PEXTUB
 t7=packed_add(t6,t7,8)               # PADDB
 t6=interleave(t6,t7,16,0)           # PEXTLH
 t7=interleave(t7,t7,16,4)           # PEXTUH
 t7=packed_add(t6,t7,16)              # PADDH
 t6=interleave(t6,t7,32,0)           # PEXTLW
 t7=interleave(t7,t7,32,2)           # PEXTUW
 t6=packed_add(t6,t7,32)              # PADDW
 t7=t6[8:16]+bytes(8)                 # PCPYUD
 t7=packed_add(t7,t6,32)              # PADDW
 return t6,t7,t7[0]                   # ANDI v0,255
def original_hash(source):
 assert len(source)==32
 t6,t7,h1=fold(source[:16],source[16:],source[16:])
 t6,t7,h2=fold(t6,t7,source[:16])
 return (h1+h2)&127,h1,h2
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong original USA ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original ELF32 MIPS LE')
 ph=struct.unpack_from('<I',b,28)[0];stride,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(va,n):
  for lo,hi,off in seg:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError('Unmapped retail VA '+hex(va))
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (lo,hi),digest in WINDOWS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError('Original instruction window differs '+hex(lo))
 for va,ins in ANCHORS.items():
  if word(va)!=ins:raise ValueError('Source opcode mismatch '+hex(va))
 for va,(fn,sub,rs,rt,rd) in MMI.items():
  w=word(va)
  if (w>>26,w&63,(w>>6)&31,(w>>21)&31,(w>>16)&31,(w>>11)&31)!=(28,fn,sub,rs,rt,rd):
   raise ValueError('Source MMI decode mismatch '+hex(va))
 for va in (0x36d688,0x36d694):
  w=word(va)
  assert w>>26==3 and ((w&0x3ffffff)<<2)==0x36cfd0
 state=0x96edc3ab
 vectors=[bytes(32),bytes([255]*32),bytes(range(32)),bytes(range(32,64))]
 for _ in range(10000):
  arr=bytearray(32)
  for i in range(32):
   state=(state*1664525+1013904223)&0xffffffff
   arr[i]=state>>24
  vectors.append(bytes(arr))
 for arr in vectors:
  bucket,h1,h2=original_hash(arr)
  if h1!=(sum(arr[16:])&255) or h2!=(sum(arr[:16])&255):
   raise ValueError('Original per-half byte fold mismatch')
  if bucket!=(sum(arr)&127):raise ValueError('Closed-form bucket mismatch')
 return dict(original_elf_sha256=SHA,complete_regions=len(WINDOWS),
             whole_instruction_words=sum((hi-lo)//4 for lo,hi in WINDOWS),
             exact_opcode_anchors=len(ANCHORS),MMI_instructions=len(MMI),
             keys_verified=len(vectors),source_key_bytes=32,source_buckets=128,
             key_hash='sum(bytes[0:32]) & 127',
             original_node_payload_offset=32,original_node_next_offset=36)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('elf',type=Path)
 result=verify(p.parse_args().elf)
 print('PASS: original resource bucket insertion and closed-form 32-byte hash')
 print(json.dumps(result,indent=2))
