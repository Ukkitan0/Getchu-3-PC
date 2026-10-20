#!/usr/bin/env python3
"""Recover short R5900 callers of previously verified scalar native-C candidates.

Only straight-line scalar callers with direct JAL into the known resume set,
JR $ra and valid delay slots are accepted. The generated scheduler retains the
caller and child PC across yields, and never treats unsupported code as a nop.
This is NOT a general R5900 runtime and does not run the full game.
"""
from __future__ import annotations
import json
from pathlib import Path
import bulk_lift as base
import resume_lift as resume

ROOT=Path(__file__).resolve().parent
MAX_WORDS=72
MAX_CALLS=4


def collect():
    leaves,_=resume.collect()
    leafentries=set(leaves)
    occupied={pc for body,_ in leaves.values() for pc,_,_,_ in body}
    callers={}
    overlap=[]
    for entry in sorted(base.starts):
        if entry in occupied:
            continue
        code=[]; num_calls=0; pc=entry
        while len(code)<MAX_WORDS-1 and base.valid(pc):
            word=base.W[(pc-base.TEXT)//4]
            if word==0x03e00008:
                if not base.valid(pc+4):break
                sw=base.W[(pc+4-base.TEXT)//4]
                stmt=base.emit(sw)
                if stmt is None:break
                code.extend(((pc,word,'return',None),(pc+4,sw,'return_slot',stmt)))
                if num_calls:
                    pcs={p for p,_,_,_ in code}
                    if pcs & occupied:overlap.append(entry)
                    else:
                        callers[entry]=code
                        occupied.update(pcs)
                break
            if word>>26==3:
                target=((pc+4)&0xf0000000)|((word&0x03ffffff)<<2)
                if target not in leafentries or num_calls>=MAX_CALLS or not base.valid(pc+4):break
                sw=base.W[(pc+4-base.TEXT)//4]
                stmt=base.emit(sw)
                if stmt is None:break
                code.extend(((pc,word,'call',target),(pc+4,sw,'call_slot',stmt)))
                num_calls+=1; pc+=8
                continue
            stmt=base.emit(word)
            if stmt is None:break
            code.append((pc,word,'scalar',stmt))
            pc+=4
    return callers,overlap

HEADER='''#ifndef AE3_CALL_LEAVES_H
#define AE3_CALL_LEAVES_H
#include "ae3_resumable_leaves.h"
/* Only recognized direct-call wrappers. Same status codes as ae3_resume. */
typedef struct AE3_CallFrame {
    uint32_t entry, pc, target, pending;
    uint32_t active, done, reserved0, reserved1;
    uint64_t retired;
    AE3_ResumeFrame child;
} AE3_CallFrame;
void ae3_call_init(AE3_CallFrame *frame, uint32_t entry);
int ae3_run_call_slice(AE3_ScalarContext *ctx, AE3_CallFrame *frame,
                       uint32_t instruction_budget);
size_t ae3_call_count(void);
uint32_t ae3_call_address(size_t index);
#endif
'''

def write_source(callers,overlap):
    (ROOT/'ae3_call_leaves.h').write_text(HEADER)
    src=[resume.PREAMBLE.replace('"ae3_resumable_leaves.h"','"ae3_call_leaves.h"')]
    for entry,code in callers.items():
        src.extend([f'/* Retail R5900 direct caller {entry:08X}. */',
                    f'static int call_{entry:08X}(AE3_ScalarContext *c,AE3_CallFrame *f,uint32_t *budget){{',
                    ' uint64_t *R=c->gpr; uint64_t tmp=0; (void)tmp;',
                    ' while (*budget && !f->done) {',
                    '  if (f->active) {',
                    '   uint64_t before=f->child.retired;',
                    '   int status=ae3_run_resume_slice(c,&f->child,*budget);',
                    '   uint64_t delta=f->child.retired-before;',
                    '   if (delta > *budget) return AE3_RESUME_INVALID;',
                    '   f->retired+=delta; *budget-=(uint32_t)delta;',
                    '   if (status==AE3_RESUME_YIELDED) return status;',
                    '   if (status!=AE3_RESUME_DONE) return status;',
                    '   /* JR ra from a leaf must return to exactly the caller continuation. */',
                    '   if ((uint32_t)R[31]!=f->pc) return AE3_RESUME_INVALID;',
                    '   f->active=0; f->target=0;',
                    '   continue;',
                    '  }',
                    '  switch(f->pc) {'])
        for pc,word,kind,arg in code:
            src.extend([f'  case UINT32_C(0x{pc:08X}): {{ /* {word:08X} {kind} */',
                        f'   if(f->pending != {1 if kind=="call_slot" else 2 if kind=="return_slot" else 0}u) return AE3_RESUME_INVALID;'])
            if kind=='call':
                src.extend([f'   R[31]=UINT32_C(0x{pc+8:08X});',
                            f'   f->target=UINT32_C(0x{arg:08X});f->pending=1;f->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='call_slot':
                src.extend([f'   {arg or "/* nop */"}',
                            f'   f->pc=UINT32_C(0x{pc+4:08X});f->pending=0;',
                            '   ae3_resume_init(&f->child,f->target);f->active=1;'])
            elif kind=='return':
                src.extend(['   f->pending=2;',f'   f->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='return_slot':
                src.extend([f'   {arg or "/* nop */"}',
                            '   f->pc=0;f->pending=0;f->done=1;'])
            else:
                src.extend([f'   {arg or "/* nop */"}',f'   f->pc=UINT32_C(0x{pc+4:08X});'])
            src.append('   break; }')
        src.extend(['  default: return AE3_RESUME_INVALID;',
                    '  }','  R[0]=0;f->retired++;(*budget)--;',
                    ' }',' return f->done?AE3_RESUME_DONE:AE3_RESUME_YIELDED;','}'])
    src.extend(['void ae3_call_init(AE3_CallFrame *f,uint32_t entry){',
                ' if(!f)return; *f=(AE3_CallFrame){0};f->entry=entry;f->pc=entry;} ',
                'int ae3_run_call_slice(AE3_ScalarContext *c,AE3_CallFrame *f,uint32_t budget){',
                ' if(!c||!f||f->pending>2||f->active>1||f->done>1||f->reserved0||f->reserved1)return AE3_RESUME_INVALID;',
                ' if(f->done)return (!f->active&&!f->pending&&!f->pc) ? AE3_RESUME_DONE:AE3_RESUME_INVALID;',
                ' switch(f->entry){'])
    for a in callers:
        src.append(f' case UINT32_C(0x{a:08X}):return call_{a:08X}(c,f,&budget);')
    src.extend([' default:return AE3_RESUME_UNKNOWN;', ' }','}'])
    src.append(f'static const uint32_t entries[{len(callers)}]={{')
    src.append(',\n'.join(f'UINT32_C(0x{a:08X})' for a in callers))
    src.extend(['};',
                'size_t ae3_call_count(void){return sizeof(entries)/sizeof(entries[0]);}',
                'uint32_t ae3_call_address(size_t i){return i<ae3_call_count()?entries[i]:0;}'])
    (ROOT/'ae3_call_leaves.c').write_text('\n'.join(src)+'\n')
    report=dict(game_sha256=base.EXPECTED,call_candidates=len(callers),
                recovered_instructions=sum(len(c) for c in callers.values()),
                interfunction_edges=sum(sum(k=='call' for _,_,k,_ in c) for c in callers.values()),
                overlapping_exclusions=[f'{a:08X}' for a in overlap],
                callers={f'{a:08X}':[f'{arg:08X}' for _,_,kind,arg in c if kind=='call'] for a,c in callers.items()},
                scope='Scalar-only straight-line caller functions with direct JAL to a previously recovered leaf, no JALR, branch, VU/MMI/COP, or recursive callers.')
    (ROOT/'call_lift_report.json').write_text(json.dumps(report,indent=2)+'\n')
    return report

if __name__=='__main__':
    a,b=collect(); r=write_source(a,b)
    print('Generated',r['call_candidates'],'direct caller candidates,',r['interfunction_edges'],'JAL edges')