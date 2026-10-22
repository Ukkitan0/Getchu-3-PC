#!/usr/bin/env python3
"""Read-only original USA MIPS control-flow audit for actor controller 0x57BCB8.

Requires the user's private SHA-pinned SCUS_975.01. Local reports contain
instruction opcode words: keep reports private and do not commit them. This
is a static disassembly aid, not a controller implementation or emulator.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from dataclasses import asdict, dataclass
from pathlib import Path

ELF_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGION_SHA = 'ca0b2213c64510b3491ec4799aebc5e6a0bb2b375cd43ea0c9f9094e75070f38'
START, END = 0x57BCB8, 0x57BD3C
# Existing source-verified opcode anchors, not guessed function bytes.
ANCHORS = {
    0x57BCB8: 0x27BDFF80,  # ADDIU sp,sp,-128
    0x57BCDC: 0x908F0030,  # LBU t7,+0x30(a0)
    0x57BCE0: 0x11E00012,  # BEQ t7,zero,0x57BD2C
    0x57BD24: 0x45010002,  # BC1T 0x57BD30
}
REGS = ('zero','at','v0','v1','a0','a1','a2','a3',
        't0','t1','t2','t3','t4','t5','t6','t7',
        's0','s1','s2','s3','s4','s5','s6','s7',
        't8','t9','k0','k1','gp','sp','fp','ra')
LOADS = {0x20:'lb',0x21:'lh',0x23:'lw',0x24:'lbu',0x25:'lhu',0x27:'lwu',
         0x31:'lwc1',0x35:'ldc1',0x36:'lqc2',0x37:'ld'}
STORES = {0x28:'sb',0x29:'sh',0x2b:'sw',0x39:'swc1',0x3d:'sdc1',0x3e:'sqc2',0x3f:'sd'}
IMMEDIATES = {0x08:'addi',0x09:'addiu',0x0a:'slti',0x0b:'sltiu',
              0x0c:'andi',0x0d:'ori',0x0e:'xori',0x0f:'lui',
              0x18:'daddi',0x19:'daddiu'}
BRANCH_LIKELY = frozenset(('beql', 'bnel', 'blezl', 'bgtzl',
                          'bltzl', 'bgezl', 'bltzall', 'bgezall',
                          'bc1fl', 'bc1tl'))
SPECIAL = {0x00:'sll',0x02:'srl',0x03:'sra',0x04:'sllv',0x06:'srlv',
           0x08:'jr',0x09:'jalr',0x0c:'syscall',0x21:'addu',0x23:'subu',
           0x24:'and',0x25:'or',0x26:'xor',0x27:'nor',0x2a:'slt',
           0x2b:'sltu',0x2d:'daddu',0x2f:'dsubu'}


def signed16(word: int) -> int:
    v = word & 0xffff
    return v - 0x10000 if v & 0x8000 else v


def branch_target(pc: int, word: int) -> int:
    return pc + 4 + signed16(word) * 4


def jump_target(pc: int, word: int) -> int:
    return ((pc + 4) & 0xf0000000) | ((word & 0x03ffffff) << 2)


@dataclass(frozen=True)
class Instruction:
    address: str
    mnemonic: str
    operands: str
    word: str
    flow: str = 'linear'
    target: str | None = None
    delay_slot: bool = False


def decode(pc: int, word: int) -> Instruction:
    op = word >> 26
    rs, rt, rd, sa, fn = ((word >> 21) & 31, (word >> 16) & 31,
                            (word >> 11) & 31, (word >> 6) & 31, word & 63)
    name, args, flow, target = 'unknown', '', 'linear', None
    if op == 0:
        name = SPECIAL.get(fn, 'special_unknown')
        if fn == 8:
            args, flow = REGS[rs], 'indirect_jump'
            if rs == 31:
                flow = 'return'
        elif fn == 9:
            args, flow = f'{REGS[rd]}, {REGS[rs]}', 'indirect_call'
        elif fn in (0,2,3):
            args = f'{REGS[rd]}, {REGS[rt]}, {sa}'
        else:
            args = f'{REGS[rd]}, {REGS[rs]}, {REGS[rt]}'
    elif op in (2,3):
        name, flow = ('j','jump') if op == 2 else ('jal','call')
        target = jump_target(pc,word)
        args = f'0x{target:08x}'
    elif op in (4,5,6,7,0x14,0x15,0x16,0x17):
        name = {4:'beq',5:'bne',6:'blez',7:'bgtz',0x14:'beql',
                0x15:'bnel',0x16:'blezl',0x17:'bgtzl'}[op]
        target, flow = branch_target(pc,word), 'branch'
        args = f'{REGS[rs]}, {REGS[rt]}, 0x{target:08x}' if op in (4,5,0x14,0x15) else f'{REGS[rs]}, 0x{target:08x}'
    elif op == 1:
        name = {0:'bltz',1:'bgez',2:'bltzl',3:'bgezl',16:'bltzal',
                17:'bgezal',18:'bltzall',19:'bgezall'}.get(rt,'regimm_unknown')
        if name != 'regimm_unknown':
            target, flow = branch_target(pc,word), 'branch'
            args = f'{REGS[rs]}, 0x{target:08x}'
    elif op == 0x11 and rs == 8:
        name = ('bc1t' if rt & 1 else 'bc1f') + ('l' if rt & 2 else '')
        target, flow = branch_target(pc,word), 'branch'
        args = f'0x{target:08x}'
    elif op in LOADS or op in STORES:
        name = (LOADS | STORES)[op]
        dest = f'f{rt}' if op in (0x31,0x35,0x39,0x3d) else f'vf{rt}' if op in (0x36,0x3e) else REGS[rt]
        args = f'{dest}, {signed16(word)}({REGS[rs]})'
    elif op in IMMEDIATES:
        name = IMMEDIATES[op]
        args = f'{REGS[rt]}, 0x{word & 0xffff:x}' if op == 15 else f'{REGS[rt]}, {REGS[rs]}, {signed16(word)}'
    elif op in (0x11,0x12,0x1c):
        name = {0x11:'cop1_unresolved',0x12:'cop2_unresolved',0x1c:'mmi_unresolved'}[op]
        args = f'rs={rs}, rt={rt}, rd={rd}, fn={fn}'
    else:
        name, args = 'unresolved', f'opcode={op}, rs={rs}, rt={rt}'
    return Instruction(f'0x{pc:08x}',name,args,f'0x{word:08x}',flow,
                       None if target is None else f'0x{target:08x}')


def analyze_words(words: list[int], start: int = START) -> dict:
    if not words:
        raise ValueError('Empty source instruction window')
    ins = [decode(start + 4 * i,w) for i,w in enumerate(words)]
    branches=[]
    delay_slots=set()
    targets=set()
    for i,item in enumerate(ins):
        if item.flow in ('branch','jump','call','indirect_jump','indirect_call','return'):
            if i+1 >= len(ins):
                raise ValueError('Original branch/call delay slot outside audited region')
            delay_slots.add(i+1)
            if item.target is not None:
                targets.add(item.target)
            branches.append({'site':item.address,'kind':item.flow,
                             'target':item.target,'delay_slot':ins[i+1].address,
                             'fallthrough':f'0x{start+4*(i+2):08x}' if item.flow in ('branch','call','indirect_call') else None,
                             'annul_delay_on_not_taken':item.flow=='branch' and item.mnemonic in BRANCH_LIKELY})
    # Keep branch delay slots IN their predecessor block. Splitting at
    # the instruction immediately after the delay slot avoids inventing
    # a C++ state transition from an MIPS jump's delay instruction.
    cuts={start, start+4*len(words)}
    for item in branches:
        site=int(item['site'],16)
        cuts.add(site+8)
        if item['target'] is not None:
            target=int(item['target'],16)
            if start<=target<start+4*len(words):
                cuts.add(target)
    ordered=sorted(cuts)
    blocks=[]
    for lo,hi in zip(ordered,ordered[1:]):
        selected=[x for x in ins if lo<=int(x.address,16)<hi]
        if not selected:continue
        transfer=next((b for b in branches if lo<=int(b['site'],16)<hi),None)
        successors=[]
        if transfer is None:
            if hi<start+4*len(words):successors=[f'0x{hi:08x}']
        elif transfer['kind']=='branch':
            successors=[x for x in (transfer['target'],transfer['fallthrough']) if x]
        elif transfer['kind'] in ('call','indirect_call'):
            successors=[transfer['fallthrough']] if transfer['fallthrough'] else []
        elif transfer['kind']=='jump' and transfer['target']:
            successors=[transfer['target']]
        blocks.append({'start':f'0x{lo:08x}','end_exclusive':f'0x{hi:08x}',
                       'instructions':[i.address for i in selected],
                       'successors':list(dict.fromkeys(successors))})
    result=[]
    for i,item in enumerate(ins):
        d=asdict(item)
        d['delay_slot']=i in delay_slots
        result.append(d)
    return {'range':[f'0x{start:08x}',f'0x{start+4*len(words):08x}'],
            'instruction_count':len(ins),'instructions':result,
            'control_transfers':branches,'basic_blocks':blocks,'branch_targets':sorted(targets),
            'unresolved_instructions':sum('unresolved' in x.mnemonic or 'unknown' in x.mnemonic for x in ins)}


def load_original_region(path: Path) -> bytes:
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError('Private original USA SCUS_975.01 SHA-256 mismatch')
    if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
        raise ValueError('Expected little-endian MIPS ELF32')
    phoff=struct.unpack_from('<I',data,28)[0]
    phentsize,phnum=struct.unpack_from('<HH',data,42)
    if phentsize<32 or phoff+phentsize*phnum>len(data):
        raise ValueError('Invalid source program header table')
    for i in range(phnum):
        typ,offset,va,_,size,_,_,_=struct.unpack_from('<8I',data,phoff+phentsize*i)
        if typ==1 and va<=START and END<=va+size and offset+size<=len(data):
            source=data[offset+START-va:offset+END-va]
            if hashlib.sha256(source).hexdigest()!=REGION_SHA:
                raise ValueError('0x57BCB8 controller region digest differs from verified source')
            return source
    raise ValueError('Original controller 0x57BCB8 region not ELF-mapped')


def audit_original(path: Path) -> dict:
    source=load_original_region(path)
    words=list(struct.unpack(f'<{len(source)//4}I',source))
    for pc,expected in ANCHORS.items():
        if words[(pc-START)//4]!=expected:
            raise ValueError(f'Retail controller opcode differs at {pc:#x}')
    report=analyze_words(words)
    report.update({'original_elf_sha256':ELF_SHA,'controller_region_sha256':REGION_SHA,
                   'source':'private SCUS_975.01, exact verified 0x57BCB8..0x57BD3C',
                   'limitation':'Static instruction facts only; no recovered gameplay update or Windows boot'})
    return report


def main() -> None:
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path)
    p.add_argument('--json-out',type=Path,help='Owner-private report containing instruction opcodes; do not commit')
    args=p.parse_args()
    report=audit_original(args.original_elf)
    output=json.dumps(report,indent=2)+'\n'
    if args.json_out:
        args.json_out.parent.mkdir(parents=True,exist_ok=True)
        args.json_out.write_text(output,encoding='utf-8')
    else:
        print(output,end='')


if __name__=='__main__':
    main()
