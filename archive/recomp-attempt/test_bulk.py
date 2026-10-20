#!/usr/bin/env python3
"""Cross-check bulk translated scalar leaf routines against a separate MIPS decoder."""
import ctypes,random,struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).parent))
import bulk_lift as lifter
random.seed(0xAE32026)
MASK=(1<<64)-1
def signed(x,b):
 x&=(1<<b)-1
 return x-(1<<b) if x&(1<<(b-1)) else x
def sx32(x):return signed(x,32)&MASK
def instr(w,R,M):
 op=w>>26; rs=(w>>21)&31; rt=(w>>16)&31;rd=(w>>11)&31;sh=(w>>6)&31; fn=w&63;imm=signed(w,16)
 a=R[rs]; b=R[rt]
 def put(r,v):
  if r:R[r]=v&MASK
 if w==0:return 0
 if op==0:
  val=None
  if fn==0:val=sx32(b<<sh)
  elif fn==2:val=sx32((b&0xffffffff)>>sh)
  elif fn==3:val=sx32(signed(b,32)>>sh)
  elif fn==4:val=sx32(b<<(a&31))
  elif fn==6:val=sx32((b&0xffffffff)>>(a&31))
  elif fn==7:val=sx32(signed(b,32)>>(a&31))
  elif fn==10:
   if not b:put(rd,a)
   return 0
  elif fn==11:
   if b:put(rd,a)
   return 0
  elif fn==33:val=sx32(a+b)
  elif fn==35:val=sx32(a-b)
  elif fn==36:val=a&b
  elif fn==37:val=a|b
  elif fn==38:val=a^b
  elif fn==39:val=~(a|b)
  elif fn==42:val=int(signed(a,64)<signed(b,64))
  elif fn==43:val=int(a<b)
  elif fn==45:val=a+b
  elif fn==47:val=a-b
  elif fn==56:val=b<<sh
  elif fn==58:val=b>>sh
  elif fn==59:val=signed(b,64)>>sh
  elif fn==60:val=b<<(sh+32)
  elif fn==62:val=b>>(sh+32)
  elif fn==63:val=signed(b,64)>>(sh+32)
  else:raise ValueError(hex(w))
  put(rd,val);return 0
 if op==9:put(rt,sx32(a+imm));return 0
 if op==25:put(rt,a+imm);return 0
 if op==10:put(rt,int(signed(a,64)<imm));return 0
 if op==11:put(rt,int(a<(imm&MASK)));return 0
 if op==12:put(rt,a&(w&65535));return 0
 if op==13:put(rt,a|(w&65535));return 0
 if op==14:put(rt,a^(w&65535));return 0
 if op==15:put(rt,sx32((w&65535)<<16));return 0
 if op in (32,33,35,36,37,39,55,40,41,43,63):
  size={32:1,33:2,35:4,36:1,37:2,39:4,55:8,40:1,41:2,43:4,63:8}[op]
  addr=(a+imm)&0x1fffffff
  if addr+size>len(M):return -1
  if op in (40,41,43,63):
   M[addr:addr+size]=int(b & ((1<<(size*8))-1)).to_bytes(size,'little')
  else:
   v=int.from_bytes(M[addr:addr+size],'little')
   if op in (32,33,35):v=signed(v,size*8)&MASK
   put(rt,v)
  return 0
 raise ValueError(hex(w))

class Ctx(ctypes.Structure):
 _fields_=[('gpr',ctypes.c_uint64*32),('ram',ctypes.POINTER(ctypes.c_uint8)),('ram_size',ctypes.c_uint32)]
if __name__=='__main__':
 dll=ctypes.CDLL(str(Path(__file__).with_name('libae3_scalar.so')))
 dll.ae3_run_scalar_leaf.argtypes=[ctypes.POINTER(Ctx),ctypes.c_uint32]
 dll.ae3_run_scalar_leaf.restype=ctypes.c_int
 dll.ae3_scalar_leaf_count.restype=ctypes.c_size_t
 assert dll.ae3_scalar_leaf_count()==len(lifter.accepted)
 trials=int(sys.argv[1]) if len(sys.argv)>1 else 7
 passed=0
 for a,items in lifter.accepted.items():
  for it in range(trials):
   R=[random.getrandbits(64) if it%3==0 else random.randrange(1,64000) for _ in range(32)]
   R[0]=0;R[29]=random.randrange(32000,60000)
   M=bytearray(random.randbytes(65536))
   eR=R.copy(); eM=M.copy()
   expected=0
   for pc,ignored,w in items:
    expected=instr(w,eR,eM)
    if expected:break
   eR[0]=0 if expected==0 else eR[0]
   buf=(ctypes.c_uint8*len(M)).from_buffer(M)
   ctx=Ctx((ctypes.c_uint64*32)(*R),buf,len(M))
   actual=dll.ae3_run_scalar_leaf(ctypes.byref(ctx),a)
   if actual!=expected or list(ctx.gpr)!=eR or M!=eM:
    differing=[(i,hex(ctx.gpr[i]),hex(v)) for i,v in enumerate(eR) if v!=ctx.gpr[i]]
    loc=next((i for i,(m,e) in enumerate(zip(M,eM)) if m!=e),-1)
    print('FAIL',hex(a),'trial',it,'results',actual,expected,'regs',differing[:5],'memory diff',loc,'instructions',[(hex(pc),hex(w)) for pc,_,w in items]);exit(1)
   passed+=1
 print('PASS',passed,'independent native C vs original-word MIPS test executions,',len(lifter.accepted),'lifted candidates')