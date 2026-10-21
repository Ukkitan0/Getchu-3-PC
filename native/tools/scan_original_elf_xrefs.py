#!/usr/bin/env python3
"""Batch analyze original AE3 USA ELF direct callgraph and data pointers.

Scans the ACTUAL original executable .text and .data, not emulator code.
Every reported JAL or J is a real instruction. Inferred enclosing function
starts are heuristic: indirect JALR, tail calls and overlay entry functions
still require manual recovery before claiming complete decompilation.
"""
import argparse
import bisect
import collections
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'

def original_sections(binary, executable_only=True):
    if binary[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',binary,18)[0]!=8:
        raise ValueError('Expected owner-original ELF32 little-endian MIPS')
    shoff=struct.unpack_from('<I',binary,32)[0]
    shstep,count,stridx=struct.unpack_from('<HHH',binary,46)
    if shstep<40 or count<2 or stridx>=count or shoff+shstep*count>len(binary):
        raise ValueError('Corrupt or unsupported source ELF section directory')
    headers=[struct.unpack_from('<10I',binary,shoff+i*shstep) for i in range(count)]
    strings=bytes(binary[headers[stridx][4]:headers[stridx][4]+headers[stridx][5]])
    def name(i):
        at=headers[i][0]
        end=strings.find(b'\0',at)
        return strings[at:end].decode('ascii','replace') if 0<=at<end else '?'
    for i,header in enumerate(headers):
        _,typ,flags,va,off,length,*_=header
        if ((not executable_only or (flags&4)) and typ==1 and length
                and length%4==0 and off+length<=len(binary)):
            yield name(i),va,memoryview(binary)[off:off+length]

def scan(original_elf,focus,depth=1):
    source=Path(original_elf).read_bytes()
    if hashlib.sha256(source).hexdigest()!=ELF_SHA:
        raise ValueError('Expected owner-original USA SCUS_975.01 (SHA pinned)')
    section=[s for s in original_sections(source) if s[0]=='.text']
    if len(section)!=1:
        raise ValueError('Original .text executable section was not found uniquely')
    _,base,code=section[0]
    end=base+len(code)
    data_pointers=collections.defaultdict(list)
    for kind,va,bytes_ in original_sections(source,executable_only=False):
        if kind not in ('.data','.rodata','.sdata','.ctors','.dtors'):
            continue
        for off in range(0,len(bytes_)-3,4):
            dst=struct.unpack_from('<I',bytes_,off)[0]
            if base<=dst<end and dst%4==0:
                data_pointers[dst].append((kind,va+off))

    jal=collections.defaultdict(list)
    jumps=collections.defaultdict(list)
    branches=0
    indirect=0
    vu0=0
    for off in range(0,len(code),4):
        addr=base+off
        inst=struct.unpack_from('<I',code,off)[0]
        op=inst>>26
        if op==18:vu0+=1
        if op==0 and (inst&63)==9:indirect+=1
        if op in (2,3):
            dst=((addr+4)&0xf0000000)|((inst&0x3ffffff)<<2)
            if base<=dst<end:
                (jal if op==3 else jumps)[dst].append(addr)
        if op in (1,4,5,6,7,20,21,22,23):
            branches+=1

    entry=struct.unpack_from('<I',source,24)[0]
    candidate_entries=sorted(set(jal)|{entry})
    def heuristic_owner(addr):
        i=bisect.bisect_right(candidate_entries,addr)-1
        return candidate_entries[i] if i>=0 else None

    levels=[]
    targets=set(focus)
    for level in range(depth+1):
        row=[]
        next_targets=set()
        for target in sorted(targets):
            sites=jal.get(target,())
            observed=[]
            for site in sites:
                owner=heuristic_owner(site)
                observed.append({'call_site':f'0x{site:08X}',
                                 'heuristic_owner':f'0x{owner:08X}' if owner else None})
                if owner is not None:next_targets.add(owner)
            row.append({
                'target':f'0x{target:08X}',
                'direct_original_JAL_count':len(sites),
                'source_calls':observed[:400],
                'calls_truncated':len(observed)>400,
                'original_source_data_pointer_locations':[
                    {'section':kind,'address':f'0x{src:08X}'}
                    for kind,src in data_pointers.get(target,())[:100]
                ]})
        levels.append({'level':level,'targets':row})
        targets=next_targets
        if not targets:break

    return {
        'retail_elf_sha256':ELF_SHA,
        'original_text_range':[f'0x{base:08X}',f'0x{end:08X}'],
        'original_instruction_words_scanned':len(code)//4,
        'exact_direct_JAL_sites':sum(map(len,jal.values())),
        'distinct_direct_JAL_targets':len(jal),
        'exact_direct_J_sites':sum(map(len,jumps.values())),
        'indirect_JALR_sites_needing_recovery':indirect,
        'COP2_macro_instructions':vu0,
        'conditional_branch_instructions':branches,
        'original_data_section_pointers_to_text':sum(map(len,data_pointers.values())),
        'distinct_data_code_pointer_targets':len(data_pointers),
        'reverse_xrefs':levels,
        'limitation':'Function owners are inferred from direct JAL target order only; vtables, tail calls, overlays and VU1 require further recovery.'
    }

if __name__=='__main__':
    cli=argparse.ArgumentParser(description=__doc__)
    cli.add_argument('private_retail_elf',type=Path)
    cli.add_argument('--focus',nargs='+',type=lambda s:int(s,0),
        default=[0x57bcb8,0x57c7cc,0x57c8ec,0x57ca68,0x4c1b3c,0x21b788,0x36f4d4,0x36f548])
    cli.add_argument('--depth',type=int,default=1)
    cli.add_argument('--output',type=Path)
    args=cli.parse_args()
    if not 0<=args.depth<=4:
        cli.error('--depth must be 0..4')
    result=json.dumps(scan(args.private_retail_elf,args.focus,args.depth),indent=2)+'\n'
    if args.output:
        args.output.write_text(result,encoding='utf-8')
        print('Wrote original source callgraph '+str(args.output))
    else:
        print(result,end='')
