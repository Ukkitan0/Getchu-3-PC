"""Source-prove the new native 0x57BCB8 initializer; never emits retail bytes."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from verify_original_windows_bootstrap import OriginalElf, EXPECTED_SHA

SPANS = {
    (0x57BCB8, 0x2A4): '90ced0cff1cd0ad2f9fa260859db156e1dcc6fb4c4c295dfe0d62fe0df6a9e0d',
    (0x57BD80, 0x58): 'e01617d367f22eabf9507deb242534b7f0de555c57527877c2f27d931342f730',
    (0x57BDD8, 0x100): '6ae88de85fe3f6d71b653b2297f87808c7b21fa47d96651f0067f73d145d0240',
    (0x57BED8, 0x84): '89125327b3b2f85ff0b18f491c5ea885e42e0626526e4bb4fa77f8d66d1f2058',
}
CALLS = {
    0x57BD00: 0x4748AC, 0x57BD90: 0x221104, 0x57BD9C: 0x21B444,
    0x57BDCC: 0x21B458, 0x57BDDC: 0x21B458, 0x57BDF0: 0x21B458,
    0x57BE00: 0x36F4D4, 0x57BE24: 0x36F548,
    0x57BF00: 0x21B854, 0x57BF28: 0x57C7CC,
}
VU = {
    0x57BCF8: (0x2C,1,1,2),
    0x57BE8C: (0x18,1,3,1), 0x57BEAC: (0x18,2,1,2),
    0x57BEC0: (0x18,2,1,2), 0x57BEC8: (0x28,3,3,1),
    0x57BECC: (0x28,3,3,2),
    0x57BEF0: (0x18,2,2,1), 0x57BEFC: (0x28,2,1,2),
    0x57BF18: (0x28,1,1,2), 0x57BF20: (0x18,1,1,2),
}

def verify(path):
    elf=OriginalElf(Path(path).read_bytes())
    def word(va):return struct.unpack('<I',elf.read(va,4))[0]
    for (start,size),expected in SPANS.items():
        if hashlib.sha256(elf.read(start,size)).hexdigest()!=expected:
            raise ValueError(f'Original initializer span mismatch at {start:#x}')
    for site,target in CALLS.items():
        op=word(site)
        if op>>26!=3 or (((site+4)&0xF0000000)|((op&0x3FFFFFF)<<2))!=target:
            raise ValueError(f'Original initializer JAL mismatch at {site:#x}')
    for site,expected in VU.items():
        op=word(site)
        if ((op&63,(op>>6)&31,(op>>11)&31,(op>>16)&31)!=expected or
            ((op>>21)&15)!=15):
            raise ValueError(f'Original initializer vector operand/order mismatch at {site:#x}')
    for site,expected in {0x708A10:0x57C554,0x708A14:0,0x708A18:0x57BF5C,0x708A1C:0,
                          0x708A20:0x40000000,0x708A24:0x3F800000,
                          0x708A28:0x3F4CCCCD,0x708A2C:0x3D888888}.items():
        if word(site)!=expected:raise ValueError(f'Original initializer data constant mismatch at {site:#x}')
    return {'original_elf_sha256':EXPECTED_SHA,'source_va':'0x57BCB8',
            'source_end_exclusive':'0x57BF5C','unique_source_words':169,
            'verified_direct_calls':len(CALLS),'verified_vector_operations':len(VU),
            'reused_native_routines':['0x221104','0x21B444','0x21B458','0x57BDB8','0x57C98C','0x4748AC'],
            'explicit_general_path_dependencies':['0x21B854','0x57C7CC owner adapter'],
            'windows_gameplay_proven':False}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('private_original_elf',type=Path)
    print(json.dumps(verify(parser.parse_args().private_original_elf),indent=2))
