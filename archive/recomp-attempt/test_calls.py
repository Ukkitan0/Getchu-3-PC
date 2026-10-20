#!/usr/bin/env python3
"""Differential test native C caller->callee execution against original MIPS words.

Independent oracle executes the actual caller JAL instructions and translated
callee instruction words via test_resume's separate MIPS instruction decoder.
No game data or machine-code-derived sources are committed to the public repo.
"""
from __future__ import annotations
import ctypes,random,sys
from pathlib import Path
import call_lift as lift
import resume_lift as rl
from test_bulk import Ctx,instr
from test_resume import Frame,oracle_word

RAND=random.Random(0x3CA11)
MEMSIZE=8*1024*1024
FIELDS=[s[0] for s in Frame._fields_]

class CallFrame(ctypes.Structure):
    _fields_=[('entry',ctypes.c_uint32),('pc',ctypes.c_uint32),
              ('target',ctypes.c_uint32),('pending',ctypes.c_uint32),
              ('active',ctypes.c_uint32),('done',ctypes.c_uint32),
              ('reserved0',ctypes.c_uint32),('reserved1',ctypes.c_uint32),
              ('retired',ctypes.c_uint64),('child',Frame)]

def child_init(entry):
    return dict(entry=entry,pc=entry,next_pc=0,pending=0,retired=0,done=0,reserved=0)

def oracle_step(state,words,leafwords,R,M):
    if state['done']:return 0
    if state['active']:
        child=state['child']
        try: rc=oracle_word(child,leafwords[child['entry']],R,M)
        except KeyError as e:raise AssertionError('Out of candidate control flow '+str(e))
        if rc:return rc
        state['retired']+=1
        if child['done']:
            if (R[31]&0xffffffff)!=state['pc']:return -4
            state['target']=0;state['active']=0
        return 0
    pc=state['pc']
    w,kind,arg=words[pc]
    expected_pending={'call_slot':1,'return_slot':2}.get(kind,0)
    if state['pending']!=expected_pending:return -4
    if kind=='call':
        R[31]=pc+8
        state.update(target=arg,pending=1,pc=pc+4)
    elif kind=='return':
        state.update(pending=2,pc=pc+4)
    else:
        rc=instr(w,R,M)
        if rc:return rc
        if kind=='call_slot':
            state.update(pc=pc+4,pending=0,active=1,child=child_init(state['target']))
        elif kind=='return_slot':
            state.update(pc=0,pending=0,done=1)
        else:state['pc']=pc+4
    state['retired']+=1
    R[0]=0
    return 0

def oracle_slice(state,words,leafwords,R,M,budget):
    if state['done']:return 0
    for _ in range(budget):
        result=oracle_step(state,words,leafwords,R,M)
        if result:return result
        if state['done']:return 0
    return 1

def check_frame(native,expected):
    keys=['entry','pc','target','pending','active','done','reserved0','reserved1','retired']
    for k in keys:
        assert getattr(native,k)==expected[k], (k,getattr(native,k),expected[k],hex(native.entry))
    for k in FIELDS:
        assert getattr(native.child,k)==expected['child'][k],(k,getattr(native.child,k),expected['child'][k])

def run(trials=3):
    dll=ctypes.CDLL(str(Path(__file__).with_name('libae3_calls.so')))
    dll.ae3_call_count.restype=ctypes.c_size_t
    dll.ae3_call_init.argtypes=[ctypes.POINTER(CallFrame),ctypes.c_uint32]
    dll.ae3_run_call_slice.argtypes=[ctypes.POINTER(Ctx),ctypes.POINTER(CallFrame),ctypes.c_uint32]
    dll.ae3_run_call_slice.restype=ctypes.c_int
    callers,_=lift.collect()
    leaves,_=rl.collect()
    assert len(callers)==dll.ae3_call_count()
    leafwords={a:{pc:w for pc,w,_,_ in code} for a,(code,_) in leaves.items()}
    stats=dict(candidates=len(callers),trials=0,compared_slices=0,jal_delay_checkpoints=0,
               executed_calls=0,completed=0,faults=0,paused=0,limited=0)
    for entry,code in callers.items():
        words={pc:(w,k,v) for pc,w,k,v in code}
        for trial in range(trials):
            R=[RAND.randrange(0,0x6000) for _ in range(32)]
            R[0]=0;R[29]=0x200000+0x1000*trial
            R[31]=0x400000+trial*4
            if trial==1:
                R[4]=0x300000;R[5]=0x302000;R[6]=0x304000
            ER=R.copy()
            M=bytearray(MEMSIZE)
            # Values loaded from object pointers are usually null/zero. 8MiB
            # includes the actual game's global data addresses for this subset.
            EM=M.copy()
            cbuffer=(ctypes.c_uint8*MEMSIZE).from_buffer(M)
            ctx=Ctx((ctypes.c_uint64*32)(*R),cbuffer,MEMSIZE)
            native=CallFrame();dll.ae3_call_init(ctypes.byref(native),entry)
            expect=dict(entry=entry,pc=entry,target=0,pending=0,active=0,done=0,
                        reserved0=0,reserved1=0,retired=0,child=child_init(0))
            for iteration in range(150):
                budget=[0,1,1,2,3,7,15][iteration%7]
                oldactive=expect['active']
                actual=dll.ae3_run_call_slice(ctypes.byref(ctx),ctypes.byref(native),budget)
                wanted=oracle_slice(expect,words,leafwords,ER,EM,budget)
                check_frame(native,expect)
                assert actual==wanted,(hex(entry),trial,iteration,actual,wanted)
                assert list(ctx.gpr)==ER,(hex(entry),trial,iteration,'register mismatch')
                stats['compared_slices']+=1
                stats['jal_delay_checkpoints']+=(expect['pending']==1)
                stats['executed_calls']+=int(oldactive==0 and expect['active']==1)
                if wanted==0:stats['completed']+=1;break
                if wanted<0:stats['faults']+=1;break
                stats['paused']+=1
            else:stats['limited']+=1  # a genuine loop may not terminate under arbitrary inputs
            assert M==EM,(hex(entry),trial,'RAM mismatch')
            stats['trials']+=1
    assert stats['executed_calls']>0
    assert stats['jal_delay_checkpoints']>0
    assert stats['completed']>0
    # No source for arbitrary PC: reject without dispatching random opcodes.
    dummy=Ctx((ctypes.c_uint64*32)(),None,0)
    bad=CallFrame();dll.ae3_call_init(ctypes.byref(bad),0xdeadbeef)
    assert dll.ae3_run_call_slice(ctypes.byref(dummy),ctypes.byref(bad),1)==-2
    # Fault recovery across the caller/callee boundary: the guest caller's
    # stack save faults before JAL. The same preserved frame must resume after
    # RAM is mapped without replaying already-retired instructions.
    entry=0x00112CC4
    words={pc:(w,k,v) for pc,w,k,v in callers[entry]}
    regs=[0]*32;regs[2]=0x2000;regs[29]=0xD000;regs[31]=0x12345678
    er=regs.copy(); mem=bytearray(MEMSIZE)
    ctx=Ctx((ctypes.c_uint64*32)(*regs),(ctypes.c_uint8*MEMSIZE).from_buffer(mem),0)
    frame=CallFrame();dll.ae3_call_init(ctypes.byref(frame),entry)
    expect=dict(entry=entry,pc=entry,target=0,pending=0,active=0,done=0,
                reserved0=0,reserved1=0,retired=0,child=child_init(0))
    assert dll.ae3_run_call_slice(ctypes.byref(ctx),ctypes.byref(frame),8)==-1
    assert oracle_slice(expect,words,leafwords,er,bytearray(),8)==-1
    check_frame(frame,expect)
    assert frame.pc==entry+4 and frame.retired==1
    ctx.ram_size=MEMSIZE; expected_ram=bytearray(MEMSIZE)
    assert dll.ae3_run_call_slice(ctypes.byref(ctx),ctypes.byref(frame),128)==0
    assert oracle_slice(expect,words,leafwords,er,expected_ram,128)==0
    check_frame(frame,expect)
    assert list(ctx.gpr)==er and mem==expected_ram
    stats['caller_fault_resume_passed']=1
    print('PASS direct JAL native-C inter-function execution vs original MIPS',stats)
    return stats

if __name__=='__main__':run(int(sys.argv[1]) if len(sys.argv)>1 else 3)