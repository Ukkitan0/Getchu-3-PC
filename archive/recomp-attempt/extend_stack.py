#!/usr/bin/env python3
"""Source-recovery extension: bounded forward-CFG callers with real JAL/JALR.

A separate collector helps keep the older direct/indirect stack scanner stable.
This intentionally does not decode backward caller branches or branch-likely opcodes.
"""
from __future__ import annotations
import cfg_lift as cfg
import bulk_lift as base

MAX_WORDS=120
MAX_BRANCHES=12
MAX_CALLS=12

def collect(extra_starts, occupied, targets):
    result={}; rejected={'unsupported':0,'flow':0,'overlap':0}
    accepted_occupied=set(occupied)
    for entry in sorted(extra_starts):
        if entry in accepted_occupied:continue
        code=[]; pc=entry; transfers=[]; slots=set(); calls=0; branch_count=0
        for _ in range(MAX_WORDS):
            if not base.valid(pc):break
            w=base.W[(pc-base.TEXT)//4]
            if w==0x03e00008:
                if not base.valid(pc+4):break
                delay=base.W[(pc+4-base.TEXT)//4]; stmt=base.emit(delay)
                if stmt is None:break
                code.extend([(pc,w,'return',None),(pc+4,delay,'return_slot',stmt)])
                slot_addr=pc+4;slots.add(slot_addr)
                pcs={p for p,_,_,_ in code}
                # Exactly cover both possible control-flow paths and disallow transfers
                # into any delay slot or out of the verified body.
                if branch_count and calls and len(pcs)==len(code) and all(t in pcs and t not in slots for t in transfers):
                    if pcs.isdisjoint(accepted_occupied):
                        result[entry]=code;accepted_occupied.update(pcs)
                    else:rejected['overlap']+=1
                else:rejected['flow']+=1
                break
            cond=cfg.branch(pc,w)
            direct_jump=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2) if (w>>26)==2 else None
            if cond or direct_jump is not None:
                if not base.valid(pc+4) or branch_count>=MAX_BRANCHES:break
                target=cond[1] if cond else direct_jump
                if target <= pc+4:break # Avoid loops and self-referential delay slots.
                delay=base.W[(pc+4-base.TEXT)//4];stmt=base.emit(delay)
                if stmt is None:break
                code.extend([(pc,w,'branch' if cond else 'jump',cond if cond else target),
                             (pc+4,delay,'branch_slot',stmt)])
                transfers.append(target);slots.add(pc+4);branch_count+=1;pc+=8;continue
            if (w>>26)==3:
                target=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
                if target not in targets or calls>=MAX_CALLS or not base.valid(pc+4):break
                delay=base.W[(pc+4-base.TEXT)//4];stmt=base.emit(delay)
                if stmt is None:break
                code.extend([(pc,w,'call',target),(pc+4,delay,'call_slot',stmt)])
                slots.add(pc+4);calls+=1;pc+=8;continue
            if (w>>26)==0 and (w&63)==9:
                rs=(w>>21)&31; rd=(w>>11)&31
                if not rs or rd!=31 or calls>=MAX_CALLS or not base.valid(pc+4):break
                delay=base.W[(pc+4-base.TEXT)//4];stmt=base.emit(delay)
                if stmt is None:break
                code.extend([(pc,w,'jalr',rs),(pc+4,delay,'jalr_slot',stmt)])
                slots.add(pc+4);calls+=1;pc+=8;continue
            stmt=base.emit(w)
            if stmt is None:break
            code.append((pc,w,'scalar',stmt));pc+=4
        else:rejected['unsupported']+=1
    return result,rejected