#!/usr/bin/env python3
"""Hash-pinned static original Ape Escape 3 USA MIPS attachment audit.

Never runs PS2 code or outputs the proprietary executable. Only public
address/instruction verification metadata and non-copyable call edges.
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
SAMPLES="""56af70:c4800054 56af78:46140032 56af7c:450000dc
56b000:8e6e0050 56b004:15d000a8 56b28c:8e8f0034 56b290:51e0ffa6 56b298:c0c40b5
56b07c:c6610054 56b080:c5e05488 56b090:c5800000 56b094:46000834 56b098:4500002e
56b09c:e6610054 56b160:460c0834 56b164:45000005 56b188:46000834 56b18c:4500003f
56b2a8:55cf0006 56b2c4:55cf0006 56b3b4:908f003d 56b3bc:11e00007 56b3c4:54a00004
56b3c8:adc50000 56b3cc:8c8f0004 56b3d0:8de50004 56b3d4:adc50000 56b3d8:24020002
56b3e8:c0102d 56b3f0:0 56b9b0:c0c4ea2 56b9c0:24020002 56b9c4:f100a
56b9d4:30af0001 56b9f0:31ef0001 56ba0c:920f003c 56ba14:dfb00000
56ba24:c09d9e7 56ba2c:a200003c 56ba9c:102d 56baa4:c0102d 56baac:0
56bff0:8c8f0008 56bff4:11e00008 56c000:8de20008 56c018:1000fffc
56c024:c0102d 56c02c:8c8e0008 56c03c:8de20010 56c05c:8c8f0008 56c06c:8def000c
315dc0:8c8f0000 315dd4:c1068ec 315dd8:a1cf00a1 315ddc:ae000000"""
TABLES={
 0x6b1220:[0x56aedc,0x56aeec,0x56af2c,0x56af3c,0x56af34,0x56af44],
 0x6b1240:[0x56aeac,0x56aebc,0x56af0c,0x56af1c,0x56af14,0x56af24],
 0x6b1260:[0x56b384,0x56b394,0x56ba98,0x56baa8,0x56baa0,0x56bab0],
 0x6b1280:[0x56b354,0x56b364,0x56b3b4,0x56b3ec,0x56b3e4,0x56b9d0]
}
FLOATS=[0x7071c0,0x7071c4,0x7071c8,0x7071cc,0x7071d0,0x7071d4,
 0x7071d8,0x7071dc,0x7071e0,0x7071e4,0x7071e8,0x7071ec,0x7071f0,0x7071f4]
def audit(path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=SHA:raise ValueError("Wrong original USA executable SHA-256")
 if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
  raise ValueError("Expected retail little-endian MIPS ELF32")
 ph=struct.unpack_from("<I",data,28)[0]
 stride,count=struct.unpack_from("<HH",data,42)
 loads=[]
 for i in range(count):
  kind,off,va,_,size,_,flags,_=struct.unpack_from("<8I",data,ph+i*stride)
  if kind==1 and off+size<=len(data):loads.append((va,va+size,off))
 if not loads:raise ValueError("No original loadable segments")
 def word(addr):
  for lo,hi,off in loads:
   if lo<=addr<=hi-4:return struct.unpack_from("<I",data,off+addr-lo)[0]
  raise ValueError("Unmapped original address "+hex(addr))
 for address,expected in TABLES.items():
  if [word(address+4*i) for i in range(len(expected))]!=expected:
   raise ValueError("Original game virtual-table mismatch "+hex(address))
 checks=[tuple(int(x,16) for x in entry.split(":")) for entry in SAMPLES.split()]
 for address,expected in checks:
  if word(address)!=expected:
   raise ValueError("Original game instruction mismatch "+hex(address))
 calls=[]
 for site in range(0x56af44,0x56b354,4):
  op=word(site)
  if op>>26==3:
   target=((site+4)&0xf0000000)|((op&0x03ffffff)<<2)
   calls.append({"site":hex(site),"callee":hex(target)})
 constants={hex(a):struct.unpack("<f",struct.pack("<I",word(a)))[0] for a in FLOATS}
 if constants["0x7071c0"]!=struct.unpack("<f",struct.pack("<I",0x3e99999a))[0]:
  raise ValueError("Original first timer phase threshold differs")
 return {"format":"ae3/original-monkey-attachment-followup/v1",
 "original_elf_sha256":SHA,
 "verified_vtables":len(TABLES),"verified_machine_words":len(checks),
 "complex_method_original_entry":"0x56af44",
 "mode_templates":{"1":"0x707110","2":"0x707130","3":"0x707150","other":"0x707170"},
 "timer_phase_constants":{"first":constants["0x7071c0"],"second":constants["0x7071d8"],
 "third":constants["0x7071dc"]},
 "additional_original_float_constants":constants,
 "complex_method_direct_calls":calls,
 "unique_direct_callees":len(set(x["callee"] for x in calls)),
 "limitations":"Only statically decoded native fragments. 16 callees and VU/COP2 operations remain to be reconstructed. No real-game Windows boot."}
if __name__=="__main__":
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("original_elf",type=Path);p.add_argument("--out",type=Path)
 args=p.parse_args()
 report=audit(args.original_elf)
 output=json.dumps(report,indent=2)+"\n"
 if args.out:
  args.out.parent.mkdir(parents=True,exist_ok=True)
  args.out.write_text(output,encoding="utf-8")
 else:print(output,end="")
 print("Verified",report["verified_vtables"],"original vtables,",
       report["verified_machine_words"],"MIPS words and",
       len(report["complex_method_direct_calls"]),"direct call sites")
