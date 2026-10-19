#!/usr/bin/env python3
"""Lift bounded, low-64-bit scalar R5900 leaf routines containing backedges.

Control-flow is translated faithfully at instruction granularity, including
MIPS delay slots and pre-slot branch decisions. Arbitrary runtime loops are
bounded by a common explicit instruction budget; returning -3 means *paused by
budget*, not that the function completed or has a known semantic result.

The restricted translation is NOT a general PS2 emulator or game port.
"""
from __future__ import annotations
import json
from pathlib import Path
import bulk_lift as base
import cfg_lift as cfg

ROOT = Path(__file__).resolve().parent
MAX_WORDS = 104
MAX_TRANSFERS = 12
INSTRUCTION_BUDGET = 4096


def scan(start):
    code = []
    slots = set()
    edges = []
    backedges = 0
    pc = start
    while len(code) < MAX_WORDS - 1:
        if not base.valid(pc):
            return None
        word = base.W[(pc-base.TEXT)//4]
        if word == 0x03e00008:  # JR $ra only
            if not base.valid(pc+4):
                return None
            slot_word = base.W[(pc+4-base.TEXT)//4]
            slot_stmt = base.emit(slot_word)
            if slot_stmt is None:
                return None
            code.extend(((pc,word,'return',None),(pc+4,slot_word,'delay_return',slot_stmt)))
            slots.add(pc+4)
            if backedges == 0 or not edges:
                return None
            pcs = {p for p,_,_,_ in code}
            if any(dest not in pcs or dest in slots or dest > pc for _,dest in edges):
                return None
            if any(dest < start for _,dest in edges):
                return None
            if len(pcs) != len(code):
                return None
            return code, backedges
        condition = cfg.branch(pc,word)
        jump = (((pc+4)&0xf0000000)|((word&0x03ffffff)<<2)) if word>>26==2 else None
        if condition or jump is not None:
            if len(edges)>=MAX_TRANSFERS or len(code)+4>MAX_WORDS:
                return None
            dest = condition[1] if condition else jump
            if not base.valid(pc+4):
                return None
            slot_word = base.W[(pc+4-base.TEXT)//4]
            slot_stmt = base.emit(slot_word)
            if slot_stmt is None:
                return None
            code.append((pc,word,'branch' if condition else 'jump',condition if condition else dest))
            code.append((pc+4,slot_word,'delay_transfer',slot_stmt))
            slots.add(pc+4)
            edges.append((pc,dest))
            if dest <= pc:
                backedges += 1
            pc += 8
        else:
            stmt = base.emit(word)
            if stmt is None:
                return None
            code.append((pc,word,'scalar',stmt))
            pc += 4
    return None


def translate():
    control, _ = cfg.translate()
    previously_occupied = {p for instr in base.accepted.values() for p,_,_ in instr}
    previously_occupied.update(p for body,_ in control.values() for p,_,_,_ in body)
    occupied = set(previously_occupied)
    recovered = {}
    rejected_overlap = []
    for entry in sorted(base.starts):
        if entry in occupied:
            continue
        result = scan(entry)
        if result is None:
            continue
        instructions, backedges = result
        positions = {p for p,_,_,_ in instructions}
        if positions & occupied:
            rejected_overlap.append(entry)
            continue
        recovered[entry] = result
        occupied.update(positions)
    return recovered, rejected_overlap


PREAMBLE = '''#include "ae3_loop_leaves.h"
#include <stdint.h>
/* Low-64-bit scalar-only MIPS R5900 subset with bounded loop steps. */
static inline uint64_t S8(uint64_t v) {return (uint64_t)(int64_t)(int8_t)(uint8_t)v;}
static inline uint64_t S16(uint64_t v) {return (uint64_t)(int64_t)(int16_t)(uint16_t)v;}
static inline uint64_t S32(uint64_t v) {return (uint64_t)(int64_t)(int32_t)(uint32_t)v;}
static int read_mem(const AE3_ScalarContext *c,uint64_t a,unsigned n,uint64_t *v){
 uint32_t off=(uint32_t)a & UINT32_C(0x1fffffff);
 if(!c->ram || off >= c->ram_size || n>c->ram_size-off) return -1;
 *v=0;for(unsigned i=0;i<n;i++) *v|=(uint64_t)c->ram[off+i]<<(8*i);
 return 0;
}
static int write_mem(AE3_ScalarContext *c,uint64_t a,unsigned n,uint64_t v){
 uint32_t off=(uint32_t)a & UINT32_C(0x1fffffff);
 if(!c->ram || off >= c->ram_size || n>c->ram_size-off) return -1;
 for(unsigned i=0;i<n;i++) c->ram[off+i]=(uint8_t)(v>>(8*i));
 return 0;
}
#define GEN_SIZE(N) \\
static int load##N(AE3_ScalarContext *c,uint64_t a,uint64_t *v){return read_mem(c,a,N,v);} \\
static int store##N(AE3_ScalarContext *c,uint64_t a,uint64_t v){return write_mem(c,a,N,v);}
GEN_SIZE(1) GEN_SIZE(2) GEN_SIZE(4) GEN_SIZE(8)
'''


def write_source(routines, overlap):
    (ROOT/'ae3_loop_leaves.h').write_text('''#ifndef AE3_LOOP_LEAVES_H
#define AE3_LOOP_LEAVES_H
#include "ae3_scalar_leaves.h"
/* -3 means instruction-budget exhaustion. State is only a partial result. */
#define AE3_LOOP_INSTRUCTION_BUDGET 4096u
int ae3_run_loop_leaf(AE3_ScalarContext *ctx, uint32_t entry);
size_t ae3_loop_leaf_count(void);
uint32_t ae3_loop_leaf_address(size_t index);
#endif
''')
    out=[PREAMBLE]
    for address,(code,nbacks) in routines.items():
        out.append(f'/* R5900 original leaf 0x{address:08X}: {nbacks} backedge(s). */')
        out.append(f'static int ae3_loop_{address:08X}(AE3_ScalarContext *c) {{')
        out.append('  uint64_t *R=c->gpr, tmp=0; (void)tmp;')
        out.append('  unsigned ticks=0;')
        conditional_index=0
        pending=None
        for pc,word,kind,arg in code:
            out.append(f'  L_{pc:08X}: ; /* {word:08X} */')
            out.append('  if (++ticks > AE3_LOOP_INSTRUCTION_BUDGET) return -3;')
            if kind=='branch':
                out.append(f'  int take_{conditional_index} = {arg[0]};')
                pending=('branch',arg[1],conditional_index)
                conditional_index+=1
            elif kind=='jump':
                pending=('jump',arg,None)
            elif kind=='delay_transfer':
                out.append('  '+(arg or '/* nop */'))
                if pending is None: raise ValueError('slot not following transfer')
                if pending[0]=='branch':
                    out.append(f'  if (take_{pending[2]}) goto L_{pending[1]:08X};')
                else:
                    out.append(f'  goto L_{pending[1]:08X};')
                pending=None
            elif kind=='return':
                pass
            elif kind=='delay_return':
                out.append('  '+(arg or '/* nop */'))
                out.append('  R[0]=0;return 0;')
            else:
                out.append('  '+(arg or '/* nop */'))
        out.append('}')
    out.extend(['int ae3_run_loop_leaf(AE3_ScalarContext *ctx,uint32_t entry){',
                ' (void)S8; (void)S16; if(!ctx)return -1; switch(entry){'])
    for addr in routines:
        out.append(f' case UINT32_C(0x{addr:08X}): return ae3_loop_{addr:08X}(ctx);')
    out.append(' default:return -2;}}')
    out.append(f'static const uint32_t entries[{len(routines)}]={{')
    out.append(',\n'.join(f'UINT32_C(0x{addr:08X})' for addr in routines))
    out.extend(['};',
                'size_t ae3_loop_leaf_count(void){return sizeof(entries)/sizeof(entries[0]);}',
                'uint32_t ae3_loop_leaf_address(size_t i){return i<ae3_loop_leaf_count()?entries[i]:0;}'])
    (ROOT/'ae3_loop_leaves.c').write_text('\n'.join(out)+'\n')
    report={'input_sha256':base.EXPECTED,'loop_candidate_count':len(routines),
            'unique_instruction_locations':sum(len(code) for code,_ in routines.values()),
            'backedges_total':sum(backedges for _,backedges in routines.values()),
            'step_budget':INSTRUCTION_BUDGET,
            'rejected_overlap':[f'{entry:08X}' for entry in overlap],
            'entries':[f'{a:08X}' for a in routines],
            'limits':'Only bounded scalar low-64-bit loop leaves; budget exhaustion -3 is not completion. No calls/MMI/COP/VU/IOP/GS.'}
    (ROOT/'loop_lift_report.json').write_text(json.dumps(report,indent=2)+'\n')
    return report

if __name__=='__main__':
    routines,overlap=translate()
    report=write_source(routines,overlap)
    print('Generated',report['loop_candidate_count'],'loop-bearing scalar leaf candidates',
          report['unique_instruction_locations'],'instruction locations and',report['backedges_total'],'backedges')