#!/usr/bin/env python3
"""Verify reconstructed original Player 12-way dispatch against USA retail ELF.
The original executable stays private: emit metadata only.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
PLAYER_MAIN = [0x5B05CC,0x5B0694,0x34A014,0x34A21C,0x442028,0x442030,
               0x34A1B8,0x349F9C,0x442048,0x349FE4,0x349FFC]
MONKEY_MAIN = [0x571D0C,0x571D8C,0x5717FC,0x5719AC,0x442028,0x442030,
               0x571AA4,0x571834,0x442048,0x5718CC,0x571904]
PLAYER_CASES = [0x34A5B4,0x34A5B4,0x34A5B4,0x34A608,0x34A5E4,0x34A5FC,
                0x34A608,0x34A614,0x34A628,0x34A630,0x34A640,0x34A640]
MONKEY_SECONDARY = [0x571648,0x571658,0x571668,0x571688,0x5716A8,
                    0x5716C4,0x5716D8,0x57173C,0x57178C]

def verify(path):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXPECTED_SHA:
        raise ValueError("Original USA SCUS_975.01 hash mismatch")
    if data[:7] != b"\x7fELF\x01\x01\x01":
        raise ValueError("Expected little-endian ELF32")
    phoff = struct.unpack_from("<I",data,28)[0]
    phsize,phcount=struct.unpack_from("<HH",data,42)
    segments=[]
    for i in range(phcount):
        kind,off,va,_,length,_,flags,_=struct.unpack_from("<IIIIIIII",data,phoff+i*phsize)
        if kind==1 and off+length<=len(data):
            segments.append((va,va+length,off))
    if not segments:raise ValueError("No valid loadable ELF segment")
    def u32(va):
        for first,end,off in segments:
            if first<=va<=end-4:
                return struct.unpack_from("<I",data,off+va-first)[0]
        raise ValueError("ELF address is not mapped: "+hex(va))
    def words(va,n):return [u32(va+i*4) for i in range(n)]
    for address,expected in [(0x6B3918,PLAYER_MAIN),(0x6B1498,MONKEY_MAIN),
                             (0x6B3960,[0x34A57C]),(0x6B14D0,MONKEY_SECONDARY),
                             (0x7122FC,PLAYER_CASES)]:
        if words(address,len(expected))!=expected:
            raise ValueError("Verified original vtable/jump-table mismatch at "+hex(address))
    if [struct.unpack("<f",struct.pack("<I",u32(va)))[0]
         for va in [0x7122F4,0x7122F8]] != [20.0,100.0]:
        raise ValueError("Player mode original floating constants mismatch")
    for addr in [0x442028,0x442030,0x442048]:
        if words(addr,2)!=[0x03E00008,0]:
            raise ValueError("Expected shared original no-op virtual method: "+hex(addr))
    # Exact original MIPS decoded dispatch entry is SLTIU selector,12,
    # followed by the table-index address calculation and indirect JR.
    if u32(0x34A580)!=0x2CAE000C or u32(0x34A5AC)!=0x01E00008:
        raise ValueError("Player original switch control-flow changed")
    return {
      "format":"ae3/authentic-original-player-action-dispatch/v1",
      "source_sha256":EXPECTED_SHA,
      "player_main_vtable":[hex(n) for n in PLAYER_MAIN],
      "monkey_main_vtable":[hex(n) for n in MONKEY_MAIN],
      "player_secondary_method":"0x34a57c",
      "monkey_secondary_methods":[hex(n) for n in MONKEY_SECONDARY],
      "player_switch_table":"0x7122fc",
      "original_case_addresses":[hex(n) for n in PLAYER_CASES],
      "original_constants":[20.0,100.0],
      "shared_virtual_noops":["0x442028","0x442030","0x442048"],
      "limits":"Original function branches verified. Underlying handlers are not implemented."
    }

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("elf",type=Path)
    p.add_argument("--out",type=Path)
    a=p.parse_args()
    doc=json.dumps(verify(a.elf),indent=2)+"\n"
    if a.out:
        a.out.parent.mkdir(parents=True,exist_ok=True)
        a.out.write_text(doc,encoding="utf-8")
    else:print(doc,end="")
    print("Original Player/Monkey vtables and 12-action dispatch: PASS")

if __name__=="__main__":main()
