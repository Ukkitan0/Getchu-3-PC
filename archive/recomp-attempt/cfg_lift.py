#!/usr/bin/env python3
"""Lift short bounded MIPS EE leaf routines with *forward* conditional branches.

Intentional constraints: no loops, nested delay-slot control flow, calls, stores to
control registers, or 128-bit/IOP/GIF operations. Every accepted instruction is
translated, including branch and return delay slots. Refuse uncertain cases.
"""
import json
import struct
from pathlib import Path
import bulk_lift as base

ROOT = Path(__file__).resolve().parent
MAX_WORDS = 80
MAX_BRANCHES = 4


def branch(pc, word):
    op = word >> 26
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    target = pc + 4 + 4 * base.sw16(word)
    a, b = f'R[{rs}]', f'R[{rt}]'
    if op == 4: condition = f'({a} == {b})'
    elif op == 5: condition = f'({a} != {b})'
    elif op == 6 and rt == 0: condition = f'((int64_t){a} <= 0)'
    elif op == 7 and rt == 0: condition = f'((int64_t){a} > 0)'
    elif op == 1 and rt == 0: condition = f'((int64_t){a} < 0)'
    elif op == 1 and rt == 1: condition = f'((int64_t){a} >= 0)'
    else: return None
    return condition, target


def find_routine(start):
    """Safely lift a single-return, loop-free control-flow sequence.

    The instruction scan covers all sequential words through JR $ra, even
    words skipped on some paths. Branch decisions are taken before delay
    slots, and each slot executes once regardless of branch direction.
    """
    code = []
    transfers = []
    forbidden_targets = set()
    pc = start
    while len(code) < MAX_WORDS:
        if not base.valid(pc): return None
        word = base.W[(pc - base.TEXT)//4]
        if word == 0x03e00008:  # JR $ra
            if not base.valid(pc + 4): return None
            delay = base.W[(pc+4-base.TEXT)//4]
            statement = base.emit(delay)
            if statement is None: return None
            code += [(pc,word,'return',None), (pc+4,delay,'delay_return',statement)]
            if not transfers: return None
            # All transfers must remain in the function and cannot enter a
            # delay slot. Reject cycles to avoid silently adding a loop.
            if any(not start <= target <= pc or target in forbidden_targets
                   for origin,target in transfers): return None
            return code, len(transfers)

        conditional = branch(pc,word)
        op = word >> 26
        direct_jump = None
        if op == 2:
            direct_jump = ((pc+4)&0xF0000000) | ((word&0x03ffffff)<<2)
        if conditional or direct_jump is not None:
            if len(transfers)>=MAX_BRANCHES or len(code)+2>MAX_WORDS:
                return None
            target = conditional[1] if conditional else direct_jump
            if target <= pc+4: return None  # backwards/self transfer
            if not base.valid(pc+4): return None
            delay = base.W[(pc+4-base.TEXT)//4]
            statement = base.emit(delay)
            if statement is None: return None
            forbidden_targets.add(pc+4)
            transfers.append((pc,target))
            code.append((pc,word,'branch' if conditional else 'jump',
                         conditional if conditional else target))
            code.append((pc+4,delay,'delay_transfer',statement))
            pc += 8
            continue

        statement = base.emit(word)
        if statement is None:return None
        code.append((pc,word,'scalar',statement))
        pc += 4
    return None


def translate():
    results = {}
    occupied = set(pc for code in base.accepted.values() for pc, _, _ in code)
    overlap = []
    for entry in sorted(base.starts):
        if entry in occupied: continue
        found = find_routine(entry)
        if found is None: continue
        instr, count = found
        pcs = {pc for pc, _, _, _ in instr}
        if pcs & occupied:
            overlap.append(entry)
            continue
        results[entry] = (instr, count)
        occupied.update(pcs)
    return results, overlap


def write_source(routines, overlapping):
    hdr = '''#ifndef AE3_CONTROL_LEAVES_H
#define AE3_CONTROL_LEAVES_H
#include "ae3_scalar_leaves.h"
int ae3_run_control_leaf(AE3_ScalarContext *ctx, uint32_t entry);
size_t ae3_control_leaf_count(void);
uint32_t ae3_control_leaf_address(size_t index);
#endif
'''
    (ROOT / 'ae3_control_leaves.h').write_text(hdr)
    src = ['''#include "ae3_control_leaves.h"
#include <stdint.h>
/* Bounded control-flow EE scalar subset. Delay slots are always honored. */
static inline uint64_t S8(uint64_t v) {return (uint64_t)(int64_t)(int8_t)(uint8_t)v;}
static inline uint64_t S16(uint64_t v) {return (uint64_t)(int64_t)(int16_t)(uint16_t)v;}
static inline uint64_t S32(uint64_t v) {return (uint64_t)(int64_t)(int32_t)(uint32_t)v;}
static int read_mem(AE3_ScalarContext *c,uint64_t a,unsigned n,uint64_t *v){
  uint32_t off=(uint32_t)a & 0x1fffffff;
  if(!c->ram || off >= c->ram_size || n>c->ram_size-off) return -1;
  *v=0;for(unsigned i=0;i<n;i++) *v|=(uint64_t)c->ram[off+i]<<(8*i);
  return 0;
}
static int write_mem(AE3_ScalarContext *c,uint64_t a,unsigned n,uint64_t v){
  uint32_t off=(uint32_t)a & 0x1fffffff;
  if(!c->ram || off >= c->ram_size || n>c->ram_size-off) return -1;
  for(unsigned i=0;i<n;i++) c->ram[off+i]=(uint8_t)(v>>(8*i));
  return 0;
}
#define GEN_SIZE(N) \\
static int load##N(AE3_ScalarContext *c,uint64_t a,uint64_t *v){return read_mem(c,a,N,v);} \\
static int store##N(AE3_ScalarContext *c,uint64_t a,uint64_t v){return write_mem(c,a,N,v);}
GEN_SIZE(1) GEN_SIZE(2) GEN_SIZE(4) GEN_SIZE(8)
''']
    for addr, (code, branches) in routines.items():
        lines = [f'/* bounded forward-branch function 0x{addr:08X}; {branches} branches */',
                 f'static int ae3_control_{addr:08X}(AE3_ScalarContext *c) {{',
                 '  uint64_t *R=c->gpr; uint64_t tmp=0; (void)tmp;']
        counter = 0
        for pc, word, kind, arg in code:
            lines.append(f'  L_{pc:08X}: ; /* {word:08X} */')
            if kind == 'branch':
                lines.append(f'  const int take_{counter}={arg[0]};')
                # branch delay slot is the NEXT linear item
            elif kind == 'jump':
                pass
            elif kind == 'delay_transfer':
                lines.append('  ' + (arg or '/* nop */'))
                transfer_kind, transfer_arg = next((kind, a) for p,w,kind,a in code if p==pc-4)
                if transfer_kind=='branch':
                    lines.append(f'  if (take_{counter}) goto L_{transfer_arg[1]:08X};')
                    counter += 1
                else:
                    lines.append(f'  goto L_{transfer_arg:08X};')
            elif kind == 'return':
                lines.append('  /* JR $ra: next instruction is always executed before returning. */')
            elif kind == 'delay_return':
                lines.append('  ' + (arg or '/* nop */'))
                lines.append('  R[0]=0;return 0;')
            else:
                lines.append('  ' + (arg or '/* nop */'))
        lines.append('}\n')
        src.append('\n'.join(lines))
    src.append('int ae3_run_control_leaf(AE3_ScalarContext *ctx,uint32_t entry){')
    src.append('  (void)S8; (void)S16; if(!ctx)return -1;switch(entry){')
    for a in routines:
        src.append(f'    case 0x{a:08X}u: return ae3_control_{a:08X}(ctx);')
    src.append('    default:return -2;}}')
    src.append(f'static const uint32_t entries[{len(routines)}]={{')
    src.append(',\n'.join(f'0x{a:08X}u' for a in routines))
    src.append('};')
    src.append('size_t ae3_control_leaf_count(void){return sizeof(entries)/sizeof(entries[0]);}')
    src.append('uint32_t ae3_control_leaf_address(size_t i){return i<ae3_control_leaf_count()?entries[i]:0;}')
    (ROOT/'ae3_control_leaves.c').write_text('\n'.join(src))
    report = {'original_sha256':base.EXPECTED,'bounded_cfg_candidates':len(routines),
              'instruction_locations':sum(len(c) for c,_ in routines.values()),
              'transfer_distribution':{str(n):sum(n==m for _,m in routines.values()) for n in range(1,MAX_BRANCHES+1)},
              'overlapping_exclusions':[f'{x:08X}' for x in overlapping],
              'entries':[f'{x:08X}' for x in routines],
              'constraints':'Only bounded forward-branch scalar leaves; no indirect returns except JR ra; no stateful system interactions.'}
    (ROOT/'cfg_lift_report.json').write_text(json.dumps(report,indent=2))
    return report


if __name__ == '__main__':
    routines, overlapping = translate()
    report = write_source(routines, overlapping)
    print('Generated',report['bounded_cfg_candidates'],'additional native C routines',
          report['instruction_locations'],'instruction locations, branch counts',report['transfer_distribution'])