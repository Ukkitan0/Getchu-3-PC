#!/usr/bin/env python3
"""Verify connected original AE3 USA Player factory -> controller dispatch.
No original game executable, assets, or compiled code are stored in Git.
"""
import argparse, hashlib, json, struct
from pathlib import Path

SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
NAMES=["player","main_player","item_chatcher","force","bar_interface",
       "iZone","iHanger","iRope","iDashBall","iHeroSwitch"]
VMETHODS=[0x57bb60,0x57bbb4,0x57bc0c,0x57bc88,0x57bc54,0x57c620]
# Exact real original USA R5900 instruction words (opcode-level metadata).
OPCODES="""
34a04c:8c82005c 34a080:1000fff3 34a088:24820064 34a0ac:24820180
34a0cc:0c0df4d2 34a0dc:26220070 34a100:26220110 34a124:26220188
34a148:262201a0 34a16c:262201d0 34a190:26220258
57ba18:240f0001 57ba1c:ac800008 57ba20:908e0004 57ba24:15c00018
57ba28:a08f0005 57ba2c:0c088591 57ba40:8dc20008 57ba44:0040f809
57ba48:79e58940 57ba54:1040000c 57ba60:8de2002c 57ba64:0040f809
57ba6c:304200ff 57ba7c:0c087626 57ba84:ae020008
57baa8:8faf0000 57baac:ac850004 57bab4:a087001d 57babc:a0800030
57bac0:ac8f000c 57bac4:ac800014 57bad4:ac860018 57bad8:29ad0002
57badc:ac8f0000 57bae0:39ad0001 57bae4:0c15ef2e
57bb08:8faf0000 57bb10:ac8f0008 57bb14:a088001d 57bb1c:ac800010
57bb20:ac8f000c 57bb28:240f0001 57bb30:a08f0030 57bb34:29ad0002
57bb48:a08d001c 57bb4c:0c15ef2e
57bb7c:8c8e0010 57bb90:01e0f809 57bb9c:dfbf0008 57bba4:ae0f0000
57bbd0:8c8e0010 57bbe4:01e0f809 57bbf4:25ef0c40 57bbfc:ae0f0000
57bc04:081068ec 57bc14:8c8f0004 57bc18:8def0040 57bc20:24020003
57bc24:8c8f0010 57bc34:8de20008 57bc38:0040f809
57bc60:8c8e0010 57bc64:11c00005 57bc70:8de20010 57bc74:0040f809
57bc90:8c8f0010 57bc94:11e00005 57bca0:8def000c 57bca4:01e0f809
31755c:0c106e00 317560:24040070 317574:0c15eea6
3175c8:0c106e00 3175cc:24040070 3175e4:0c15eebe
37d348:3c0f006a 37d350:8de299bc 37d354:3c0f006a
37d35c:ade099bc 37d360:3c0f006a 37d364:8def99bc
37d368:11e00002 37d370:91e208a0
221644:3c0f0064 221650:10800003 221668:3c0f0064
5b04b4:0c088594 5b0654:0c088594 5b071c:0c088594
54f8dc:0c088594 550924:0c088594 5509d4:0c088594
"""
EDGES={
 0x34a0cc:0x37d348,0x57ba2c:0x221644,0x57ba7c:0x21d898,
 0x57bae4:0x57bcb8,0x57bb4c:0x57bcb8,
 0x31755c:0x41b800,0x317574:0x57ba98,
 0x3175c8:0x41b800,0x3175e4:0x57baf8,
 0x57bc04:0x41a3b0,
 0x5b04b4:0x221650,0x5b0654:0x221650,0x5b071c:0x221650,
 0x54f8dc:0x221650,0x550924:0x221650,0x5509d4:0x221650
}
class OriginalELF:
 def __init__(self,path):
  self.data=path.read_bytes()
  if hashlib.sha256(self.data).hexdigest()!=SHA:
   raise ValueError("Not the exact USA SCUS_975.01 retail ELF")
  if self.data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.data,18)[0]!=8:
   raise ValueError("Expected little-endian MIPS ELF32")
  ph=struct.unpack_from("<I",self.data,28)[0]
  stride,count=struct.unpack_from("<HH",self.data,42)
  self.segments=[]
  for i in range(count):
   kind,off,addr,_,size,_,flags,_=struct.unpack_from("<8I",self.data,ph+i*stride)
   if kind==1 and off+size<=len(self.data):
    self.segments.append((addr,addr+size,off))
  if not self.segments:raise ValueError("No ELF load segments")
 def read(self,addr,n):
  for start,end,off in self.segments:
   if start<=addr and addr+n<=end:
    return self.data[off+addr-start:off+addr-start+n]
  raise ValueError("Unmapped original ELF address "+hex(addr))
 def word(self,addr):return struct.unpack("<I",self.read(addr,4))[0]

def verify(path):
 elf=OriginalELF(path)
 checks={int(a,16):int(w,16) for a,w in (item.split(":") for item in OPCODES.split())}
 for pc,w in checks.items():
  if elf.word(pc)!=w:raise ValueError("Retail opcode differs at "+hex(pc))
 for pc,dst in EDGES.items():
  w=elf.word(pc);target=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
  if w>>26 not in (2,3) or target!=dst:
   raise ValueError("Original direct call/jump differs at "+hex(pc))
 if elf.word(0x6b3918+8)!=0x34a014:
  raise ValueError("Player vtable[2] is not the recovered original factory")
 if [elf.word(0x6b1820+4*i) for i in range(6)]!=VMETHODS:
  raise ValueError("Controller original vtable differs")
 for i,name in enumerate(NAMES):
  if elf.read(0x7121e0+16*i,16)!=name.encode("ascii").ljust(16,b"\0"):
   raise ValueError("Original name mismatch: "+name)
 if elf.read(0x708940,16)!=b"main_player".ljust(16,b"\0"):
  raise ValueError("Original 0x57BA00 lookup name is not main_player")
 return {
  "format":"ae3/original-retail-player-factory-manager-chain/v1",
  "original_elf_sha256":SHA,
  "exact_original_MIPS_instructions":len(checks),
  "original_J_JAL_target_checks":len(EDGES),
  "name_entries": [{"name":name,"original_va":hex(0x7121e0+16*i)} for i,name in enumerate(NAMES)],
  "player_factory_vtable_2":"0x34a014",
  "original_manager_factory_key":"main_player",
  "original_controller_vtable":[hex(x) for x in VMETHODS],
  "original_112_byte_allocations":["0x317538","0x31759c"],
  "original_global_register_sites":["0x5b04b4","0x54f8dc"],
  "original_global_clear_sites":["0x5b0654","0x5b071c","0x550924","0x5509d4"],
  "remaining_dependencies":["0x57bcb8","0x21d898","child virtual slot 11","0x37d348 provider setup"],
  "game_status":"No real retail native game boot; incomplete original engine"
 }
if __name__=="__main__":
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("original_elf",type=Path)
 ap.add_argument("--out",type=Path)
 a=ap.parse_args()
 report=verify(a.original_elf)
 payload=json.dumps(report,indent=2)+"\n"
 if a.out:
  a.out.parent.mkdir(parents=True,exist_ok=True)
  a.out.write_text(payload,encoding="utf-8")
 else:print(payload,end="")
 print("PASS original ELF: 93 opcode sites, 16 call/jump targets, two vtables, ten names")
