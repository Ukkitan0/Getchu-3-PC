#!/usr/bin/env python3
"""Verify retail Ape Escape 3 USA stage resource-registration evidence.

Reads only owner-private SCUS_975.01 and extracted woods_c source files.
Emits no executable, model, stage or Lua bytes.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import struct

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOWS={
 (0x24D858,0x24D9EC):"154dd26d320162b79f7aa75512afa0a6938b580754e5c473847cb2c18a6b3511",
 (0x25182C,0x251BC8):"cef6579182e855442ede0b3980a2d3c5c3680ce3f81d02250e8d0e05859d23de",
 (0x24D4F0,0x24D5FC):"7ec60322e5ae08a2e117a0b4c120837a4a7ac1d3c16fda83a2e6930afe6b20ad",
}
OPCODES={
 0x24D894:0xAC910000,  # name at record+0
 0x24D8BC:0xAE910004,  # type at record+4
 0x24D8CC:0xAE920008,  # pointer at record+8
 0x24D8D4:0xAE93000C,  # size at record+12
 0x24D8F8:0x144F0014,  # original first-dot condition
 0x2519D4:0x12C0000D,  # nullable filter
 0x2519D8:0x240F002A,  # '*'
 0x2519DC:0x82CE0000,  # filter first byte
 0x2519E0:0x11CF000A,  # wildcard branch
 0x251A3C:0x25EF0010,  # 16-byte append pointer increment
}
CALLS={
 0x24D8DC:0x627CF8, # find first '.' (NOT +0xD8)
 0x24D950:0x627038, # prefix substring
 0x24D9B0:0x627038, # suffix substring
 0x2519AC:0x24D858, # source record constructor
 0x2519FC:0x42C44C, # compare selector to record +4
 0x251A30:0x50ACD4, # append
 0x251A84:0x50A238, # first/expanded append
}
PRIVATE_FILES={
 "placeholder.plc":(6528,"e91dbf300b3dc9e410c3af4d50e6761f8ee78a64a3912bad64aac1eb956de9ae"),
 "navigation.pck":(3328,"d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0"),
 "actionseq.asq":(448,"cb4bbe475f2b2fd377838b5a32ef683dbf17576328ff8be2d663bc9593683edc"),
 "monkeydb.xml":(715,"b85ec96dcd1aa5fde7dfa496ecf045938a8133fe2dce1147443f711f58f802e8"),
 "area.luc":(4781,"85ac02cf8fb9bdf33b0f98b036e5fa66f37c4412146b1fa91b9920381269273e"),
 "bg.pck":(3428752,"e406ff6466f6a930b2f9c9ea8a0f926f3f9352170f2dd41e85d6390c7120352e"),
 "character.pck":(995872,"993c71929259083102c3cb643c4683edb0e6db757cb71106589c35b7745d61b3"),
 "exdb.pck":(5728,"185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615"),
 "apelodtex.pck":(9872,"3a6c06d2cf36ce9d3b5b28fd1cb29bff8a0826836a45000bef4ec15d5c1b1d00"),
}
def check(condition:bool,error:str)->None:
 if not condition:raise ValueError(error)
def digest(data:bytes)->str:return hashlib.sha256(data).hexdigest()
def elf_check(path:Path)->dict:
 data=path.read_bytes()
 check(digest(data)==ELF_SHA,"Wrong private original USA ELF")
 check(data[:7]==b"\x7fELF\x01\x01\x01","Expected original little-endian ELF32")
 phoff=struct.unpack_from("<I",data,28)[0]
 stride,n=struct.unpack_from("<HH",data,42)
 check(stride>=32 and phoff+stride*n<=len(data),"Original ELF program header truncated")
 segments=[]
 for i in range(n):
  kind,offset,va,_,size,_,_,_=struct.unpack_from("<8I",data,phoff+i*stride)
  if kind==1:
   check(offset+size<=len(data),"Original load segment truncated")
   segments.append((va,va+size,offset))
 def read(va:int,length:int)->bytes:
  for start,end,off in segments:
   if start<=va and va+length<=end:return data[off+va-start:off+va-start+length]
  raise ValueError(f"Original source VA {va:#x} is unmapped")
 def word(va:int)->int:return struct.unpack("<I",read(va,4))[0]
 for (a,b),expected in WINDOWS.items():
  check(digest(read(a,b-a))==expected,f"Retail original code window changed: {a:#x}")
 for va,expected in OPCODES.items():
  check(word(va)==expected,f"Original instruction mismatch at {va:#x}")
 for pc,target in CALLS.items():
  instruction=word(pc)
  decoded=((pc+4)&0xF0000000)|((instruction&0x03FFFFFF)<<2)
  check(instruction>>26==3 and decoded==target,f"Original JAL mismatch {pc:#x}")
 return {"source_sha256":ELF_SHA,"whole_source_windows":len(WINDOWS),
         "instruction_words":sum((b-a)//4 for a,b in WINDOWS),
         "independent_opcodes":len(OPCODES),"original_JAL_edges":len(CALLS),
         "record_constructor":"0x24d858","registration":"0x25182c",
         "named_lookup":"0x24d4f0",
         "source_record_offsets":{"name":0,"type_suffix":4,"data_pointer":8,"length":12}}
def source_check(stage:Path)->dict:
 check(stage.is_dir(),"Private woods_c original resource folder missing")
 names=[]
 for filename,(size,sha) in PRIVATE_FILES.items():
  raw=(stage/filename).read_bytes()
  check(len(raw)==size and digest(raw)==sha,
        f"Original USA woods_c file mismatch: {filename}")
  dot=filename.find(".")
  check(dot>0,f"Original source name has no extension: {filename}")
  names.append((filename[:dot],filename[dot+1:]))
 for name,typ in (("placeholder","plc"),("navigation","pck"),
                  ("actionseq","asq"),("monkeydb","xml")):
  check((name,typ) in names,"Original stage startup resource absent")
 return {"stage":"woods_c","SHA256_verified_source_files":len(names),
         "source_type_counts":{t:sum(typ==t for _,typ in names)
                               for t in sorted({typ for _,typ in names})},
         "stage_initialization_keys":["placeholder","navigation","actionseq","monkeydb"],
         "retail_game_bytes_committed":False}
def main()->None:
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument("private_usa_elf",type=Path)
 parser.add_argument("private_woods_c_folder",type=Path)
 args=parser.parse_args()
 print(json.dumps({"elf":elf_check(args.private_usa_elf),
                   "stage":source_check(args.private_woods_c_folder)},indent=2))
 print("ORIGINAL_STAGE_RECORD_ELF_RETAIL_VERIFICATION_PASSED")
if __name__=="__main__":main()
