#!/usr/bin/env python3
"""Force both sides of an actual retail BEQ/JALR caller and verify sliced native C.

Tests the recovered routine at 0x3817d0, with a valid virtual callee on its
not-taken path and a direct return on its taken path. The comparison oracle
interprets original instruction words; no generated C is used as the oracle.
"""
from __future__ import annotations
import ctypes
from pathlib import Path
import stack_lift as lift
import resume_lift as resume
from test_bulk import Ctx
from test_stack import Stack, new_activation, oracle_slice, compare, MEMSIZE

ENTRY=0x003817D0
CALLEE=0x003B3CE0

def run():
    callers,_,_,_,flow,_=lift.collect()
    assert ENTRY in flow and any(kind=='branch' for _,_,kind,_ in callers[ENTRY])
    leaves,_=resume.collect()
    assert CALLEE in leaves
    bodies={a:{pc:(w,k,arg) for pc,w,k,arg in body} for a,body in callers.items()}
    leafwords={a:{pc:w for pc,w,_,_ in code} for a,(code,_) in leaves.items()}
    dll=ctypes.CDLL(str(Path(__file__).with_name('libae3_stack.so')))
    dll.ae3_stack_init.argtypes=[ctypes.POINTER(Stack),ctypes.c_uint32]
    dll.ae3_run_stack_slice.argtypes=[ctypes.POINTER(Ctx),ctypes.POINTER(Stack),ctypes.c_uint32]
    dll.ae3_run_stack_slice.restype=ctypes.c_int
    results={}
    for taken in (False,True):
        regs=[0]*32;regs[4]=0x300000;regs[29]=0x5F0000;regs[31]=0x710000
        ref_regs=regs.copy()
        ram=bytearray(MEMSIZE)
        ram[0x300004:0x300008]=(0x300100).to_bytes(4,'little')
        ram[0x300100]=1 if taken else 0
        ram[0x300000:0x300004]=(0x300200).to_bytes(4,'little')
        ram[0x300200:0x300204]=CALLEE.to_bytes(4,'little')
        ref_ram=ram.copy()
        ctx=Ctx((ctypes.c_uint64*32)(*regs),(ctypes.c_uint8*MEMSIZE).from_buffer(ram),MEMSIZE)
        native=Stack();dll.ae3_stack_init(ctypes.byref(native),ENTRY)
        state=dict(entry=ENTRY,depth=1,done=0,reserved=0,retired=0,frames=[new_activation(ENTRY)])
        observed_branch=False;observed_pending=False;observed_leaf=False
        for i in range(120):
            budget=(1,0,1,1,1)[i%5]
            got=dll.ae3_run_stack_slice(ctypes.byref(ctx),ctypes.byref(native),budget)
            want=oracle_slice(state,bodies,leafwords,ref_regs,ref_ram,budget)
            state['depth']=len(state['frames'])
            assert got==want,(taken,i,got,want)
            compare(native,state)
            assert list(ctx.gpr)==ref_regs,(taken,i,'registers')
            assert ram==ref_ram,(taken,i,'memory')
            if native.depth:
                a=native.frames[native.depth-1]
                observed_branch|=a.pc==0x3817E8 and a.pending==3
                observed_pending|=a.pc==(0x381808 if taken else 0x3817EC) and a.pending==0
                observed_leaf|=a.leaf_active and a.leaf.entry==CALLEE
            if got==0: break
            assert got==1,(taken,i,got)
        else:raise AssertionError('did not finish expected path')
        assert observed_branch and observed_pending,(taken,observed_branch,observed_pending)
        if taken: assert not observed_leaf
        else: assert observed_leaf
        results['taken' if taken else 'not_taken']={'slices':i+1,'instructions':native.retired,'called_leaf':bool(observed_leaf)}
    print('PASS targeted original-game BEQ with/without JALR:',results)

if __name__=='__main__':run()