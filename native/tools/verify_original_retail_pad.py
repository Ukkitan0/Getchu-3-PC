#!/usr/bin/env python3
"""Verify original USA retail pad lifecycle, query table and analog constants.

Reads a private original SHA-pinned ELF, emits only facts and digests.
No original executable bytes or proprietary resources are published.
"""
import argparse
import hashlib
import struct
from pathlib import Path

USA_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
DELTA = 0xff000
REGIONS = {
    (0x517788,0x51785c): 'd06d1b0474c547b4116cd43d594f47c2ae6b1348d2268d2168e8a0a31c44aef8',
    (0x51785c,0x5179b0): '4a9d9334c9cde016d15c2d5751370118562aeaf5d93f09979cbc74f28fa6595b',
    (0x5179b0,0x517a90): 'cd947d3afa5aade6a6b276f9cce608e409f2a22fed9d6d35d4786a883faecf3a',
    (0x26eb34,0x26ed54): 'e9cdb09d5a1ecd4594a03fb172f11fd09c2ae94c6499d0ff323514238adbdf78',
}
CHANNELS = (0,3,4,6,7,5,12,14,15,13,10,8,1,11,9,2)

def audit(path: Path) -> None:
    b=path.read_bytes()
    assert hashlib.sha256(b).hexdigest()==USA_SHA, 'original ELF SHA does not match'
    def word(va:int)->int:return struct.unpack_from('<I',b,va-DELTA)[0]
    def float32(va:int)->float:return struct.unpack_from('<f',b,va-DELTA)[0]
    def jal(site:int,target:int)->None:
        w=word(site)
        assert w>>26==3 and (((site+4)&0xF0000000)|((w&0x3ffffff)<<2))==target, f'{site:#x} original call changed'
    for (a,z),expected in REGIONS.items():
        assert hashlib.sha256(b[a-DELTA:z-DELTA]).hexdigest()==expected, f'original source range {a:#x}..{z:#x} changed'
    assert struct.unpack_from('<16I',b,0x6ED0C8-DELTA)==CHANNELS, 'retail button query table changed'
    assert float32(0x6ED0AC)==struct.unpack('<f',bytes.fromhex('8888083c'))[0]
    assert float32(0x6ED0B0)==0.140625
    assert float32(0x6ED0B4)==-1.
    assert float32(0x6ED0B8)==1.
    assert float32(0x6ED0BC)==0.625
    assert float32(0x6ED0C0)==0.375
    for addr in (0x5179F4,0x517A2C,0x517A40,0x517A54,0x517A68):
        jal(addr,0x4059E8)
    jal(0x5178D8,0x405700)
    jal(0x26EBB0,0x51785C)
    jal(0x26EBC8,0x51785C)
    jal(0x26EBDC,0x517788)
    jal(0x26EBEC,0x517788)
    assert word(0x517898)==0x960f000c, 'button state offset changed'
    assert word(0x5178AC)==0xae0f0008, 'edge transition store offset changed'
    assert word(0x5178EC)==0xa600000c, 'disconnected button zeroing changed'
    print('ORIGINAL_RETAIL_PAD_ELF_AUDIT_PASSED',
          'source_ranges=4 button_channels=16 analog_constants=6 call_edges=9',
          'source_sha256='+USA_SHA)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_usa_elf',type=Path)
    audit(p.parse_args().original_usa_elf)
