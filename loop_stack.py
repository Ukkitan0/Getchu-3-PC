#!/usr/bin/env python3
"""Conservative bounded backward/forward branch recovery for R5900 callers.

These are candidate function boundaries inferred from direct-call targets, not
complete or authoritative decompilation. Accept only bodies that terminate in a
single JR $ra and whose branches stay within their fully decoded body. Neither
branches into delay slots nor nested delay-slot transfers are permitted.
"""
from __future__ import annotations
import bulk_lift as b
import cfg_lift as cfg

MAX_WORDS = 192
MAX_BRANCHES = 24
MAX_CALLS = 16


def collect(starts, occupied, valid_targets):
    results = {}
    used = set(occupied)
    rejects = {"unsupported": 0, "not_loop": 0, "flow": 0,
               "overlap": 0, "exhausted": 0}
    for entry in sorted(starts):
        if entry in used: continue
        code = []
        slots = set()
        transfers = []
        calls = 0
        branches = 0
        backedges = 0
        pc = entry
        reason = 'unsupported'
        for _ in range(MAX_WORDS):
            if not b.valid(pc): break
            w = b.W[(pc - b.TEXT) // 4]
            if w == 0x03e00008:
                if not b.valid(pc+4): break
                delay = b.W[(pc+4-b.TEXT)//4]
                stmt = b.emit(delay)
                if stmt is None: break
                code.extend([(pc, w, 'return', None),
                             (pc+4, delay, 'return_slot', stmt)])
                slots.add(pc+4)
                pcs = {p for p,_,_,_ in code}
                # No instruction address may be repeated; every directed edge
                # has to lead to an instruction and not a delay-slot word.
                if not backedges or not calls:
                    reason = 'not_loop'
                elif len(pcs) != len(code) or not all(t in pcs and t not in slots for t in transfers):
                    reason = 'flow'
                elif not pcs.isdisjoint(used):
                    reason = 'overlap'
                else:
                    results[entry] = code
                    used.update(pcs)
                    reason = None
                break
            cond = cfg.branch(pc, w)
            jump = ((pc+4)&0xf0000000)|((w&0x3ffffff)<<2) if w>>26==2 else None
            if cond is not None or jump is not None:
                if branches >= MAX_BRANCHES or not b.valid(pc+4): break
                target = cond[1] if cond is not None else jump
                # A backedge may target entry or a scalar/call/branch, but
                # never any delay-slot word or a different function.
                if target < entry or (target & 3):
                    reason = 'flow'; break
                delay = b.W[(pc+4-b.TEXT)//4]
                stmt = b.emit(delay)
                if stmt is None:break
                code.extend([(pc,w,'branch' if cond is not None else 'jump',
                              cond if cond is not None else target),
                             (pc+4,delay,'branch_slot',stmt)])
                slots.add(pc+4)
                transfers.append(target)
                backedges += int(target <= pc)
                branches += 1
                pc += 8
                continue
            if w>>26 == 3:
                target = ((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
                if target not in valid_targets or calls >= MAX_CALLS or not b.valid(pc+4):break
                delay = b.W[(pc+4-b.TEXT)//4]
                stmt = b.emit(delay)
                if stmt is None:break
                code.extend([(pc,w,'call',target),(pc+4,delay,'call_slot',stmt)])
                slots.add(pc+4); calls += 1; pc += 8
                continue
            if w>>26==0 and (w&63)==9:
                rs=(w>>21)&31; rd=(w>>11)&31
                if not rs or rd!=31 or calls >= MAX_CALLS or not b.valid(pc+4):break
                delay = b.W[(pc+4-b.TEXT)//4]
                stmt = b.emit(delay)
                if stmt is None: break
                code.extend([(pc,w,'jalr',rs),(pc+4,delay,'jalr_slot',stmt)])
                slots.add(pc+4); calls += 1; pc += 8
                continue
            stmt = b.emit(w)
            if stmt is None:break
            code.append((pc,w,'scalar',stmt));pc+=4
        else:
            reason = 'exhausted'
        if reason is not None: rejects[reason] += 1
    return results, rejects