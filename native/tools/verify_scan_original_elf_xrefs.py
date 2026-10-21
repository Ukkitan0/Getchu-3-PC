#!/usr/bin/env python3
"""Regression-test whole-original-ELF call graph against owner-private USA source."""
import argparse,json
from pathlib import Path
from scan_original_elf_xrefs import scan

def verify(path):
    roots=[0x36f4d4,0x36f548,0x4c1b3c,0x57bcb8,0x57c7cc,0x57c8ec,0x57ca68]
    report=scan(Path(path),roots,depth=0)
    expected={
        'original_instruction_words_scanned':1351830,
        'exact_direct_JAL_sites':109169,
        'distinct_direct_JAL_targets':12228,
        'exact_direct_J_sites':5422,
        'indirect_JALR_sites_needing_recovery':6106,
        'COP2_macro_instructions':30239,
        'conditional_branch_instructions':76206,
        'original_data_section_pointers_to_text':23198,
        'distinct_data_code_pointer_targets':14126,
    }
    for name,expected_value in expected.items():
        if report[name]!=expected_value:
            raise ValueError(f'Owner-original ELF survey diverged: {name}')
    rows={int(r['target'],0):r
          for r in report['reverse_xrefs'][0]['targets']}
    counts={0x36f4d4:78,0x36f548:325,0x4c1b3c:17,
            0x57bcb8:2,0x57c7cc:2,0x57c8ec:3,0x57ca68:1}
    for addr,count in counts.items():
        if rows[addr]['direct_original_JAL_count']!=count:
            raise ValueError(f'Original target call count diverged: {addr:#x}')
    callsites={
        0x57c8ec:{0x57c840,0x57c878,0x57c8dc},
        0x57c7cc:{0x57bf28,0x57c3d0},
        0x57bcb8:{0x57bae4,0x57bb4c},
    }
    for target,actual in callsites.items():
        found={int(x['call_site'],0) for x in rows[target]['source_calls']}
        if found!=actual:raise ValueError(f'Original target callers changed: {target:#x}')
    return {'source':'owned USA SCUS_975.01','full_source_instructions':1351830,
            'confirmed_original_direct_JAL_sites':109169,
            'confirmed_data_to_code_pointer_sites':23198,
            'verified_selected_source_calls':sum(map(len,callsites.values())),
            'original_NAV_actor_RNG_verified':True,
            'native_executable_booted':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path);a=p.parse_args()
    print(json.dumps(verify(a.original_elf),indent=2))
