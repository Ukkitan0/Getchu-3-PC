#!/usr/bin/env python3
"""Decode fixed original MMI input instructions without executing PS2 code.

This is a static decompiler aid; it does not claim to have recovered
semantic source-vector ownership or a full original navrange predicate.
"""
import argparse
import json
from pathlib import Path
from verify_original_navrange_basis_projection import SHA
import hashlib,struct

GROUP = {
  8: ('PADDW PSUBW PCGTW PMAXW PADDH PSUBH PCGTH PMAXH PADDB PSUBB PCGTB ? ? ? ? ? '
      'PADDSW PSUBSW PEXTLW PPACW PADDSH PSUBSH PEXTLH PPACH PADDSB PSUBSB PEXTLB PPACB ? ? PEXT5 PPAC5').split(),
  40: ('? PABSW PCEQW PMINW PADSBH PABSH PCEQH PMINH ? ? PCEQB ? ? ? ? ? '
      'PADDUW PSUBUW PEXTUW ? PADDUH PSUBUH PEXTUH ? PADDUB PSUBUB PEXTUB QFSRV ? ? ? ?').split(),
  9: ('PMADDW ? PSLLVW PSRLVW PMSUBW ? ? ? PMFHI PMFLO PINTH ? PMULTW PDIVW PCPYLD ? '
      'PMADDH PHMADH PAND PXOR PMSUBH PHMSBH ? ? ? ? PEXEH PREVH PMULTH PDIVBW PEXEW PROT3W').split(),
  41: ('PMADDUW ? ? PSRAVW ? ? ? ? PMTHI PMTLO PINTEH ? PMULTUW PDIVUW PCPYUD ? '
       '? ? POR PNOR ? ? ? ? ? ? PEXCH PCPYH ? ? PEXCW ?').split(),
}
assert all(len(group)==32 for group in GROUP.values())

def decode_mmi(address,word):
    if word>>26==1 and ((word>>16)&31) in (24,25):
        return dict(address=f'0x{address:x}',op='MTSAB' if ((word>>16)&31)==24 else 'MTSAH',
                    rs=(word>>21)&31,imm=word&0xffff,raw=f'0x{word:08x}')
    if word>>26 != 0x1c:return None
    fn=word&63
    if fn not in GROUP:return None
    sub=(word>>6)&31
    name=GROUP[fn][sub]
    return dict(address=f'0x{address:x}',op=name,
                rs=(word>>21)&31,rt=(word>>16)&31,rd=(word>>11)&31,
                source_group=fn,subop=sub,raw=f'0x{word:08x}')

def audit(elf):
    b=elf.read_bytes()
    if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong original USA retail ELF')
    ph=struct.unpack_from('<I',b,28)[0];stride,n=struct.unpack_from('<HH',b,42)
    seg=[]
    for i in range(n):
        typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
        if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
    def read(va):
        for start,stop,off in seg:
            if start<=va and va+4<=stop:return struct.unpack_from('<I',b,off+va-start)[0]
        raise ValueError('Unmapped retail VA')
    instructions=[]
    for va in range(0x21effc,0x21f1ac,4):
        op=decode_mmi(va,read(va))
        if op:instructions.append(op)
    return {'original_elf_sha256':SHA,'start':'0x21effc','end':'0x21f1ac',
            'mmi_instruction_count':len(instructions),
            'instructions':instructions}
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('elf',type=Path)
    parser.add_argument('--json',action='store_true')
    args=parser.parse_args()
    report=audit(args.elf)
    if args.json:print(json.dumps(report,indent=2))
    else:
        print('Private original MMI static decode:',report['mmi_instruction_count'],'instructions')
        for row in report['instructions']:
            print(row['address'],row['op'],
                  ' '.join(f'{k}={v}' for k,v in row.items() if k in ('rs','rt','rd','imm')))
        print('Input register provenance unresolved; this is NOT the completed eligibility predicate')
