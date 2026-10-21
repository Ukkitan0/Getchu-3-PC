#!/usr/bin/env python3
"""Independently audit private USA AE3 ELF source call and original woods_c EXDB PCK.

No copyrighted retail data is emitted or checked into the repository.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
EXDB_PCK_SHA='185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615'
WINDOWS={
 (0x35fcb0,0x35fd80):'3c8eee3f1ce66ad50443f7cb5a005ed610434b4925502f952a193ef0047c682e',
 (0x26d120,0x26d138):'fc2d9b311b0daa5714080c19c0aab726f214600f658e5f27b8a58da8cbb7cd5d',
 (0x26d214,0x26d228):'956f1eca03b4ef6a1d730382d58b220f42aa1fa80790f352d21524991378bc43',
 (0x1bb63c,0x1bb704):'9831cd65e4ed078527915f67ba86e7bfb865fcdf5e867d3f8ff2be25eba60bcd',
 (0x2bb03c,0x2bb084):'e4e87983feecf0691f5cc5ba6a953b72aaf0939f71719c03b94c1bd05d370594'
}
ANCHORS={
 0x35fcbc:0x0c09b7ce,0x35fcc4:0x0c09b448,
 0x35fd10:0x0c09b485,0x35fd18:0x8c440000,
 0x35fd20:0x0c10b113,0x35fd24:0x24a540a8,
 0x35fd28:0x14400011,0x35fd3c:0x8fa50014,
 0x35fd54:0x0c09b485,0x35fd5c:0x8e050008,
 0x35fd64:0x0c06ed8f,0x35fd68:0x8c46000c,
 0x26d120:0x8c8f0000,0x26d124:0x8dee0000,
 0x26d128:0x8de20004,0x26d134:0x00021103,
 0x26d214:0x8c8f0000,0x26d218:0x00052900,
 0x26d21c:0x8de20000,0x26d224:0x00451021,
 0x1bb664:0x12600027,0x1bb66c:0x0c0aec0f,
 0x1bb674:0x0c106e00,0x1bb6a4:0x240f00b0,
 0x2ba6a8:0x8d840008, # Header count is field 2, NOT field 0.
}
CALLS={
 0x35fcbc:0x26df38,0x35fcc4:0x26d120,
 0x35fd10:0x26d214,0x35fd20:0x42c44c,
 0x35fd44:0x26d214,0x35fd54:0x26d214,
 0x35fd64:0x1bb63c,0x1bb66c:0x2bb03c
}

def check_elf(path):
 b=path.read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Wrong original USA ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original MIPS little-endian ELF32')
 p=struct.unpack_from('<I',b,28)[0];stride,n=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(n):
  typ,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,p+i*stride)
  if typ==1 and off+sz<=len(b):seg.append((va,va+sz,off))
 def read(va,n):
  for lo,hi,off in seg:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError(f'Unmapped retail VA {va:#x}')
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (lo,hi),digest in WINDOWS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError('Original entire source function differs '+hex(lo))
 for va,original in ANCHORS.items():
  if word(va)!=original:raise ValueError('Source instruction differs '+hex(va))
 for va,target in CALLS.items():
  ins=word(va)
  if ins>>26!=3 or (((va+4)&0xf0000000)|((ins&0x03ffffff)<<2))!=target:
   raise ValueError('Retail JAL target mismatch '+hex(va))
 if read(0x7140a0,5)!=b'exdb\0' or read(0x7140a8,14)!=b'bgm_desc.exdb\0':
  raise ValueError('Retail audio source type/name literals differ')
 return {'retail_ELF_instructions':sum((hi-lo)//4 for lo,hi in WINDOWS),
         'verified_source_opcodes':len(ANCHORS),
         'verified_JALs':len(CALLS),
         'source_extension':'exdb','original_bgm_entry':'bgm_desc.exdb',
         'original_16_byte_resource_entry_offsets':{'name':0,'payload':8,'length':12}}

def check_pck(path):
 b=path.read_bytes()
 if hashlib.sha256(b).hexdigest()!=EXDB_PCK_SHA:
  raise ValueError('Wrong original owner-provided woods_c/exdb.pck')
 if b[:4]!=b'PCK\x00':raise ValueError('Original PCK header differs')
 table,count=struct.unpack_from('<II',b,4)
 if (table,count,len(b))!=(0x40,2,5728):raise ValueError('Original PCK structure differs')
 rows=[struct.unpack_from('<4I',b,table+i*16) for i in range(count)]
 def string(pos):return b[pos:b.index(b'\x00',pos)].decode('ascii')
 source=[]
 for name_off,typ_off,offset,size in rows:
  if offset+size>len(b):raise ValueError('Original PCK truncated')
  item=b[offset:offset+size]
  source.append({'name':string(name_off),'type':string(typ_off),'offset':offset,'size':size,
                 'record_marker':item[:4].decode('ascii')})
 if source!=[{'name':'monkeyprofile.exdb','type':'exdb','offset':0x60,'size':0x1400,'record_marker':'EDB\n'},
             {'name':'MainCameraParam.exdb','type':'exdb','offset':0x1460,'size':0x200,'record_marker':'EDB\n'}]:
  raise ValueError('Original PCK entry offsets/names differ')
 camera=b[0x1460:0x1660]
 if b'stn:MainCameraParam:18:1\n' not in camera or b'sizest:160\nb:352\n' not in camera:
  raise ValueError('Original camera EXDB source structure differs')
 if camera[352:368]!=b'camparam_default':raise ValueError('Original camera name differs')
 if camera[384:391]!=b'woods_c':raise ValueError('Original camera area differs')
 if struct.unpack_from('<f',camera,352+116)[0]!=30.:
  raise ValueError('Original camera parameter numeric value differs')
 return {'pck_sha256':EXDB_PCK_SHA,'real_entries':source,
         'camera_child_fields':18,'camera_data_offset':352,
         'camera_record_stride':160,'camera_records':1,
         'monkey_child_fields':59,'monkey_records':4,
         'monkey_record_stride':1024,'monkey_record_start_offset':1024}
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('original_elf',type=Path)
 parser.add_argument('private_woods_c_exdb_pck',type=Path)
 args=parser.parse_args()
 print('PASS: authentic retail EXDB PCK source + ELF loader dispatch + header-token correction')
 print(json.dumps({'source':check_elf(args.original_elf),'real_asset':check_pck(args.private_woods_c_exdb_pck)},indent=2))
