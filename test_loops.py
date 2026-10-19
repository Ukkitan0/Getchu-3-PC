#!/usr/bin/env python3
"""Differential test original R5900 instruction words against native C loop lifter.

The test executes each original instruction via the scalar reference interpreter;
branch and delay-slot scheduling are independently modeled. Execution budget
includes delay-slot words, and matches the generated C execution limit.
"""
import ctypes
import random
import sys
from pathlib import Path
import loop_lift as lifter
from test_bulk import Ctx, instr, signed

MASK = (1 << 64) - 1
RANDOM = random.Random(0xAE3EED)


def run_interpreter(entry, code, regs, mem, budget=lifter.INSTRUCTION_BUDGET):
    words={pc:word for pc,word,_,_ in code}
    pc=entry
    executed=0
    def step_word(pc):
        nonlocal executed
        if executed >= budget:
            return -3
        executed+=1
        return instr(words[pc],regs,mem)
    while True:
        if pc not in words:
            raise AssertionError(f'Bad execution target for {entry:#x}: {pc:#x}')
        word=words[pc]
        if word == 0x03e00008:
            if executed>=budget:return -3
            executed+=1  # JR $ra
            rc=step_word(pc+4)
            if rc == 0:regs[0]=0
            return rc
        op=word>>26
        rs=(word>>21)&31;rt=(word>>16)&31
        target=pc+4+4*signed(word,16)
        take=False
        is_branch=False
        if op==4:
            take=(regs[rs]==regs[rt]);is_branch=True
        elif op==5:
            take=(regs[rs]!=regs[rt]);is_branch=True
        elif op==6 and rt==0:
            take=signed(regs[rs],64)<=0;is_branch=True
        elif op==7 and rt==0:
            take=signed(regs[rs],64)>0;is_branch=True
        elif op==1 and rt==0:
            take=signed(regs[rs],64)<0;is_branch=True
        elif op==1 and rt==1:
            take=signed(regs[rs],64)>=0;is_branch=True
        if is_branch or op==2:
            if executed>=budget:return -3
            executed+=1
            rc=step_word(pc+4)
            if rc:return rc
            if op==2:pc=((pc+4)&0xf0000000)|((word&0x03ffffff)<<2)
            else:pc=target if take else pc+8
        else:
            rc=step_word(pc)
            if rc:return rc
            pc+=4


def check_one(lib, entry, code, regs, mem):
    expect_regs=regs.copy(); expect_mem=mem.copy()
    expected=run_interpreter(entry,code,expect_regs,expect_mem)
    buf=(ctypes.c_uint8*len(mem)).from_buffer(mem)
    context=Ctx((ctypes.c_uint64*32)(*regs),buf,len(mem))
    actual=lib.ae3_run_loop_leaf(ctypes.byref(context),entry)
    if actual!=expected or list(context.gpr)!=expect_regs or mem!=expect_mem:
        reg_diffs=[(r,hex(int(context.gpr[r])),hex(expect_regs[r])) for r in range(32)
                   if context.gpr[r]!=expect_regs[r]]
        mem_idx=next((i for i,(x,y) in enumerate(zip(mem,expect_mem)) if x!=y),-1)
        raise AssertionError(f'{entry:08X}: return {actual}/{expected}; regs {reg_diffs[:5]} memory {mem_idx}')
    return expected


def run(trials=8):
    lib=ctypes.CDLL(str(Path(__file__).with_name('libae3_loop.so')))
    lib.ae3_run_loop_leaf.argtypes=[ctypes.POINTER(Ctx),ctypes.c_uint32]
    lib.ae3_run_loop_leaf.restype=ctypes.c_int
    lib.ae3_loop_leaf_count.restype=ctypes.c_size_t
    routines,_=lifter.translate()
    assert len(routines)==lib.ae3_loop_leaf_count()
    tested=0; completed=0; exhausted=0; faults=0
    for entry,(code,nloops) in routines.items():
        for t in range(trials):
            regs=[RANDOM.getrandbits(64) if t%3==0 else RANDOM.randrange(0,65536) for _ in range(32)]
            regs[0]=0; regs[29]=RANDOM.randrange(32000,59000)
            if t%2==1:
                for pc,w,kind,arg in code:
                    if kind!='branch':continue
                    rs=(w>>21)&31; rt=(w>>16)&31
                    if (w>>26) in (4,5) and rs and rt:
                        regs[rt]=regs[rs]
                        break
            mem=bytearray(RANDOM.randbytes(65536))
            outcome=check_one(lib,entry,code,regs,mem)
            completed+=outcome==0
            exhausted+=outcome==-3
            faults+=outcome==-1
            tested+=1
    assert tested and exhausted, 'loop test should exercise non-terminating cases'
    print(f'PASS {tested} original-MIPS vs native-C loop comparisons over {len(routines)} candidates;'
          f' {completed} completed, {exhausted} hit budget, {faults} memory faults')

if __name__=='__main__':run(int(sys.argv[1]) if len(sys.argv)>1 else 8)