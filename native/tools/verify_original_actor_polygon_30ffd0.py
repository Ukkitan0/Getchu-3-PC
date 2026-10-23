#!/usr/bin/env python3
"""Audit exact original 0x30FFD0 actor current polygon selector.

Requires owner's private SHA-pinned Ape Escape 3 USA executable. Never
commits commercial game data or fabricated spatial update semantics.
"""
import argparse,hashlib,json,struct
from pathlib import Path
PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
SOURCE_START=0x30FFD0
SOURCE_END=0x31009C
SOURCE_SHA="141b7623d8668917a8e3826bc39372ad9a5501596b03602ad4358edc044861c5"
SOURCE_OPCODE_ANCHORS={
    0x30FFD0:0x27BDFFD0, 0x30FFF0:0x8C8F0014,
    0x30FFF4:0x14A00019, 0x30FFFC:0x8C840030,
    0x310000:0x10800009, 0x310020:0x50400007,
    0x310028:0x8E440028, 0x310034:0x54400001,
    0x310038:0xAE420030, 0x31003C:0x8E420030,
    0x310078:0x304200FF, 0x31007C:0x50400003,
    0x310084:0x1000FFED, 0x310088:0xAE510030,
    0x31008C:0x1080FFEB, 0x310094:0x1000FFDE,
    0x310098:0x00000000}
CALL_TARGETS={
    0x310010:0x21B788,
    0x31002C:0x21D898,
    0x310070:0x21B788
}
class Original:
    def __init__(self,fn):
        self.data=Path(fn).read_bytes()
        if hashlib.sha256(self.data).hexdigest()!=PIN:
            raise ValueError("Expected exact original USA SCUS_975.01")
        if self.data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.data,18)[0]!=8:
            raise ValueError("Expected little-endian MIPS ELF32")
        h=struct.unpack_from("<I",self.data,28)[0]
        stride,count=struct.unpack_from("<HH",self.data,42)
        self.segments=[]
        for i in range(count):
            kind,off,va,_,n,_,_,_=struct.unpack_from("<8I",self.data,h+i*stride)
            if kind==1 and off+n<=len(self.data):
                self.segments.append((va,va+n,off))
    def read(self,va,n):
        for start,end,off in self.segments:
            if start<=va and va+n<=end:
                return self.data[off+va-start:off+va-start+n]
        raise ValueError("Original ELF address unmapped "+hex(va))
    def word(self,va):
        return struct.unpack("<I",self.read(va,4))[0]
def verify(path):
    elf=Original(path)
    if hashlib.sha256(elf.read(SOURCE_START,SOURCE_END-SOURCE_START)).hexdigest()!=SOURCE_SHA:
        raise ValueError("Complete source 0x30FFD0 method changed")
    if elf.word(0x706304)!=0x3dcccccd:
        raise ValueError("Original source eligibility margin is not 0.1f")
    if elf.word(0x57C6C0)>>26!=3 or (((0x57C6C0+4)&0xf0000000)|((elf.word(0x57C6C0)&0x03ffffff)<<2))!=SOURCE_START:
        raise ValueError("Original 0x57C620 does not invoke 0x30FFD0")
    for pc,expected in SOURCE_OPCODE_ANCHORS.items():
        if elf.word(pc)!=expected:
            raise ValueError("Original source opcode changed "+hex(pc))
    for pc,target in CALL_TARGETS.items():
        word=elf.word(pc)
        actual=((pc+4)&0xf0000000)|((word&0x03ffffff)<<2)
        if word>>26!=3 or actual!=target:
            raise ValueError("Original source call target changed "+hex(pc))
    return {"original_sha256":PIN,"original_function":"0x30FFD0",
        "verified_complete_original_instruction_words":(SOURCE_END-SOURCE_START)//4,
        "verified_opcode_sites":len(SOURCE_OPCODE_ANCHORS),
        "verified_direct_calls":len(CALL_TARGETS),
        "original_query_margin_bits":"0x3DCCCCCD",
        "in_game_actor_owner_update_verified":False}
if __name__=="__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("original_elf",type=Path)
    a=ap.parse_args()
    print(json.dumps(verify(a.original_elf),indent=2))
    print("PASS original 0x30FFD0 source selector")
