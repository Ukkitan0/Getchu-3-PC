#!/usr/bin/env python3
"""Independent reference interpreter for the bounded branch/return subset.

Checks branch decisions BEFORE delay slots, fallthrough, delay-slot execution,
full low 64-bit register state and byte-exact 64KiB synthetic memory changes.
"""
import ctypes
import random
import sys
from pathlib import Path
import cfg_lift as lifter
from test_bulk import Ctx, instr, signed

MASK = (1 << 64) - 1
rng = random.Random(0xAE3C0F)


def execute(entry, code, regs, memory):
    end = code[-2][0]
    words = {pc:word for pc, word, _, _ in code}
    pc = entry
    steps = 0
    while steps < 300:
        steps += 1
        word = words[pc]
        if word == 0x03e00008:
            result = instr(words[pc+4], regs, memory)
            if result == 0: regs[0] = 0
            return result
        branch = lifter.branch(pc, word)
        direct_jump = ((pc+4)&0xF0000000) | ((word&0x03ffffff)<<2) if (word>>26)==2 else None
        if branch is not None:
            op = word >> 26
            rs = (word >> 21) & 31
            rt = (word >> 16) & 31
            x, y = regs[rs], regs[rt]
            if op == 4: take = x == y
            elif op == 5: take = x != y
            elif op == 6: take = signed(x, 64) <= 0
            elif op == 7: take = signed(x, 64) > 0
            elif op == 1 and rt == 0: take = signed(x, 64) < 0
            elif op == 1 and rt == 1: take = signed(x, 64) >= 0
            else: raise AssertionError(hex(word))
            result = instr(words[pc+4], regs, memory)
            if result: return result
            pc = branch[1] if take else pc+8
        elif direct_jump is not None:
            result = instr(words[pc+4], regs, memory)
            if result:return result
            pc=direct_jump
        else:
            result = instr(word, regs, memory)
            if result: return result
            pc += 4
        if pc > end or pc not in words: raise AssertionError(f'out-of-bounds CFG: {entry:x} -> {pc:x}')
    raise AssertionError(f'nonterminating CFG at {entry:x}')


def run(trials=8):
    dll=ctypes.CDLL(str(Path(__file__).with_name('libae3_control.so')))
    dll.ae3_run_control_leaf.argtypes=[ctypes.POINTER(Ctx),ctypes.c_uint32]
    dll.ae3_run_control_leaf.restype=ctypes.c_int
    dll.ae3_control_leaf_count.restype=ctypes.c_size_t
    routines, _ = lifter.translate()
    assert len(routines) == dll.ae3_control_leaf_count()
    n=0
    for entry, (code, num_branches) in routines.items():
        for t in range(trials):
            regs=[rng.getrandbits(64) if t%4==0 else rng.randrange(65536) for _ in range(32)]
            regs[0]=0
            regs[29]=rng.randrange(32000,60000)
            # In half the trials, deliberately equalize the argument GPRs to
            # exercise BEQ branches rather than only rare random equality.
            if t%2==1 and any(kind=='branch' for pc,w,kind,arg in code):
                word=next(w for pc,w,kind,_ in code if kind=='branch')
                rs=(word>>21)&31;rt=(word>>16)&31
                if (word>>26) in (4,5) and rs !=0 and rt !=0: regs[rt]=regs[rs]
            mem=bytearray(rng.randbytes(65536))
            expregs=regs.copy(); expmem=mem.copy()
            expected=execute(entry,code,expregs,expmem)
            buf=(ctypes.c_uint8*len(mem)).from_buffer(mem)
            ctx=Ctx((ctypes.c_uint64*32)(*regs),buf,len(mem))
            actual=dll.ae3_run_control_leaf(ctypes.byref(ctx),entry)
            if actual!=expected or list(ctx.gpr)!=expregs or mem!=expmem:
                diffs=[(i,hex(ctx.gpr[i]),hex(v)) for i,v in enumerate(expregs) if ctx.gpr[i]!=v]
                badbyte=next((i for i,(a,b) in enumerate(zip(mem,expmem)) if a!=b),-1)
                raise AssertionError(f'{entry:08x} trial {t}: status {actual}/{expected}, regs {diffs[:4]}, mem byte {badbyte}')
            n+=1
    print(f'PASS {n} original-word control-flow comparisons across {len(routines)} routines')


if __name__ == '__main__': run(int(sys.argv[1]) if len(sys.argv)>1 else 8)