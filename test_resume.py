#!/usr/bin/env python3
"""Compare every PC-sliced native C candidate with a byte-by-byte original-word interpreter.

Checks snapshots at *every* instruction boundary, including between branches and
slots, and across independent slice budgets. Also tests memory fault retry,
corrupt state rejection, repeat completion, and zero-budget behavior.
"""
from __future__ import annotations
import ctypes
import random
import sys
from pathlib import Path
import resume_lift as lifter
from test_bulk import Ctx, instr, signed

rng = random.Random(0x3A5E0B)
MASK = (1<<64)-1


class Frame(ctypes.Structure):
    _fields_=[('entry',ctypes.c_uint32),('pc',ctypes.c_uint32),
              ('next_pc',ctypes.c_uint32),('pending',ctypes.c_uint32),
              ('retired',ctypes.c_uint64),('done',ctypes.c_uint32),
              ('reserved',ctypes.c_uint32)]


def branch_target(pc,w,R):
    op=w>>26
    rs=(w>>21)&31; rt=(w>>16)&31
    a=R[rs]; b=R[rt]
    if op==2:
        return ((pc+4)&0xF0000000)|((w&0x03FFFFFF)<<2)
    if op==4: take=(a==b)
    elif op==5:take=(a!=b)
    elif op==6:take=(signed(a,64)<=0)
    elif op==7:take=(signed(a,64)>0)
    elif op==1 and rt==0:take=(signed(a,64)<0)
    elif op==1 and rt==1:take=(signed(a,64)>=0)
    else:return None
    return pc+4+4*signed(w,16) if take else pc+8


def oracle_word(frame, words, R, M):
    if frame['done']:return 0
    pc=frame['pc']; w=words[pc]
    if frame['pending']:
        rc=instr(w,R,M)
        if rc:return rc
        if frame['pending']==2:
            frame['pc']=0;frame['done']=1
        else:
            frame['pc']=frame['next_pc']
        frame['next_pc']=0;frame['pending']=0
    elif w==0x03E00008:
        frame['pending']=2;frame['pc']=pc+4
    else:
        target=branch_target(pc,w,R)
        if target is not None:
            frame['next_pc']=target;frame['pending']=1;frame['pc']=pc+4
        else:
            rc=instr(w,R,M)
            if rc:return rc
            frame['pc']=pc+4
    R[0]=0
    frame['retired']+=1
    return 0


def simulate(frame, words, R, M, budget):
    if frame['done']:return 0
    for _ in range(budget):
        rc=oracle_word(frame,words,R,M)
        if rc:return rc
        if frame['done']:return 0
    return 1


def compare(f,R,M,expect,ER,EM,entry,trial,iteration,status,exp_status):
    state={k:getattr(f,k) for k in ('entry','pc','next_pc','pending','retired','done','reserved')}
    if status!=exp_status or state!=expect or list(R)!=ER or M!=EM:
        diff=[(i,hex(int(R[i])),hex(ER[i])) for i in range(32) if R[i]!=ER[i]]
        mismatch=next((i for i,(a,b) in enumerate(zip(M,EM)) if a!=b),-1)
        raise AssertionError(f'{entry:08X} trial={trial} slice={iteration} status={status}/{exp_status} '
                             f'frame={state}/{expect} regs={diff[:4]} memory={mismatch}')


def run(trials=2, limit=72):
    lib=ctypes.CDLL(str(Path(__file__).with_name('libae3_resumable.so')))
    lib.ae3_resume_init.argtypes=[ctypes.POINTER(Frame),ctypes.c_uint32]
    lib.ae3_run_resume_slice.argtypes=[ctypes.POINTER(Ctx),ctypes.POINTER(Frame),ctypes.c_uint32]
    lib.ae3_run_resume_slice.restype=ctypes.c_int
    lib.ae3_resume_count.restype=ctypes.c_size_t
    routines,counts=lifter.collect()
    assert lib.ae3_resume_count()==len(routines)
    counts_observed=dict(done=0, yielded=0, faults=0, between_slots=0, instructions=0, cases=0)
    for entry,(code,group) in routines.items():
        words={pc:w for pc,w,kind,arg in code}
        for trial in range(trials if group!='loop' else trials+1):
            values=[rng.getrandbits(64) if trial%3==0 else rng.randrange(0,65536) for _ in range(32)]
            values[0]=0;values[29]=rng.randrange(28000,60000)
            if trial&1:
                # Force equal registers in early equality branches.
                for pc,w,kind,_ in code:
                    if kind!='branch':continue
                    rs=(w>>21)&31;rt=(w>>16)&31
                    if (w>>26) in (4,5) and rs and rt:values[rt]=values[rs]
                    break
            memory=bytearray(rng.randbytes(65536)); exp_memory=memory.copy()
            exp_regs=values.copy()
            buf=(ctypes.c_uint8*len(memory)).from_buffer(memory)
            context=Ctx((ctypes.c_uint64*32)(*values),buf,len(memory))
            frame=Frame();lib.ae3_resume_init(ctypes.byref(frame),entry)
            expected={key:getattr(frame,key) for key,_ in Frame._fields_}
            iterations=0
            while expected['retired']<limit:
                slices=[0,1,2,3,7,1,5,4]
                budget=slices[iterations%len(slices)]
                result=lib.ae3_run_resume_slice(ctypes.byref(context),ctypes.byref(frame),budget)
                exp_status=simulate(expected,words,exp_regs,exp_memory,budget)
                compare(frame,context.gpr,memory,expected,exp_regs,exp_memory,entry,trial,iterations,result,exp_status)
                counts_observed['instructions']+=budget if exp_status==1 else 0
                counts_observed['between_slots']+=(frame.pending!=0)
                counts_observed['cases']+=1
                if result==0:counts_observed['done']+=1
                elif result==-1:counts_observed['faults']+=1
                else:counts_observed['yielded']+=1
                iterations+=1
                if result<=0: break
                if iterations>150: raise AssertionError('Unexpected iteration limit')
            # Completed execution is idempotent (including a zero-budget call).
            if frame.done:
                assert lib.ae3_run_resume_slice(ctypes.byref(context),ctypes.byref(frame),0)==0
    # Invalid program counters and inconsistent branch/slot frames are rejected.
    example=next(iter(routines)); ctx=Ctx((ctypes.c_uint64*32)(),None,0)
    bad=Frame();lib.ae3_resume_init(ctypes.byref(bad),example)
    bad.pc=0xDEADC0DE
    assert lib.ae3_run_resume_slice(ctypes.byref(ctx),ctypes.byref(bad),1)==-4
    lib.ae3_resume_init(ctypes.byref(bad),example)
    bad.reserved=1
    assert lib.ae3_run_resume_slice(ctypes.byref(ctx),ctypes.byref(bad),1)==-4
    lib.ae3_resume_init(ctypes.byref(bad),0xFFFFFFFF)
    assert lib.ae3_run_resume_slice(ctypes.byref(ctx),ctypes.byref(bad),1)==-2
    # A memory fault must leave the faulting PC intact, then permit retry once
    # the memory mapping has been fixed; not a destructive pseudo-return.
    memops={32,33,35,36,37,39,55,40,41,43,63}
    memory_entry=next(a for a,(body,_) in routines.items()
                      if body[0][1]>>26 in memops)
    words={pc:w for pc,w,_,_ in routines[memory_entry][0]}
    ram=bytearray(65536)
    ram[20:24]=(0x76543210).to_bytes(4,'little')
    mapped=(ctypes.c_uint8*len(ram)).from_buffer(ram)
    context=Ctx((ctypes.c_uint64*32)(),mapped,0)
    frame=Frame(); lib.ae3_resume_init(ctypes.byref(frame),memory_entry)
    assert lib.ae3_run_resume_slice(ctypes.byref(context),ctypes.byref(frame),1)==-1
    assert frame.pc==memory_entry and frame.retired==0 and frame.pending==0
    context.ram_size=len(ram)
    expect={key:getattr(frame,key) for key,_ in Frame._fields_}
    ER=[0]*32; EM=ram.copy()
    actual=lib.ae3_run_resume_slice(ctypes.byref(context),ctypes.byref(frame),1)
    desired=simulate(expect,words,ER,EM,1)
    compare(frame,context.gpr,ram,expect,ER,EM,memory_entry,'retry',0,actual,desired)
    counts_observed['retry_memory_fault_passed']=1
    assert counts_observed['between_slots']>0
    assert counts_observed['yielded']>0
    assert counts_observed['done']>0
    assert counts_observed['faults']>0
    print('PASS resumable scalar native C vs original MIPS',len(routines),'candidates',
          counts_observed,'subsets',counts)

if __name__=='__main__':run(int(sys.argv[1]) if len(sys.argv)>1 else 2,
                           int(sys.argv[2]) if len(sys.argv)>2 else 72)