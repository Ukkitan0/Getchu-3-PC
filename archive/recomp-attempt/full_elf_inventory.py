#!/usr/bin/env python3
"""Exhaustive *word inventory* for retail AE3 .text and .vutext sections.

Each aligned word in .text gets a row, even when llvm-objdump elides repeated
NOPs. Disassembly is an instruction *candidate*, not a validated function,
particularly where code and embedded data are intermixed. VU packets are
separate and must not be decoded as R5900. No game bytes are committed to git.
"""
from __future__ import annotations
import argparse
import collections
import gzip
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

EXPECTED='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
LINE=re.compile(r'^\s*([0-9a-fA-F]+):\s+((?:[0-9a-fA-F]{2}\s+){4})(.*)$')

def sections(data:bytes):
    if len(data)<52 or data[:4]!=b'\x7fELF' or data[4:6]!=b'\x01\x01':
        raise ValueError('Expected ELF32 little-endian')
    shoff=struct.unpack_from('<I',data,32)[0]
    shentsize,shnum,shstr=struct.unpack_from('<HHH',data,46)
    if shentsize<40 or shnum<1 or shstr>=shnum or shoff+shnum*shentsize>len(data):
        raise ValueError('ELF section header bounds invalid')
    sh=[]
    for i in range(shnum):
        off=shoff+i*shentsize
        sh.append(dict(zip(('name_off','type','flags','addr','offset','size','link','info','align','entsize'),struct.unpack_from('<10I',data,off))))
    strs=sh[shstr]
    if strs['offset']+strs['size']>len(data):raise ValueError('ELF section string table out of bounds')
    names=data[strs['offset']:strs['offset']+strs['size']]
    out={}
    for s in sh:
        n=s['name_off']
        if n>=len(names):continue
        end=names.find(b'\0',n)
        if end<0:continue
        name=names[n:end].decode('ascii','replace')
        if s['type']!=8 and s['offset']+s['size']>len(data):
            raise ValueError(f'Out-of-bounds section {name}')
        out[name]=s
    return out

def main():
    p=argparse.ArgumentParser()
    p.add_argument('elf',type=Path)
    p.add_argument('--out',type=Path,default=Path('analysis/full_elf'))
    p.add_argument('--llvm-objdump',default='llvm-objdump')
    a=p.parse_args()
    data=a.elf.read_bytes()
    sha=hashlib.sha256(data).hexdigest()
    if sha!=EXPECTED:raise SystemExit('Unknown ELF sha256; refuse to generate ungrounded inventory: '+sha)
    s=sections(data)
    if '.text' not in s or '.vutext' not in s:raise SystemExit('Expected both .text and .vutext sections')
    text=s['.text']; vu=s['.vutext']
    if text['size']%4 or vu['size']%8:raise SystemExit('Unaligned machine text or VU packet length')
    a.out.mkdir(parents=True,exist_ok=True)
    decoded={}
    command=[a.llvm_objdump,'--triple=mipsel','-d','--section=.text',str(a.elf)]
    proc=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,errors='replace')
    assert proc.stdout is not None
    for line in proc.stdout:
        m=LINE.match(line)
        if not m:continue
        pc=int(m.group(1),16)
        if pc<text['addr'] or pc>=text['addr']+text['size']:continue
        if (pc-text['addr'])%4:continue
        mnemonic=m.group(3).strip().split('\t')[0]
        if mnemonic:decoded[pc]=m.group(3).strip().replace('\t',' ')
    errors=proc.stderr.read() if proc.stderr else ''
    if proc.wait():raise RuntimeError('llvm-objdump failed: '+errors[:1000])
    textdata=data[text['offset']:text['offset']+text['size']]
    raw_codes=collections.Counter()
    mnemonics=collections.Counter()
    direct_calls=collections.Counter()
    direct_branch_count=0
    zero_words=0
    unknown=0
    rendered=0
    output=a.out/'ee_text_word_inventory.tsv.gz'
    with gzip.open(output,'wt',encoding='utf-8',compresslevel=6) as f:
        f.write('address\tword\tllvm_disassembly\n')
        for idx in range(text['size']//4):
            pc=text['addr']+idx*4
            word=struct.unpack_from('<I',textdata,idx*4)[0]
            asm=decoded.get(pc)
            if asm is None: asm='nop (llvm elided)' if not word else '<no llvm disassembly>'
            else:rendered+=1
            if word==0:zero_words+=1
            if asm.startswith('<unknown>') or asm.startswith('<no llvm'):unknown+=1
            raw_codes[word>>26]+=1
            mnemonics[asm.split()[0]]+=1
            if word>>26==3:
                target=((pc+4)&0xf0000000)|((word&0x3ffffff)<<2)
                if text['addr']<=target<text['addr']+text['size']:
                    direct_calls[target]+=1
            if word>>26 in (1,4,5,6,7,20,21,22,23):direct_branch_count+=1
            f.write(f'{pc:08X}\t{word:08X}\t{asm}\n')
    vu_bytes=data[vu['offset']:vu['offset']+vu['size']]
    with gzip.open(a.out/'vu_microcode_packet_inventory.tsv.gz','wt',encoding='ascii',compresslevel=6) as f:
        f.write('address\tpacket64_le\n')
        for i in range(0,len(vu_bytes),8):
            f.write(f'{vu["addr"]+i:08X}\t{int.from_bytes(vu_bytes[i:i+8],"little"):016X}\n')
    with (a.out/'direct_jal_targets.tsv').open('w') as f:
        f.write('target\tstatic_sites\n')
        for target,count in sorted(direct_calls.items()):f.write(f'{target:08X}\t{count}\n')
    summary={
        'game_sha256':sha,
        'machine_text':{'address':f'{text["addr"]:08X}','bytes':text['size'],
                        'aligned_words':text['size']//4,'llvm_rendered_words':rendered,
                        'zero_words':zero_words,'llvm_unrecognized_words':unknown,
                        'base_opcode_frequency':{f'{k:02X}':v for k,v in sorted(raw_codes.items())},
                        'mnemonic_frequency':dict(mnemonics.most_common(60)),
                        'candidate_static_direct_jal_targets':len(direct_calls),
                        'candidate_static_direct_jal_sites':sum(direct_calls.values()),
                        'candidate_branch_word_count':direct_branch_count},
        'vu_microcode':{'address':f'{vu["addr"]:08X}','bytes':vu['size'],'64bit_packets':vu['size']//8,
                        'sha256':hashlib.sha256(vu_bytes).hexdigest()},
        'warning':'THIS IS A FULL WORD-BY-WORD BINARY INVENTORY, NOT A COMPLETE DECOMPILATION. Not all .text words represent reachable code; llvm also cannot decode all R5900 opcodes. VU code is separate.',
    }
    (a.out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(f"PASS {summary['machine_text']['aligned_words']:,} EE words, {vu['size']//8:,} VU packets, {len(direct_calls):,} candidate direct targets")
    print('Outputs:',a.out.resolve())

if __name__=='__main__':main()