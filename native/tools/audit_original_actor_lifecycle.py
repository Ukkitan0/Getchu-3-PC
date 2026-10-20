#!/usr/bin/env python3
"""Verify original AE3 retail actor lifecycle code and map direct call edges.
Only metadata is generated. No retail machine code or game files go into Git.
"""
import argparse, hashlib, json, struct
from pathlib import Path

PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
# Address => (instruction site, 2=J or 3=JAL, original call target)
ROUTES={
0x57173c:[(0x571770,3,0x3126e8)],
0x57178c:[(0x5717d8,3,0x312760)],
0x5718cc:[(0x5718fc,2,0x310730)],
0x571668:[(0x571680,2,0x3168d0)],
0x5716a8:[(0x5716bc,2,0x30fda0)],
0x349fe4:[(0x349ff4,2,0x33f240)],
0x349ffc:[(0x34a00c,2,0x33f26c)],
0x33f240:[(0x33f250,3,0x34310c),(0x33f264,2,0x3511d4)],
0x33fd90:[(0x33fda0,3,0x26fa1c),(0x33fda8,3,0x29157c),(0x33fdbc,2,0x29164c)],
0x349b2c:[(0x349b34,3,0x35ab88),(0x349b3c,3,0x362178)],
0x2915ec:[(0x291608,2,0x2acc6c)],
0x291588:[(0x291594,3,0x41b800),(0x2915a0,3,0x291560)],
0x2915c0:[(0x2915d0,3,0x41a3b0)],
0x3126e8:[(0x312700,3,0x41b800),(0x312724,3,0x2762bc),(0x31273c,3,0x277118)],
0x571904:[(0x571918,3,0x310890),(0x571928,3,0x571e10),
          (0x571934,3,0x316174),(0x571944,3,0x3108d4),
          (0x571960,3,0x20af68),(0x571990,3,0x30fc4c),(0x571998,3,0x41a3b0)]
}
PLAYER=[0x5b05cc,0x5b0694,0x34a014,0x34a21c,0x442028,0x442030,
        0x34a1b8,0x349f9c,0x442048,0x349fe4,0x349ffc]
MONKEY=[0x571d0c,0x571d8c,0x5717fc,0x5719ac,0x442028,0x442030,
        0x571aa4,0x571834,0x442048,0x5718cc,0x571904]
AUX=[0x571648,0x571658,0x571668,0x571688,0x5716a8,
     0x5716c4,0x5716d8,0x57173c,0x57178c]
OTHERS=[0x20daa4,0x20d70c,0x20d514,0x34a70c,0x5b0258,0x315a0c,
        0x5714b0,0x3126e8,0x312760,0x310730,0x3168d0,0x30fda0,
        0x33fd90,0x349b2c,0x2915ec,0x33f240,0x33f26c,
        0x33fce4,0x26fb24,0x29157c,0x29164c,
        0x34310c,0x3511d4,0x2762bc,0x277118,0x41b800,
        0x26fa1c,0x20af68,0x291588,0x2915c0,0x291560]
class OriginalElf:
    def __init__(self,path):
        self.bytes=path.read_bytes()
        if hashlib.sha256(self.bytes).hexdigest()!=PIN:raise ValueError("Not the exact original USA SCUS_975.01")
        if self.bytes[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.bytes,18)[0]!=8:
            raise ValueError("Expected little-endian MIPS ELF")
        ph=struct.unpack_from("<I",self.bytes,28)[0]
        size,n=struct.unpack_from("<HH",self.bytes,42)
        self.segments=[]
        for i in range(n):
            typ,off,va,_,length,_,flags,_=struct.unpack_from("<IIIIIIII",self.bytes,ph+i*size)
            if typ==1 and off+length<=len(self.bytes):
                self.segments.append((va,va+length,off,flags))
    def word(self,pc):
        for lo,hi,off,_ in self.segments:
            if lo<=pc<=hi-4:return struct.unpack_from("<I",self.bytes,off+pc-lo)[0]
        raise ValueError("Unmapped original address "+hex(pc))
    def words(self,pc,count):return [self.word(pc+4*i) for i in range(count)]

def target(pc,w):return ((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)

def analyze(elf):
    if elf.words(0x6b3918,11)!=PLAYER or elf.words(0x6b1498,11)!=MONKEY or elf.words(0x6b14d0,9)!=AUX:
        raise ValueError("Original Player/Monkey virtual tables differ")
    for pc,expected in [
                        (0x26fa1c,0x8c8f0004),(0x26fa24,0x24020063),
                        (0x26fa28,0x01e57821),(0x26fa2c,0x71ee78c8),
                        (0x26fa34,0x71e210e8),(0x26fa40,0xac820004),
                        (0x20af7c,0x8c8f004c),(0x20af8c,0x8def0010),
                        (0x20af98,0xae11004c),(0x291584,0x8de21fe8),
                        (0x2915b0,0xadf01fe8),(0x2915d8,0xae001fe8),
                        (0x707100,0x3e4ccccd),(0x312704,0x24040008),
                        (0x312710,0x25ef1240),(0x312714,0xac510004),
                        (0x312718,0xac4f0000),(0x571774,0x8e040060),
                        (0x5717dc,0x8e040060),(0x571750,0x8def0064),
                        (0x571778,0xae020064),(0x5717b0,0x8def0064),
                        (0x5717e0,0xae020064),(0x5718dc,0x8c840064),
                        (0x5718e4,0x8def000c),(0x33fda4,0x24849910),
                        (0x2915f8,0x8defe478)]:
        if elf.word(pc)!=expected:raise ValueError("Original source data/instruction differs at "+hex(pc))
    for method,sites in ROUTES.items():
        for pc,kind,dest in sites:
            op=elf.word(pc)
            if op>>26!=kind or target(pc,op)!=dest:
                raise ValueError("Original branch differs at "+hex(pc)+" from "+hex(method))
    roots=sorted(set(PLAYER+MONKEY+AUX+OTHERS+list(ROUTES)))
    sampled=[]
    for pc in roots:
        edges=[]
        for site in range(pc,pc+0x80,4):
            try:op=elf.word(site)
            except ValueError:break
            if op>>26 in (2,3):
                edges.append({"site":hex(site),"kind":"JAL" if op>>26==3 else "J","target":hex(target(site,op))})
        sampled.append({"entry":hex(pc),"first_128_bytes_edges":edges})
    inbound={pc:[] for pc in roots}
    for lo,hi,off,flags in elf.segments:
        if not flags&1:continue
        for pc in range(lo,hi-3,4):
            op=struct.unpack_from("<I",elf.bytes,off+pc-lo)[0]
            if op>>26 not in (2,3):continue
            dst=target(pc,op)
            if dst in inbound:inbound[dst].append(pc)
    return {
        "format":"ae3/original-engine-callgraph-and-lifecycle/v1",
        "retail_elf_sha256":PIN,
        "source_methods_checked":len(ROUTES)+3,
        "exact_original_branches_verified":sum(len(v) for v in ROUTES.values()),
        "sampled_entry_count":len(roots),
        "semantics":"Source-level host C++ call ordering reconstructed for original routines; original callees remain to be ported.",
        "warning":"First-128-byte windows are NOT entire functions, and direct incoming references may include embedded data; indirect calls not resolved.",
        "verified_edges":[{"entry":hex(k),"edges":[{"site":hex(site),"target":hex(dst),"kind":"JAL" if typ==3 else "J"} for site,typ,dst in vals]} for k,vals in ROUTES.items()],
        "outbound_windows":sampled,
        "potential_inbound":[{"entry":hex(pc),"total":len(inbound[pc]),"sites":[hex(x) for x in inbound[pc][:16]]} for pc in roots]
    }
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path);p.add_argument("--out",type=Path)
    args=p.parse_args()
    report=analyze(OriginalElf(args.original_elf))
    content=json.dumps(report,indent=2)+"\n"
    if args.out:
        args.out.parent.mkdir(parents=True,exist_ok=True)
        args.out.write_text(content,encoding="utf-8")
    else:print(content,end="")
    print("Source methods:",report["source_methods_checked"])
    print("Verified branches:",report["exact_original_branches_verified"])
    print("Callgraph roots:",report["sampled_entry_count"])
if __name__=="__main__":main()
