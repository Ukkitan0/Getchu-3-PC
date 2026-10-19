#!/usr/bin/env python3
"""Recover direct JAL -> JAL call chains, compile resumable stack execution to C.

Supported subset: scalar, straight-line verified caller bodies; target is either an
existing verified native leaf or another recovered caller. Each MIPS word costs
one instruction budget unit, including JAL/JR delay slots. Reject unsupported
calls and overlapping function bounds instead of fabricating control flow.
"""
from __future__ import annotations
import json
from pathlib import Path
import bulk_lift as b
import call_lift as existing
import resume_lift as resume
import extend_stack as extension

ROOT=Path(__file__).resolve().parent
MAX_WORDS=72
MAX_DEPTH=8

def collect():
    old,_=existing.collect()
    leaves,_=resume.collect()
    callers=dict(old)
    leaf_entries=set(leaves)
    occupied={pc for code in callers.values() for pc,_,_,_ in code}
    occupied.update(pc for code,_ in leaves.values() for pc,_,_,_ in code)
    nested={}
    # Each added caller MUST call a previously accepted caller. Avoid newly
    # inferred cyclic graphs and ambiguous entrypoints within other bodies.
    for entry in sorted(b.starts):
        if entry in occupied:continue
        code=[];seen_nested=False;targets=[];pc=entry
        for _ in range(MAX_WORDS):
            if not b.valid(pc):break
            w=b.W[(pc-b.TEXT)//4]
            if w==0x03e00008:
                if not b.valid(pc+4):break
                sw=b.W[(pc+4-b.TEXT)//4];stmt=b.emit(sw)
                if stmt is None:break
                code.extend([(pc,w,'return',None),(pc+4,sw,'return_slot',stmt)])
                addresses={p for p,_,_,_ in code}
                if seen_nested and not addresses.intersection(occupied):
                    callers[entry]=code; nested[entry]=code
                    occupied.update(addresses)
                break
            if (w>>26)==3:
                target=((pc+4)&0xF0000000)|((w&0x03ffffff)<<2)
                # No unverified callees and no delay-slot control transfers.
                if target not in leaf_entries and target not in old:break
                if not b.valid(pc+4):break
                sw=b.W[(pc+4-b.TEXT)//4];stmt=b.emit(sw)
                if stmt is None:break
                code.extend([(pc,w,'call',target),(pc+4,sw,'call_slot',stmt)])
                seen_nested|=target in old
                targets.append(target);pc+=8
                continue
            stmt=b.emit(w)
            if stmt is None:break
            code.append((pc,w,'scalar',stmt));pc+=4
    indirect={}
    for entry in sorted(b.starts):
        if entry in occupied:continue
        code=[]; seen_jalr=False;pc=entry
        for _ in range(MAX_WORDS):
            if not b.valid(pc):break
            w=b.W[(pc-b.TEXT)//4]
            if w==0x03e00008:
                if not b.valid(pc+4):break
                sw=b.W[(pc+4-b.TEXT)//4];stmt=b.emit(sw)
                if stmt is None:break
                code.extend([(pc,w,'return',None),(pc+4,sw,'return_slot',stmt)])
                addresses={p for p,_,_,_ in code}
                if seen_jalr and not addresses.intersection(occupied):
                    callers[entry]=code;indirect[entry]=code
                    occupied.update(addresses)
                break
            if (w>>26)==0 and (w&63)==9: # JALR rd,rs
                rs=(w>>21)&31;rd=(w>>11)&31
                # Only ABI link register and nonzero source; capture before link.
                if not rs or rd!=31 or not b.valid(pc+4):break
                sw=b.W[(pc+4-b.TEXT)//4];stmt=b.emit(sw)
                if stmt is None:break
                code.extend([(pc,w,'jalr',rs),(pc+4,sw,'jalr_slot',stmt)])
                seen_jalr=True;pc+=8
                continue
            if (w>>26)==3:
                target=((pc+4)&0xF0000000)|((w&0x03ffffff)<<2)
                if target not in leaf_entries and target not in old:break
                if not b.valid(pc+4):break
                sw=b.W[(pc+4-b.TEXT)//4];stmt=b.emit(sw)
                if stmt is None:break
                code.extend([(pc,w,'call',target),(pc+4,sw,'call_slot',stmt)])
                pc+=8;continue
            stmt=b.emit(w)
            if stmt is None:break
            code.append((pc,w,'scalar',stmt));pc+=4
    flow, flow_rejected=extension.collect(b.starts,occupied,set(callers)|leaf_entries)
    callers.update(flow)
    return dict(sorted(callers.items())),nested,indirect,leaf_entries,flow,flow_rejected

HEADER=f'''#ifndef AE3_STACK_LEAVES_H
#define AE3_STACK_LEAVES_H
#include "ae3_resumable_leaves.h"
/* Strict bound: exceeding this returns INVALID, never runs unknown MIPS code. */
#define AE3_STACK_MAX_DEPTH {MAX_DEPTH}u
typedef struct AE3_StackActivation {{
    uint32_t entry,pc,pending,target;
    uint32_t return_pc,return_target,leaf_active,reserved;
    uint64_t retired;
    AE3_ResumeFrame leaf;
}} AE3_StackActivation;
typedef struct AE3_StackFrame {{
    uint32_t entry,depth,done,reserved;
    uint64_t retired;
    AE3_StackActivation frames[AE3_STACK_MAX_DEPTH];
}} AE3_StackFrame;
void ae3_stack_init(AE3_StackFrame *frame,uint32_t entry);
int ae3_run_stack_slice(AE3_ScalarContext *ctx,AE3_StackFrame *frame,uint32_t budget);
size_t ae3_stack_count(void);
uint32_t ae3_stack_address(size_t index);
#endif
'''

def generate():
    callers,nested,indirect,leaves,flow,flow_rejected=collect()
    assert len(callers)==len(set(callers))
    (ROOT/'ae3_stack_leaves.h').write_text(HEADER)
    src=[resume.PREAMBLE.replace('"ae3_resumable_leaves.h"','"ae3_stack_leaves.h"')]
    src.extend(['static int caller_entry(uint32_t pc) {', ' switch(pc){'])
    src.extend(f' case UINT32_C(0x{addr:08X}): return 1;' for addr in callers)
    src.extend([' default: return 0;',' }','}',
                'static int leaf_entry(uint32_t pc) {',
                ' for(size_t i=0;i<ae3_resume_count();i++)',
                '  if(ae3_resume_address(i)==pc) return 1;',
                ' return 0;','}'])
    src.extend(['/* A step retires exactly one guest instruction, or returns a fault. */',
                'static int stack_step(AE3_ScalarContext *c,AE3_StackFrame *f){',
                ' uint64_t *R=c->gpr;uint64_t tmp=0;(void)tmp;',
                ' AE3_StackActivation *a=&f->frames[f->depth-1u];',
                ' switch(a->entry){'])
    for entry,code in callers.items():
        src.extend([f' case UINT32_C(0x{entry:08X}):', '  switch(a->pc){'])
        for pc,w,kind,arg in code:
            src.extend([f'   case UINT32_C(0x{pc:08X}):{{ /* {w:08X}: {kind} */',
                        f'    if(a->pending!={1 if kind in ("call_slot","jalr_slot") else 2 if kind=="return_slot" else 3 if kind=="branch_slot" else 0}u) return AE3_RESUME_INVALID;'])
            if kind=='call':
                src.extend([f'    R[31]=UINT32_C(0x{pc+8:08X});',
                            f'    a->target=UINT32_C(0x{arg:08X}); a->pending=1;',
                            f'    a->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='jalr':
                src.extend([f'    {{ uint64_t raw=R[{arg}];',
                            '      if(raw!=(uint64_t)(uint32_t)raw && raw!=(uint64_t)(int64_t)(int32_t)raw) return AE3_RESUME_UNKNOWN;',
                            '      uint32_t candidate=(uint32_t)raw;',
                            '      if(!caller_entry(candidate) && !leaf_entry(candidate)) return AE3_RESUME_UNKNOWN;',
                            '      a->target=candidate; }',
                            f'    R[31]=UINT32_C(0x{pc+8:08X});a->pending=1;',
                            f'    a->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='branch':
                condition,target=arg
                src.extend([f'    a->target=({condition}) ? UINT32_C(0x{target:08X}) : UINT32_C(0x{pc+8:08X});',
                            '    a->pending=3;',
                            f'    a->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='jump':
                src.extend([f'    a->target=UINT32_C(0x{arg:08X});a->pending=3;',
                            f'    a->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='branch_slot':
                src.extend([f'    {arg or "/* nop */"}',
                            '    a->pc=a->target;a->target=0;a->pending=0;'])
            elif kind in ('call_slot','jalr_slot'):
                # Atomic bound/target validation BEFORE modifying slot effects.
                src.append('    if(!caller_entry(a->target) && !leaf_entry(a->target)) return AE3_RESUME_UNKNOWN;')
                src.append('    if(caller_entry(a->target) && f->depth>=AE3_STACK_MAX_DEPTH) return AE3_RESUME_INVALID;')
                src.extend([f'    {arg or "/* nop */"}',
                            f'    a->pc=UINT32_C(0x{pc+4:08X});a->pending=0;'])
                src.extend(['    if(leaf_entry(a->target)){',
                            '     ae3_resume_init(&a->leaf,a->target);a->leaf_active=1;',
                            '    } else {',
                            '      AE3_StackActivation *child=&f->frames[f->depth++];',
                            '      *child=(AE3_StackActivation){0};child->entry=a->target;',
                            '      child->pc=a->target;child->return_pc=a->pc;',
                            '    }'])
            elif kind=='return':
                src.extend(['    a->return_target=(uint32_t)R[31];a->pending=2;',
                            f'    a->pc=UINT32_C(0x{pc+4:08X});'])
            elif kind=='return_slot':
                src.append('    if(f->depth>1u && a->return_target!=a->return_pc) return AE3_RESUME_INVALID;')
                src.append(f'    {arg or "/* nop */"}')
                src.extend(['    a->pending=0;a->pc=0;',
                            '    if(f->depth>1u){ f->depth--; } else {f->depth=0;f->done=1;}'])
            else:
                src.extend([f'    {arg or "/* nop */"}',f'    a->pc=UINT32_C(0x{pc+4:08X});'])
            src.extend(['    break; }'])
        src.extend(['   default:return AE3_RESUME_INVALID;','  }','  break;'])
    src.extend([' default:return AE3_RESUME_UNKNOWN;',' }',
                ' R[0]=0; a->retired++;f->retired++;return AE3_RESUME_DONE;','}'])
    src.extend(['void ae3_stack_init(AE3_StackFrame *f,uint32_t entry){',
                ' if(!f)return;*f=(AE3_StackFrame){0};f->entry=entry;f->depth=1;',
                ' f->frames[0].entry=entry;f->frames[0].pc=entry;}',
                'int ae3_run_stack_slice(AE3_ScalarContext *c,AE3_StackFrame *f,uint32_t budget){',
                ' if(!c||!f||f->reserved||f->done>1||f->depth>AE3_STACK_MAX_DEPTH)return AE3_RESUME_INVALID;',
                ' if(f->done)return f->depth==0?AE3_RESUME_DONE:AE3_RESUME_INVALID;',
                ' if(!f->depth||f->frames[0].entry!=f->entry)return AE3_RESUME_INVALID;',
                ' while(budget){',
                '  AE3_StackActivation *a=&f->frames[f->depth-1u];',
                '  if(a->reserved||a->pending>3||a->leaf_active>1)return AE3_RESUME_INVALID;',
                '  if(a->leaf_active){',
                '   uint64_t before=a->leaf.retired;',
                '   int status=ae3_run_resume_slice(c,&a->leaf,budget);',
                '   uint64_t delta=a->leaf.retired-before;',
                '   if(delta>budget)return AE3_RESUME_INVALID;',
                '   a->retired+=delta;f->retired+=delta;budget-=(uint32_t)delta;',
                '   if(status!=AE3_RESUME_DONE) return status;',
                '   if((uint32_t)c->gpr[31]!=a->pc)return AE3_RESUME_INVALID;',
                '   a->leaf_active=0;a->target=0;continue;',
                '  }',
                '  int result=stack_step(c,f);if(result!=AE3_RESUME_DONE)return result;',
                '  budget--;if(f->done)return AE3_RESUME_DONE;',
                ' }',
                ' return AE3_RESUME_YIELDED;','}'])
    src.extend([f'static const uint32_t entries[{len(callers)}]={{',
                ',\n'.join(f'UINT32_C(0x{a:08X})' for a in callers),
                '};',
                'size_t ae3_stack_count(void){return sizeof(entries)/sizeof(entries[0]);}',
                'uint32_t ae3_stack_address(size_t i){return i<ae3_stack_count()?entries[i]:0;}'])
    (ROOT/'ae3_stack_leaves.c').write_text('\n'.join(src)+'\n')
    report={
        'retail_sha256':b.EXPECTED,'existing_callers':len(callers)-len(nested)-len(indirect)-len(flow),
        'new_nested_callers':len(nested),'new_jalr_callers':len(indirect),'new_branch_callers':len(flow),
        'jalr_addresses':[f'{a:08X}' for a in indirect],'callers_total':len(callers),
        'new_nested_addresses':[f'{a:08X}' for a in nested],
        'new_branch_addresses':[f'{a:08X}' for a in flow],
        'branch_caller_rejections':flow_rejected,
        'nested_direct_edges':sum(k=='call' and v in set(callers) for code in nested.values() for _,_,k,v in code),
        'stack_bound':MAX_DEPTH,'instructions_new':sum(len(code) for code in nested.values())+sum(len(code) for code in indirect.values())+sum(len(code) for code in flow.values()),
        'scope':'Bounded scalar callers with forward CFG branches. JALR supported with strict runtime whitelist of recovered call/leaf targets. Not full R5900 runtime.'
    }
    (ROOT/'stack_lift_report.json').write_text(json.dumps(report,indent=2)+'\n')
    return report

if __name__=='__main__':
    print('Recovered stack-enabled callers:',generate())