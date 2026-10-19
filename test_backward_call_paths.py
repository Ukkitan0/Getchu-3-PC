#!/usr/bin/env python3
"""Reference-test a retail counted loop with repeated JAL calls and delay-slot yields.

Retail routine 0x00105AC4 looks up N 32-bit entries through JAL 0x00105A28,
then stores each result sequentially. The test is not synthetic pseudocode: the
independent oracle decodes the actual SCUS_975.01 MIPS instructions.
"""
from __future__ import annotations
import ctypes
from pathlib import Path
import stack_lift as lift
import resume_lift as resume
from test_bulk import Ctx
from test_stack import Stack,new_activation,oracle_slice,compare,MEMSIZE

ENTRY=0x00105AC4
CALLEE=0x00105A28
DEST=0x300000
SOURCE=0x301000


def run():
    callers,_,_,_,flow,meta=lift.collect()
    assert ENTRY in flow and ENTRY in {int(x,16) for x in meta['backward_addresses']}
    body=callers[ENTRY]
    assert sum(kind=='call' and arg==CALLEE for _,_,kind,arg in body)==1
    leaves,_=resume.collect()
    bodies={addr:{pc:(word,kind,arg) for pc,word,kind,arg in instructions}
            for addr,instructions in callers.items()}
    leafwords={addr:{pc:word for pc,word,_,_ in code} for addr,(code,_) in leaves.items()}
    dll=ctypes.CDLL(str(Path(__file__).with_name('libae3_stack.so')))
    dll.ae3_stack_init.argtypes=[ctypes.POINTER(Stack),ctypes.c_uint32]
    dll.ae3_run_stack_slice.argtypes=[ctypes.POINTER(Ctx),ctypes.POINTER(Stack),ctypes.c_uint32]
    dll.ae3_run_stack_slice.restype=ctypes.c_int
    results={}
    for count in (0,1,3):
        regs=[0]*32
        regs[4]=DEST;regs[5]=SOURCE;regs[6]=count
        regs[29]=0x5f0000;regs[31]=0x710000
        refregs=regs.copy()
        ram=bytearray(MEMSIZE)
        values=[0x12345678,0x23571113,0xDEADBEEF,0x87654321]
        for i,value in enumerate(values):
            ram[SOURCE+4*i:SOURCE+4*i+4]=value.to_bytes(4,'little')
        rram=ram.copy()
        ctx=Ctx((ctypes.c_uint64*32)(*regs),(ctypes.c_uint8*MEMSIZE).from_buffer(ram),MEMSIZE)
        state=Stack();dll.ae3_stack_init(ctypes.byref(state),ENTRY)
        expected=dict(entry=ENTRY,depth=1,done=0,reserved=0,retired=0,frames=[new_activation(ENTRY)])
        backward_slots=0;callee_visits=0
        for i in range(1200):
            budget=(0,1,2,1,3)[i%5]
            got=dll.ae3_run_stack_slice(ctypes.byref(ctx),ctypes.byref(state),budget)
            want=oracle_slice(expected,bodies,leafwords,refregs,rram,budget)
            expected['depth']=len(expected['frames'])
            assert got==want,(count,i,got,want)
            compare(state,expected)
            assert list(ctx.gpr)==refregs,(count,i,'GPR mismatch')
            assert ram==rram,(count,i,'RAM mismatch')
            if state.depth:
                a=state.frames[state.depth-1]
                if a.pc==0x105b10 and a.pending==3 and a.target==0x105af4:
                    backward_slots+=1
                if a.leaf_active and a.leaf.entry==CALLEE:
                    callee_visits+=1
            if got==0:break
            assert got==1,(count,i,got)
        else:raise AssertionError(f'count={count} did not terminate')
        assert backward_slots>0 if count>1 else backward_slots==0,(count,backward_slots)
        assert callee_visits>0 if count else callee_visits==0
        for j in range(4):
            actual=int.from_bytes(ram[DEST+4*j:DEST+4*j+4],'little')
            assert actual==(values[j] if j<count else 0),(count,j,hex(actual))
        assert state.depth==0 and state.done==1
        assert ctx.gpr[29]==regs[29] and ctx.gpr[31]==regs[31]
        results[count]={'slices':i+1,'guest_instructions':state.retired,
                        'taken_backward_delay_checkpoints':backward_slots,
                        'callee_checkpoints':callee_visits}
    print('PASS retail backward-branch/JAL multi-iteration memory results:',results)

if __name__=='__main__':run()