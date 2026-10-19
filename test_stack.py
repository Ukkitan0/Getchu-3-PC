#!/usr/bin/env python3
"""Instruction-sliced C call stack vs independent decoder of the retail MIPS words.

Exercises nested real JAL, JR $ra, delay slots, saved return addresses, RAM,
zero-budget yields, fault-and-retry, and non-nested recovered callers.
"""
from __future__ import annotations
import ctypes,random,sys
from pathlib import Path
import stack_lift as lift
import resume_lift as rl
from test_bulk import Ctx,instr
from test_resume import Frame,oracle_word

MAX_DEPTH=lift.MAX_DEPTH
RNG=random.Random(0xAE3C011)
MEMSIZE=0x800000

class Activation(ctypes.Structure):
    _fields_=[(name,ctypes.c_uint32) for name in ('entry','pc','pending','target','return_pc','return_target','leaf_active','reserved')]+[('retired',ctypes.c_uint64),('leaf',Frame)]
class Stack(ctypes.Structure):
    _fields_=[(name,ctypes.c_uint32) for name in ('entry','depth','done','reserved')]+[('retired',ctypes.c_uint64),('frames',Activation*MAX_DEPTH)]

def new_activation(entry,ret=0):
    return dict(entry=entry,pc=entry,pending=0,target=0,return_pc=ret,
                return_target=0,leaf_active=0,reserved=0,retired=0,
                leaf=dict(entry=0,pc=0,next_pc=0,pending=0,retired=0,done=0,reserved=0))

def compare(stack,state):
    for name in ('entry','depth','done','reserved','retired'):
        assert getattr(stack,name)==state[name], (name,getattr(stack,name),state[name])
    for index,expected in enumerate(state['frames']):
        actual=stack.frames[index]
        for name in ('entry','pc','pending','target','return_pc','return_target','leaf_active','reserved','retired'):
            assert getattr(actual,name)==expected[name], (index,name,getattr(actual,name),expected[name],hex(stack.entry))
        for name,_ in Frame._fields_:
            assert getattr(actual.leaf,name)==expected['leaf'][name],(index,name,getattr(actual.leaf,name),expected['leaf'][name])

def oracle_step(state,bodies,leafwords,regs,memory):
    if state['done']:return 0
    a=state['frames'][-1]
    if a['leaf_active']:
        leaf=a['leaf'];old=leaf['retired']
        rc=oracle_word(leaf,leafwords[leaf['entry']],regs,memory)
        if rc:return rc
        delta=leaf['retired']-old
        a['retired']+=delta;state['retired']+=delta
        if leaf['done']:
            if (regs[31]&0xffffffff)!=a['pc']:return -4
            a['leaf_active']=0;a['target']=0
        return 0
    pc=a['pc']
    try:w,kind,arg=bodies[a['entry']][pc]
    except KeyError:return -4
    if a['pending']!=({'call_slot':1,'jalr_slot':1,'return_slot':2}.get(kind,0)):return -4
    if kind=='call':
        regs[31]=pc+8
        a.update(target=arg,pending=1,pc=pc+4)
    elif kind=='jalr':
        raw=regs[arg]
        if raw!=(raw&0xffffffff) and raw!=((raw&0xffffffff)|((0xffffffff00000000) if raw&0x80000000 else 0)):
            return -2
        target=raw&0xffffffff
        if target not in bodies and target not in leafwords:return -2
        regs[31]=pc+8
        a.update(target=target,pending=1,pc=pc+4)
    elif kind=='return':
        a.update(return_target=regs[31]&0xffffffff,pending=2,pc=pc+4)
    else:
        if kind=='return_slot' and len(state['frames'])>1 and a['return_target']!=a['return_pc']:
            return -4
        if kind in ('call_slot','jalr_slot') and a['target'] in bodies and len(state['frames'])>=MAX_DEPTH:
            return -4
        rc=instr(w,regs,memory)
        if rc:return rc
        if kind in ('call_slot','jalr_slot'):
            a.update(pc=pc+4,pending=0)
            if a['target'] in leafwords:
                a['leaf_active']=1
                a['leaf']=dict(entry=a['target'],pc=a['target'],next_pc=0,pending=0,retired=0,done=0,reserved=0)
            else:state['frames'].append(new_activation(a['target'],a['pc']))
        elif kind=='return_slot':
            a.update(pc=0,pending=0)
            if len(state['frames'])==1:
                state['frames'].clear();state['done']=1
            else:state['frames'].pop()
        else:a['pc']=pc+4
    a['retired']+=1;state['retired']+=1;regs[0]=0
    return 0

def oracle_slice(state,bodies,leafwords,regs,memory,budget):
    if state['done']:return 0
    for _ in range(budget):
        rc=oracle_step(state,bodies,leafwords,regs,memory)
        if rc:return rc
        if state['done']:return 0
    return 1

def run(trials=2):
    dll=ctypes.CDLL(str(Path(__file__).with_name('libae3_stack.so')))
    dll.ae3_stack_count.restype=ctypes.c_size_t
    dll.ae3_stack_init.argtypes=[ctypes.POINTER(Stack),ctypes.c_uint32]
    dll.ae3_run_stack_slice.argtypes=[ctypes.POINTER(Ctx),ctypes.POINTER(Stack),ctypes.c_uint32]
    dll.ae3_run_stack_slice.restype=ctypes.c_int
    callers,nested,indirect,_=lift.collect()
    leaves,_=rl.collect()
    bodies={a:{pc:(w,k,v) for pc,w,k,v in code} for a,code in callers.items()}
    leafwords={a:{pc:w for pc,w,_,_ in code} for a,(code,_) in leaves.items()}
    assert dll.ae3_stack_count()==len(callers)
    stats={'callers':len(callers),'nested_candidates':len(nested),'indirect_candidates':len(indirect),'trials':0,'slices':0,'nested_entered':0,
           'max_stack_depth':0,'jal_delay_checkpoints':0,'completed':0,'faulted':0,'yielded':0}
    for entry in callers:
        for trial in range(trials):
            values=[RNG.randrange(0,0x3000) for _ in range(32)]
            values[0]=0;values[29]=0x5f0000+trial*0x1000
            values[31]=0x7f0000+trial*4
            values[4]=0x300000;values[5]=0x302000;values[6]=0x304000
            reference=values.copy()
            mem=bytearray(MEMSIZE);oracle_mem=mem.copy()
            ctx=Ctx((ctypes.c_uint64*32)(*values),(ctypes.c_uint8*len(mem)).from_buffer(mem),len(mem))
            frame=Stack();dll.ae3_stack_init(ctypes.byref(frame),entry)
            expected=dict(entry=entry,depth=1,done=0,reserved=0,retired=0,frames=[new_activation(entry)])
            for iteration in range(120):
                budget=(0,1,1,2,3,5,7,11)[iteration%8]
                got=dll.ae3_run_stack_slice(ctypes.byref(ctx),ctypes.byref(frame),budget)
                desired=oracle_slice(expected,bodies,leafwords,reference,oracle_mem,budget)
                expected['depth']=len(expected['frames'])
                assert got==desired,(hex(entry),trial,iteration,got,desired)
                compare(frame,expected)
                assert list(ctx.gpr)==reference,(hex(entry),trial,iteration,'GPR mismatch')
                stats['slices']+=1
                stats['max_stack_depth']=max(stats['max_stack_depth'],frame.depth)
                stats['nested_entered']+=int(frame.depth>1)
                stats['jal_delay_checkpoints']+=int(bool(expected['frames']) and expected['frames'][-1]['pending']==1)
                if got==0:stats['completed']+=1;break
                if got<0:stats['faulted']+=1;break
                stats['yielded']+=1
            assert mem==oracle_mem,(hex(entry),trial,'RAM differs')
            stats['trials']+=1
    assert stats['max_stack_depth']>=2,stats
    assert stats['nested_entered']>0 and stats['jal_delay_checkpoints']>0,stats
    assert stats['completed']>0,stats
    # Actual retail virtual-dispatch wrapper: object->vtable->function pointer
    # at 0x0010CD14. The referenced leaf must execute and return to the
    # original caller, after a JALR with its independently decoded delay slot.
    entry=0x0010CD14
    assert entry in indirect
    reg=[0]*32;reg[4]=0x300000;reg[29]=0x5F0000;reg[31]=0x710000
    ram=bytearray(MEMSIZE);erm=ram.copy()
    ram[0x300000:0x300004]=(0x301000).to_bytes(4,'little')
    ram[0x301000:0x301004]=(0x003B3CE0).to_bytes(4,'little')
    erm[:]=ram
    ctx=Ctx((ctypes.c_uint64*32)(*reg),(ctypes.c_uint8*MEMSIZE).from_buffer(ram),MEMSIZE)
    native=Stack();dll.ae3_stack_init(ctypes.byref(native),entry)
    state=dict(entry=entry,depth=1,done=0,reserved=0,retired=0,frames=[new_activation(entry)])
    oracle_regs=reg.copy();visited=False
    for iteration in range(160):
        budget=(1,2,1,0,4)[iteration%5]
        actual=dll.ae3_run_stack_slice(ctypes.byref(ctx),ctypes.byref(native),budget)
        wanted=oracle_slice(state,bodies,leafwords,oracle_regs,erm,budget)
        state['depth']=len(state['frames'])
        assert actual==wanted,(iteration,actual,wanted)
        compare(native,state)
        assert list(ctx.gpr)==oracle_regs
        if native.frames[0].leaf.entry==0x003B3CE0:visited=True
        if actual==0:break
        assert actual==1,(iteration,actual)
    else:raise AssertionError('JALR valid-target test failed to terminate')
    assert visited and ram==erm
    stats['indirect_valid_target_completed']=1
    # Unsupported virtual function pointers MUST fault as UNKNOWN, without
    # spuriously changing $ra or executing the JALR delay slot.
    ram[0x301000:0x301004]=(0xDEADBEEF).to_bytes(4,'little')
    ctx=Ctx((ctypes.c_uint64*32)(*reg),(ctypes.c_uint8*MEMSIZE).from_buffer(ram),MEMSIZE)
    bad=Stack();dll.ae3_stack_init(ctypes.byref(bad),entry)
    assert dll.ae3_run_stack_slice(ctypes.byref(ctx),ctypes.byref(bad),64)==-2
    assert bad.frames[0].pc==0x0010CD28
    assert ctx.gpr[31]==0x710000
    stats['indirect_unknown_rejected']=1
    unknown=Stack();dll.ae3_stack_init(ctypes.byref(unknown),0xDEADBEEF)
    assert dll.ae3_run_stack_slice(ctypes.byref(ctx),ctypes.byref(unknown),1)==-2
    print('PASS native stacked JAL/JR against original R5900 words',stats)
    return stats

if __name__=='__main__':run(int(sys.argv[1]) if len(sys.argv)>1 else 2)