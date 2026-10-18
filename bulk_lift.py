#!/usr/bin/env python3
"""Conservative native-C EE leaf lifter, NEVER treats unknown opcodes as no-ops.
Only direct-call candidate targets; excludes any unknown instructions or nested control flow.
Requires original US retail ELF; output contains user-owned translated code.
"""
import hashlib,json,struct,os
from pathlib import Path
ROOT=Path(__file__).resolve().parent
INPUT=Path(os.environ.get('AE3_ELF', str(ROOT/'SCUS_975.01')))
if not INPUT.is_file():
 raise SystemExit('Original SCUS_975.01 missing: set AE3_ELF to its path')
ELF=INPUT.read_bytes()
EXPECTED='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
if hashlib.sha256(ELF).hexdigest()!=EXPECTED: raise SystemExit('Wrong game revision or corrupt ELF; refused')
TEXT=0x100000; LEN=0x528258; CODE=ELF[0x1000:0x1000+LEN]
W=struct.unpack('<%dI'%(len(CODE)//4),CODE)
def valid(pc): return TEXT<=pc<TEXT+LEN and not(pc&3)
def sw16(n): return (n&65535)-((n&32768)<<1)
def emit(w):
 op=w>>26; rs=(w>>21)&31;rt=(w>>16)&31;rd=(w>>11)&31;sa=(w>>6)&31;fn=w&63; imm=sw16(w)
 a=f'R[{rs}]'; b=f'R[{rt}]'; shift=f'{sa}'
 def setr(d,rhs):return '' if d==0 else f'R[{d}] = {rhs};'
 if w==0:return '/* nop */'
 if op==0:
  if fn==0:return setr(rd,f'S32((uint32_t)({b}) << {shift})')
  if fn==2:return setr(rd,f'S32((uint32_t)({b}) >> {shift})')
  if fn==3:return setr(rd,f'S32((uint32_t)((int32_t){b} >> {shift}))')
  if fn==4:return setr(rd,f'S32((uint32_t){b} << ((uint32_t){a} & 31))')
  if fn==6:return setr(rd,f'S32((uint32_t){b} >> ((uint32_t){a} & 31))')
  if fn==7:return setr(rd,f'S32((uint32_t)((int32_t){b} >> ((uint32_t){a} & 31)))')
  if fn==10:return '' if rd==0 else f'if (!{b}) R[{rd}]=R[{rs}];'
  if fn==11:return '' if rd==0 else f'if ({b}) R[{rd}]=R[{rs}];'
  if fn==33:return setr(rd,f'S32((uint32_t){a} + (uint32_t){b})')
  if fn==35:return setr(rd,f'S32((uint32_t){a} - (uint32_t){b})')
  if fn==36:return setr(rd,f'({a} & {b})')
  if fn==37:return setr(rd,f'({a} | {b})')
  if fn==38:return setr(rd,f'({a} ^ {b})')
  if fn==39:return setr(rd,f'~({a} | {b})')
  if fn==42:return setr(rd,f'((int64_t){a} < (int64_t){b})')
  if fn==43:return setr(rd,f'({a} < {b})')
  if fn==45:return setr(rd,f'({a} + {b})')
  if fn==47:return setr(rd,f'({a} - {b})')
  if fn==56:return setr(rd,f'({b} << {shift})')
  if fn==58:return setr(rd,f'({b} >> {shift})')
  if fn==59:return setr(rd,f'(uint64_t)((int64_t){b} >> {shift})')
  if fn==60:return setr(rd,f'({b} << {sa+32})')
  if fn==62:return setr(rd,f'({b} >> {sa+32})')
  if fn==63:return setr(rd,f'(uint64_t)((int64_t){b} >> {sa+32})')
  return None
 if op==9:return setr(rt,f'S32((uint32_t){a} + UINT32_C({imm&0xffffffff}))')
 if op==25:return setr(rt,f'({a} + UINT64_C({imm&0xffffffffffffffff}))')
 if op==10:return setr(rt,f'((int64_t){a} < INT64_C({imm}))')
 if op==11:return setr(rt,f'({a} < UINT64_C({imm&0xffffffffffffffff}))')
 if op==12:return setr(rt,f'({a} & UINT64_C({w&0xffff}))')
 if op==13:return setr(rt,f'({a} | UINT64_C({w&0xffff}))')
 if op==14:return setr(rt,f'({a} ^ UINT64_C({w&0xffff}))')
 if op==15:return setr(rt,f'S32(UINT32_C({(w&0xffff)<<16}))')
 sizes={32:(1,True),33:(2,True),35:(4,True),36:(1,False),37:(2,False),39:(4,False),55:(8,False)}
 if op in sizes:
  sz,signed=sizes[op]
  rhs=f'load{sz}(c, {a} + UINT64_C({imm&0xffffffffffffffff}), &tmp)'
  expr= f'S{sz*8}(tmp)' if signed and sz<8 else f'tmp'
  return f'if ({rhs}) return -1; '+setr(rt,expr)
 sizes={40:1,41:2,43:4,63:8}
 if op in sizes:
  sz=sizes[op]
  return f'if (store{sz}(c, {a} + UINT64_C({imm&0xffffffffffffffff}), {b})) return -1;'
 return None

def find_starts():
 starts={0x100134}
 for i,w in enumerate(W):
  if w>>26==3:
   pc=TEXT+4*i; target=((pc+4)&0xf0000000)|((w&0x03ffffff)<<2)
   if valid(target): starts.add(target)
 for lo,hi in [(0x62e660,0x62e8a4),(0x62e8a4,0x62e958)]:
  for p in range(lo,hi,4):
   a=struct.unpack_from('<I',ELF,p-TEXT+0x1000)[0]
   if valid(a): starts.add(a)
 return starts

def leaf(address, max_words=48):
 if not valid(address):return None
 ins=[]
 for i in range(max_words):
  pc=address+i*4
  if not valid(pc):return None
  w=W[(pc-TEXT)//4]
  if w==0x03e00008: # jr ra, then delay instruction
   pc2=pc+4
   if not valid(pc2):return None
   w2=W[(pc2-TEXT)//4]
   stmt=emit(w2)
   if stmt is None:return None
   ins.append((pc2,stmt,w2))
   return ins
  stmt=emit(w)
  if stmt is None:return None
  ins.append((pc,stmt,w))
 return None

starts=find_starts(); found={a:x for a in sorted(starts) if (x:=leaf(a)) is not None};
# Remove candidate entries that start inside a previously accepted leaf. Any two
# nearby leaf start addresses may be real tail-call entries, so record exclusions.
accepted={};overlap=[]; occupied=set()
for a,code in sorted(found.items(),key=lambda e:(e[0])):
 locations={p for p,_,_ in code};
 if locations.intersection(occupied):overlap.append(a);continue
 accepted[a]=code;occupied.update(locations)
print('Candidate direct-call / ctor entrypoints',len(starts),'bounded straight-line leaf candidates',len(found),'non-overlapping',len(accepted),'instructions',len(occupied))
print('first 20:',[hex(a) for a in list(accepted)[:20]])

# Source library interface; the reg file preserves full lower 64 bits of PS2 EE GPRs.
# High 64 bits of each R5900 GPR are intentionally OUT OF SCOPE here.
header='''#ifndef AE3_SCALAR_LEAVES_H
#define AE3_SCALAR_LEAVES_H
#include <stdint.h>
#include <stddef.h>
typedef struct AE3_ScalarContext {
    uint64_t gpr[32];
    uint8_t *ram;
    uint32_t ram_size;
} AE3_ScalarContext;
/* Return: 0 executed, -1 unmapped memory, -2 not among verified scalar leaf candidates. */
int ae3_run_scalar_leaf(AE3_ScalarContext *ctx, uint32_t entry);
size_t ae3_scalar_leaf_count(void);
uint32_t ae3_scalar_leaf_address(size_t index);
#endif
'''
(ROOT/'ae3_scalar_leaves.h').write_text(header)
parts=['''#include "ae3_scalar_leaves.h"
#include <limits.h>
/* MIPS EE low 64-bit GPR scalar subset. No 128-bit/MMI/VU/COP support. */
static inline uint64_t S8(uint64_t v) { return (uint64_t)(int64_t)(int8_t)(uint8_t)v; }
static inline uint64_t S16(uint64_t v) { return (uint64_t)(int64_t)(int16_t)(uint16_t)v; }
static inline uint64_t S32(uint64_t v) { return (uint64_t)(int64_t)(int32_t)(uint32_t)v; }
static int read_mem(const AE3_ScalarContext *c,uint64_t address, unsigned n, uint64_t *v) {
  /* The EE's cached/uncached address aliases map down to physical RAM. */
  uint32_t off=(uint32_t)address & UINT32_C(0x1fffffff);
  if (!c->ram || off >= c->ram_size || n>c->ram_size-off) return -1;
  *v=0;
  for (unsigned i=0;i<n;i++) *v |= (uint64_t)c->ram[off+i]<<(i*8);
  return 0;
}
static int write_mem(AE3_ScalarContext *c,uint64_t address,unsigned n,uint64_t v) {
  uint32_t off=(uint32_t)address & UINT32_C(0x1fffffff);
  if (!c->ram || off >= c->ram_size || n>c->ram_size-off) return -1;
  for (unsigned i=0;i<n;i++) c->ram[off+i]=(uint8_t)(v>>(i*8));
  return 0;
}
#define GEN_SIZE(N) \\
 static int load##N(const AE3_ScalarContext *c,uint64_t a,uint64_t *v){return read_mem(c,a,N,v);} \\
 static int store##N(AE3_ScalarContext *c,uint64_t a,uint64_t v){return write_mem(c,a,N,v);}
GEN_SIZE(1) GEN_SIZE(2) GEN_SIZE(4) GEN_SIZE(8)
''']
for a,code in accepted.items():
 lines=[f'/* Translated original EE {a:#010x}, {len(code)} scalar instructions including JR delay slot. */',f'static int ae3_leaf_{a:08X}(AE3_ScalarContext *c) {{','  uint64_t *R = c->gpr;', '  uint64_t tmp = 0; (void)tmp;']
 for pc,stmt,w in code:
  lines.append(f'  /* {pc:08X}: {w:08X} */ {stmt}')
 lines.append('  R[0] = 0; return 0;\n}')
 parts.append('\n'.join(lines))
parts.append('''int ae3_run_scalar_leaf(AE3_ScalarContext *ctx,uint32_t entry) {
  (void)S8; (void)S16;
  if (!ctx) return -1;
  switch(entry) {''')
for a in accepted:parts.append(f'    case UINT32_C(0x{a:08X}): return ae3_leaf_{a:08X}(ctx);')
parts.append('''    default: return -2;
  }
}''')
parts.append(f'''static const uint32_t entrylist[{len(accepted)}]={{''')
parts.append(',\n'.join('  UINT32_C(0x%08X)'%a for a in accepted))
parts.append('''};
size_t ae3_scalar_leaf_count(void){return sizeof(entrylist)/sizeof(entrylist[0]);}
uint32_t ae3_scalar_leaf_address(size_t i){return i < ae3_scalar_leaf_count()? entrylist[i] : 0;}
''')
(ROOT/'ae3_scalar_leaves.c').write_text('\n'.join(parts))
metadata={'input_sha256':hashlib.sha256(ELF).hexdigest(),'direct_call_candidates':len(starts),'bounded_scalar_leaf_candidates':len(found),'nonoverlapping_scalar_leaf_functions':len(accepted),'machine_instruction_count':len(occupied),'generated_function_entries':sorted(accepted),'excluded_overlapping_candidate_entries':overlap,'excluded_functional_scope':'R5900 128-bit registers, branches inside functions, indirect calls, COP/MMI/VU and kernel/IOP services are NOT ported','note':'Verified byte exact opcodes, not yet behavior validated'}
(ROOT/'lift_report.json').write_text(json.dumps(metadata,indent=2))