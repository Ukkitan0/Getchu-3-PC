#!/usr/bin/env python3
"""Ensure newly recovered call/loop candidates meet strict original-word CFG rules."""
import loop_stack
import stack_lift
import resume_lift


def run():
    callers,nested,indirect,leaf_entries,combined,rejected=stack_lift.collect()
    leaves,_=resume_lift.collect()
    prev=set(callers)-{int(x,16) for x in rejected['backward_addresses']}
    occupied={pc for addr,body in callers.items() if addr in prev
              for pc,*_ in body}
    occupied.update(pc for code,_ in leaves.values() for pc,*_ in code)
    loops,stats=loop_stack.collect(stack_lift.b.starts,occupied,prev|leaf_entries)
    expected={int(x,16) for x in rejected['backward_addresses']}
    assert set(loops)==expected and len(loops)==42, (len(loops),len(expected))
    for addr,body in loops.items():
        pcs={pc for pc,_,_,_ in body}
        slots={pc for pc,_,kind,_ in body if kind.endswith('_slot')}
        assert len(pcs)==len(body) and not pcs.intersection(occupied)
        assert body[-2][2]=='return' and body[-1][2]=='return_slot'
        assert sum(kind in ('call','jalr') for _,_,kind,_ in body)>0
        backward=[]
        for pc,word,kind,arg in body:
            if kind in ('branch','jump'):
                target=arg[1] if kind=='branch' else arg
                assert target in pcs and target not in slots
                if target <= pc:backward.append((pc,target))
        assert backward,(hex(addr),body)
        assert all(pc not in slots for pc,_ in backward)
    assert stack_lift.MAX_DEPTH==8
    print(f'PASS checked {len(loops)} recovered MIPS caller loops, '
          'all branch targets, delay-slot exclusions and original boundaries')

if __name__=='__main__':run()