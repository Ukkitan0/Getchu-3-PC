#!/usr/bin/env python3
"""Verify the retail EE entrypoint's first register-reset instructions.

An audit only, not a substitute for translating MMI/HI-LO/COP1 instructions.
Requires original NTSC-U SCUS_975.01. Machine words are never committed.
"""
import json
from pathlib import Path
import bulk_lift as game

ENTRY = 0x00100134

def word(pc):
    if not game.valid(pc):
        raise ValueError(f'outside executable: {pc:08X}')
    return game.W[(pc-game.TEXT)//4]

def run():
    assert game.EXPECTED == '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
    targets=list(range(1,26))+list(range(28,32))
    for idx,rd in enumerate(targets):
        pc=ENTRY+4*idx
        w=word(pc)
        # PCSX2 R5900 MMI/ MM1 opcode table: PADDUW uses op 0x1C,
        # funct 0x28 and shift-selector 0x10. Here both source GPRs are zero.
        assert (w>>26,w&63,(w>>6)&31,(w>>21)&31,(w>>16)&31,(w>>11)&31)==(0x1c,0x28,16,0,0,rd),(hex(pc),hex(w))
    addr=ENTRY+4*len(targets)
    for expected in (0x00000011,0x70000011,0x00000013,0x70000013):
        assert word(addr)==expected,(hex(addr),hex(word(addr)))
        addr+=4
    assert word(addr)==0x04190000  # BGEZALL $zero, self-next, sets return link
    addr+=4
    for fpr in range(32):
        w=word(addr)
        assert (w>>26,(w>>21)&31,(w>>16)&31,(w>>11)&31)==(0x11,4,0,fpr),(hex(addr),hex(w))
        addr+=4
    assert addr==0x0010023C
    # Next FPU accumulator/control writes require a real COP1 state model.
    next_words=[f'{word(addr+4*i):08X}' for i in range(4)]
    report={
        'sha256':game.EXPECTED,
        'entry':f'{ENTRY:08X}',
        'zeroed_128bit_gpr_registers':targets,
        'verified_padduw_128bit_zero_count':len(targets),
        'verified_hilo_bank_clear_count':4,
        'verified_mtc1_fpr_zero_count':32,
        'first_remaining_pc':f'{addr:08X}',
        'next_machine_words':next_words,
        'execution_supported':False,
        'requirements':['128-bit GPR lane model (PADDUW)','HI/LO0 and HI/LO1','COP1 FPR and accumulator/control state','startup branches and PS2 services'],
        'notes':'Decoded original-word initialization only; not a boot success or guest execution.'
    }
    target=Path(__file__).resolve().parent/'boot_entry_audit_report.json'
    target.write_text(json.dumps(report,indent=2)+'\n')
    print(f'PASS boot entry audit: {len(targets)} PADDUW GPR clears, 4 HI/LO clears, 32 MTC1 clears; next 0x{addr:08X} COP1 instruction is unsupported')
    return report

if __name__=='__main__': run()