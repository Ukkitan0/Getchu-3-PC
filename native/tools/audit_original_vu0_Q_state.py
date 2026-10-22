#!/usr/bin/env python3
"""Source-audit original USA VU0 Q writers, readers and fallback VMULq."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
FALLBACK=(0x57cb9c,0x57cbb8,0x57cc94,0x57ccb0)
def scan(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Original US SCUS_975.01 SHA mismatch')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+size<=len(b):segments.append((va,va+size,off))
 def word(a):
  for lo,hi,off in segments:
   if lo<=a and a+4<=hi:return struct.unpack_from('<I',b,off+a-lo)[0]
  raise ValueError('Unmapped original VA '+hex(a))
 for addr in FALLBACK:
  w=word(addr)
  if w>>26!=18 or ((w>>21)&31)<16 or (w&63)!=0x1c:
   raise ValueError('Source VMULq not where claimed '+hex(addr))
 writes=[];reads=[];vdiv=[];vsqrt=[];vmulq=[]
 lo=0x100000;size=0x528258
 for a in range(lo,lo+size,4):
  w=word(a)
  if w>>26!=18:continue
  rs=(w>>21)&31;rd=(w>>11)&31;fun=w&63
  if rs==6 and rd==22:writes.append(a)
  if rs==2 and rd==22:reads.append(a)
  if rs<16:continue
  if fun==0x1c:vmulq.append(a)
  if fun>=60:
   sp=(w&3)|((w>>4)&0x7c)
   if sp==0x34:vdiv.append(a)
   if sp==0x35:vsqrt.append(a)
 if writes!=[0x41db9c,0x41dbb0] or reads!=[0x41dbbc]:
  raise ValueError('Original COP2 Q writer/reader inventory changed')
 if vdiv!=[0x3ba4f0] or vsqrt!=[0x3ba3dc] or len(vmulq)!=781:
  raise ValueError('Original VU Q arithmetic inventory changed')
 if any(0x57ca68<=a<0x57cd1c for a in writes+vdiv+vsqrt):
  raise ValueError('Fallback unexpectedly initializes Q itself')
 return {'source_USA_SHA256':SHA,'retail_text_words_scanned':size//4,
  'original_VMULq_sites':len(vmulq),
  'fallback_VMULq_source_addresses':[hex(x) for x in FALLBACK],
  'source_CTC2_Q_write_addresses':[hex(x) for x in writes],
  'source_CFC2_Q_read_addresses':[hex(x) for x in reads],
  'source_VDIV_address':[hex(x) for x in vdiv],
  'source_VSQRT_address':[hex(x) for x in vsqrt],
  'fallback_needs_caller_owned_Q':True,
  'original_Windows_game_booted':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_USA_elf',type=Path);a=p.parse_args()
 print(json.dumps(scan(a.original_USA_elf),indent=2))
