#!/usr/bin/env python3
"""Generate PC-resumable native-C execution for the verified scalar candidate subset.

Each instruction (including *individual* MIPS branch delay slots) consumes one
scheduler step. The guest PC and previously decided branch target persist across
slice boundaries. Unsupported instructions and non-candidate addresses are
rejected, never silently emulated or treated as no-ops.

This is NOT a complete R5900 execution runtime.
"""
from __future__ import annotations
import json
from pathlib import Path
import bulk_lift as scalar
import cfg_lift as forward
import loop_lift as backward

ROOT = Path(__file__).resolve().parent


def collect():
    candidates = {}
    occupied = set()
    counts = {}
    for name, group in (
        ('scalar', {a: (v, 0) for a, v in scalar.accepted.items()}),
        ('forward', forward.translate()[0]),
        ('loop', backward.translate()[0]),
    ):
        n = 0
        for entry, (items, _) in sorted(group.items()):
            if name == 'scalar':
                # Original straight-line collector drops JR ra, retaining the
                # return delay slot as the last element. Reinsert JR itself.
                end = items[-1][0]
                assert scalar.W[(end - 4 - scalar.TEXT)//4] == 0x03e00008
                body = [(pc, w, 'scalar', stmt) for pc, stmt, w in items[:-1]]
                body += [(end-4, 0x03e00008, 'return', None),
                         (end, items[-1][2], 'delay_return', items[-1][1])]
            else:
                body = items
            addresses = [pc for pc, _, _, _ in body]
            if len(set(addresses)) != len(addresses):
                raise RuntimeError(f'duplicate instruction address in {entry:08X}')
            if set(addresses) & occupied:
                raise RuntimeError(f'overlapping candidates in {entry:08X}')
            # All branch instructions must immediately precede their valid
            # delay slot. No label target may enter a delay slot.
            slots = {pc for pc, _, kind, _ in body if kind.startswith('delay_')}
            for idx, (pc, _, kind, value) in enumerate(body):
                if kind not in ('jump', 'branch', 'return'): continue
                if idx + 1 >= len(body) or body[idx+1][0] != pc+4:
                    raise RuntimeError(f'bad delay at {pc:08X}')
                if not body[idx+1][2].startswith('delay_'):
                    raise RuntimeError(f'missing delay at {pc:08X}')
                if kind in ('branch', 'jump'):
                    dest = value[1] if kind == 'branch' else value
                    if dest not in addresses or dest in slots:
                        raise RuntimeError(f'invalid control target {dest:08X}')
            candidates[entry] = (body, name)
            occupied.update(addresses)
            n += 1
        counts[name] = n
    return dict(sorted(candidates.items())), counts


HEADER = '''#ifndef AE3_RESUMABLE_LEAVES_H
#define AE3_RESUMABLE_LEAVES_H
#include "ae3_scalar_leaves.h"
/* Scheduler results: complete=0, paused=1, memory fault=-1,
   unknown candidate=-2, corrupt PC/delay state=-4. */
#define AE3_RESUME_DONE 0
#define AE3_RESUME_YIELDED 1
#define AE3_RESUME_MEMORY_FAULT (-1)
#define AE3_RESUME_UNKNOWN (-2)
#define AE3_RESUME_INVALID (-4)
typedef struct AE3_ResumeFrame {
    uint32_t entry;
    uint32_t pc;
    uint32_t next_pc;
    uint32_t pending; /* 0 normal, 1 branch delay, 2 return delay */
    uint64_t retired; /* successfully executed guest words, includes slots */
    uint32_t done;
    uint32_t reserved;
} AE3_ResumeFrame;
void ae3_resume_init(AE3_ResumeFrame *frame, uint32_t entry);
int ae3_run_resume_slice(AE3_ScalarContext *ctx, AE3_ResumeFrame *frame,
                        uint32_t instruction_budget);
size_t ae3_resume_count(void);
uint32_t ae3_resume_address(size_t index);
#endif
'''

# Match the conservative memory alias model used by all existing native lifts.
PREAMBLE = '''#include "ae3_resumable_leaves.h"
#include <stdint.h>
static inline uint64_t S8(uint64_t v) {return (uint64_t)(int64_t)(int8_t)(uint8_t)v;}
static inline uint64_t S16(uint64_t v) {return (uint64_t)(int64_t)(int16_t)(uint16_t)v;}
static inline uint64_t S32(uint64_t v) {return (uint64_t)(int64_t)(int32_t)(uint32_t)v;}
static int read_mem(const AE3_ScalarContext *c, uint64_t a, unsigned n, uint64_t *v){
 uint32_t off=(uint32_t)a & UINT32_C(0x1fffffff);
 if(!c->ram || off>=c->ram_size || n>c->ram_size-off) return -1;
 *v=0; for(unsigned i=0;i<n;i++) *v |= (uint64_t)c->ram[off+i]<<(8*i);
 return 0;
}
static int write_mem(AE3_ScalarContext *c, uint64_t a, unsigned n, uint64_t v){
 uint32_t off=(uint32_t)a & UINT32_C(0x1fffffff);
 if(!c->ram || off>=c->ram_size || n>c->ram_size-off) return -1;
 for(unsigned i=0;i<n;i++) c->ram[off+i]=(uint8_t)(v>>(8*i));
 return 0;
}
#define GEN_SIZE(N) \\
static int load##N(AE3_ScalarContext *c,uint64_t a,uint64_t *v){return read_mem(c,a,N,v);} \\
static int store##N(AE3_ScalarContext *c,uint64_t a,uint64_t v){return write_mem(c,a,N,v);}
GEN_SIZE(1) GEN_SIZE(2) GEN_SIZE(4) GEN_SIZE(8)
'''


def generate():
    routines, counts = collect()
    (ROOT/'ae3_resumable_leaves.h').write_text(HEADER)
    c = [PREAMBLE]
    for entry, (code, category) in routines.items():
        c.extend([f'/* {category} original MIPS candidate entry 0x{entry:08X} */',
                  f'static int resume_{entry:08X}(AE3_ScalarContext *c, AE3_ResumeFrame *f, uint32_t budget) {{',
                  ' uint64_t *R=c->gpr; uint64_t tmp=0; (void)tmp;',
                  ' while(budget) {', '  switch(f->pc) {'])
        for pc, word, kind, arg in code:
            c.append(f'  case UINT32_C(0x{pc:08X}): {{ /* {word:08X} {kind} */')
            c.append('   if(f->pending != %du) return AE3_RESUME_INVALID;' % (
                1 if kind=='delay_transfer' else 2 if kind=='delay_return' else 0))
            if kind == 'branch':
                c.append(f'   f->next_pc = {arg[0]} ? UINT32_C(0x{arg[1]:08X}) : UINT32_C(0x{pc+8:08X});')
                c.append(f'   f->pending=1; f->pc=UINT32_C(0x{pc+4:08X});')
            elif kind == 'jump':
                c.append(f'   f->next_pc=UINT32_C(0x{arg:08X}); f->pending=1; f->pc=UINT32_C(0x{pc+4:08X});')
            elif kind == 'return':
                c.append(f'   f->pending=2; f->pc=UINT32_C(0x{pc+4:08X});')
            elif kind == 'delay_transfer':
                c.append(f'   {arg or "/* nop */"}')
                c.append('   f->pc=f->next_pc; f->pending=0; f->next_pc=0;')
            elif kind == 'delay_return':
                c.append(f'   {arg or "/* nop */"}')
                c.append('   f->pc=0; f->pending=0; f->next_pc=0; f->done=1;')
            else:
                c.append(f'   {arg or "/* nop */"}')
                c.append(f'   f->pc=UINT32_C(0x{pc+4:08X});')
            c.append('   break; }')
        c.extend(['  default: return AE3_RESUME_INVALID;',
                  '  }',
                  '  c->gpr[0]=0; f->retired++; budget--;',
                  '  if(f->done) return AE3_RESUME_DONE;',
                  ' }', ' return AE3_RESUME_YIELDED;', '}'])
    c.extend(['void ae3_resume_init(AE3_ResumeFrame *f,uint32_t entry){',
              ' if(!f)return; f->entry=entry; f->pc=entry; f->next_pc=0;',
              ' f->pending=0;f->retired=0;f->done=0;f->reserved=0;}',
              'int ae3_run_resume_slice(AE3_ScalarContext *c,AE3_ResumeFrame *f,uint32_t budget){',
              ' if(!c || !f) return AE3_RESUME_INVALID;',
              ' if(f->done>1 || f->pending>2 || f->reserved) return AE3_RESUME_INVALID;',
              ' if(f->done)return (f->pc==0 && f->pending==0 && f->next_pc==0) ? AE3_RESUME_DONE : AE3_RESUME_INVALID;',
                            ' switch(f->entry){'])
    for addr in routines:
        c.append(f' case UINT32_C(0x{addr:08X}): return resume_{addr:08X}(c,f,budget);')
    c.extend([' default:return AE3_RESUME_UNKNOWN;', ' }', '}'])
    c.extend([f'static const uint32_t entries[{len(routines)}]={{',
              ',\n'.join(f' UINT32_C(0x{a:08X})' for a in routines),
              '};',
              'size_t ae3_resume_count(void){return sizeof(entries)/sizeof(entries[0]);}',
              'uint32_t ae3_resume_address(size_t i){return i<ae3_resume_count()?entries[i]:0;}'])
    (ROOT/'ae3_resumable_leaves.c').write_text('\n'.join(c)+'\n')
    report = dict(game_sha256=scalar.EXPECTED, candidate_count=len(routines),
                  category_counts=counts,
                  represented_original_instruction_locations=sum(len(code) for code,_ in routines.values()),
                  guarantees='One instruction per step, deferred branch/return slot, resumable budget and memory fault; scalar low-64-bit only.',
                  limitations='No cross-function calls, full 128-bit EE, FPU/COP/MMI/VU/IOP/GIF/GS. No playable game runtime.')
    (ROOT/'resume_lift_report.json').write_text(json.dumps(report,indent=2)+'\n')
    return report

if __name__=='__main__':
    info=generate()
    print(f'Generated resumable C for {info["candidate_count"]} candidates and '
          f'{info["represented_original_instruction_locations"]} instruction locations')