#!/usr/bin/env python3
"""Verify source-level native action recovery against original USA SCUS_975.01.

Only static instructions, addresses and counts are emitted. The retail ELF
remains owner-private. No PlayStation hardware is emulated.
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
ANCHORS="""
3168ac:8c8f0000 3168b0:03e00008 3168b4:91e20045
3168b8:8c8f0000 3168bc:03e00008 3168c0:8de20048
3168c4:8c8f0000 3168c8:03e00008 3168cc:ade5000c
3102fc:8c8f000c 310300:31ef0001 310308:11e0000f
310310:8c8f0040 310314:15e0000d 31031c:8c8f0034
310320:15e00008 310328:0c0c5a2b 31032c:8c840020
310330:10400005 310338:0c0c5bf4 310340:0002802b
310344:0200102d
310358:8c8f0034 31035c:11e00006 310364:8c8f0030
310368:11e00003 310370:8c8f0040 310374:2de20001
310380:8c8f0000 310388:8de20070
3103c0:8c8f0038 3103c4:10af0005 3103cc:8c840000
3103d0:0c158e3d 3103d4:8e050014 3103d8:ae110038
3103f8:8c840000 310400:248400d6 310404:08059488
316fec:0c15ee80 316ff4:8e0e0008 317000:15c00006
317008:0c15ee80 317010:8e0f0008 317014:11e00003
31701c:24110001
57ba0c:908f0005 57ba10:15e0001d 57ba18:240f0001
57ba1c:ac800008 57ba20:908e0004 57ba24:15c00018
57ba28:a08f0005 57ba2c:0c088591 57ba44:0040f809
57ba54:1040000c 57ba60:8de2002c 57ba64:0040f809
57ba6c:304200ff 57ba70:10400005 57ba7c:0c087626
57ba84:ae020008
276278:3c0f0063 276280:91ef54f7 276284:15e00004
27628c:8c84001c 276290:0809acc4
2762c4:8c84001c 2762cc:0809ad49
276588:78840050 276590:080d6792
"""
EDGES=[(0x310328,0x3168ac),(0x310338,0x316fd0),
(0x316fec,0x57ba00),(0x317008,0x57ba00),(0x57ba2c,0x221644),
(0x57ba7c,0x21d898),(0x3103d0,0x5638f4),(0x310404,0x165220),
(0x276290,0x26b310),(0x2762cc,0x26b524),(0x276590,0x359e48)]
# Short bounded disassembly windows: do not mistake adjoining functions or
# embedded data for complete function bodies.
DEPENDENCIES={
0x276234:0x276240,0x276538:0x27655c,0x276580:0x276598,
0x276b68:0x276bf4,0x2767f4:0x276838,0x2773d4:0x27744c,
0x36da14:0x36daa0,0x36dbf4:0x36dc80,0x276598:0x276600,
0x2762bc:0x2762d4,0x276274:0x2762a0,0x3103f0:0x31040c,
0x3102d4:0x3102f0,0x441f40:0x441f70,0x30fd54:0x30fda0,
0x30fdcc:0x30fe50
}
class Elf:
 def __init__(self,path):
  self.data=path.read_bytes()
  if hashlib.sha256(self.data).hexdigest()!=SHA:raise ValueError("Wrong original USA SCUS_975.01")
  if self.data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.data,18)[0]!=8:
   raise ValueError("Expected little-endian MIPS ELF32")
  ph=struct.unpack_from("<I",self.data,28)[0]
  size,n=struct.unpack_from("<HH",self.data,42)
  self.load=[]
  for i in range(n):
   k,o,v,_,length,_,flags,_=struct.unpack_from("<8I",self.data,ph+i*size)
   if k==1 and o+length<=len(self.data):self.load.append((v,v+length,o))
  if not self.load:raise ValueError("Missing load segments")
 def word(self,addr):
  for a,b,off in self.load:
   if a<=addr<=b-4:return struct.unpack_from("<I",self.data,off+addr-a)[0]
  raise ValueError("Unmapped retail ELF address "+hex(addr))
def dest(site,opcode):return ((site+4)&0xf0000000)|((opcode&0x3ffffff)<<2)
def verify(path):
 elf=Elf(path)
 anchors={int(x.split(":")[0],16):int(x.split(":")[1],16) for x in ANCHORS.split()}
 for va,w in anchors.items():
  if elf.word(va)!=w:raise ValueError("Retail opcode mismatch "+hex(va))
 for pc,addr in EDGES:
  op=elf.word(pc)
  if op>>26 not in (2,3) or dest(pc,op)!=addr:
   raise ValueError("Original call target mismatch "+hex(pc))
 windows=[]
 for start,end in DEPENDENCIES.items():
  direct=[]
  for pc in range(start,end,4):
   op=elf.word(pc)
   if op>>26 in (2,3):
    direct.append({"site":hex(pc),"target":hex(dest(pc,op)),
                   "kind":"JAL" if op>>26==3 else "J"})
  windows.append({"entry":hex(start),"sampled_bytes":end-start,"calls":direct})
 return {"format":"ae3/original-action-and-manager-elf-audit/v1",
         "source_sha256":SHA,"verified_original_instructions":len(anchors),
         "verified_direct_targets":len(EDGES),
         "native_original_routines":["0x3168ac","0x3168b8","0x3168c4",
            "0x57ba00","0x316fd0","0x3102f0","0x310358","0x310380",
            "0x3103a8","0x3103f0","0x276274","0x2762bc","0x276580"],
         "bounded_dependency_windows":windows,
         "caution":"Untranslated game factory virtual methods and COP2/VU behavior remain. No retail Windows gameplay. Bounded windows are not entire function CFGs."}
if __name__=="__main__":
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("original_elf",type=Path);p.add_argument("--out",type=Path)
 args=p.parse_args()
 report=verify(args.original_elf)
 output=json.dumps(report,indent=2)+"\n"
 if args.out:
  args.out.parent.mkdir(parents=True,exist_ok=True)
  args.out.write_text(output,encoding="utf-8")
 else:print(output,end="")
 print("PASS: verified",report["verified_original_instructions"],"opcode anchors and",
       report["verified_direct_targets"],"original call targets")
