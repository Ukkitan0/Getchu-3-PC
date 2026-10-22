#!/usr/bin/env python3
"""Decode genuine AE3 USA fallback VU0 macro instructions, not an emulator.

Original SCUS_975.01 remains owner-private. This prints only instruction
metadata: vector op names, source/destination registers, component masks.
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
BODY='6e33608fffc7f8f6980c8eefaee4f856925ba2b0d60fa7fd095dbc8f9c80c886'
OPS={0:'VADDx',1:'VADDy',2:'VADDz',3:'VADDw',
  0x18:'VMULx',0x19:'VMULy',0x1a:'VMULz',0x1b:'VMULw',
  0x1c:'VMULq',0x28:'VADD',0x2a:'VMUL',0x2c:'VSUB'}
SPECIAL={0x30:'VMOVE',0x38:'VRNEXT',0x39:'VRGET',0x3a:'VRINIT',0x3b:'VRXOR'}
PAIRS=((0x57cb90,0x57cb94),(0x57cbb0,0x57cbb4),
       (0x57cc88,0x57cc8c),(0x57cca8,0x57ccac))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=SHA:
        raise ValueError('Original USA SCUS_975.01 SHA mismatch')
    if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
        raise ValueError('Expected original ELF32 MIPS little-endian')
    ph=struct.unpack_from('<I',data,28)[0]
    step,num=struct.unpack_from('<HH',data,42)
    seg=[]
    for i in range(num):
        typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+step*i)
        if typ==1 and off+size<=len(data):seg.append((va,va+size,off))
    def read(va,n):
        for lo,hi,off in seg:
            if lo<=va and va+n<=hi:return data[off+va-lo:off+va-lo+n]
        raise ValueError(f'Source address unmapped: {va:#x}')
    def word(va):return struct.unpack('<I',read(va,4))[0]
    if hashlib.sha256(read(0x57ca68,0x2b4)).hexdigest()!=BODY:
        raise ValueError('Retail 0x57CA68 original code changed')
    def decode(addr):
        instruction=word(addr)
        if instruction>>26!=18 or (instruction>>21)&31<16:return None
        op=instruction&63
        sub=(instruction&3)|((instruction>>4)&0x7c) if op>=60 else None
        name=(SPECIAL.get(sub,'UNKNOWN') if sub is not None else OPS.get(op,'UNKNOWN'))
        if name=='UNKNOWN':raise ValueError(f'Unknown original VU0 opcode {addr:#x}')
        mask=''.join(s for s,bit in (('x',24),('y',23),('z',22),('w',21))
                     if instruction&(1<<bit))
        return {'address':f'0x{addr:08X}','name':name,
          'destinationVF':(instruction>>6)&31,
          'sourceVF':(instruction>>11)&31,
          'argumentVF':(instruction>>16)&31,
          'destinationMask':mask,
          'sourceScalar':'xyzw'[(instruction>>21)&3]}
    ledger={}
    for name,first,last in (('first_candidate',0x57cb4c,0x57cbbc),
                            ('five_later_candidates',0x57cc48,0x57ccb4)):
        ledger[name]=[x for a in range(first,last,4) if (x:=decode(a))]
        labels=[x['name'] for x in ledger[name]]
        if labels.count('VRINIT')!=2 or labels.count('VRXOR')!=2 or labels.count('VMULq')!=2:
            raise ValueError('Original VU0 candidate block special operations changed')
    if len(ledger['first_candidate'])!=20 or len(ledger['five_later_candidates'])!=19:
        raise ValueError('Original fallback vector pipeline count differs')
    if sum(x['name']=='VMOVE' for x in ledger['first_candidate'])!=1:
        raise ValueError('Original first candidate has unique VMOVE')
    if any(x['name']=='VMOVE' for x in ledger['five_later_candidates']):
        raise ValueError('Original repeated candidate erroneously copies VF')
    for init,xor in PAIRS:
        a,b=decode(init),decode(xor)
        if (a['name'],a['sourceVF'],a['sourceScalar'])!=('VRINIT',0,'w'):
            raise ValueError('Original VRINIT not reading constant VF0.w')
        if (b['name'],b['sourceVF'],b['sourceScalar'])!=('VRXOR',0,'x'):
            raise ValueError('Original VRXOR not reading constant VF0.x')
        if xor!=init+4:raise ValueError('VU random source instruction order differs')
    for addr,dest in ((0x57cadc,0x36f4d4),(0x57cb00,0x36f548),
                       (0x57cbc4,0x4748ac),(0x57cbd8,0x36f4d4),
                       (0x57cbfc,0x36f548),(0x57ccbc,0x4748ac)):
        op=word(addr)
        if op>>26!=3 or ((((addr+4)&0xf0000000)|((op&0x3ffffff)<<2))!=dest):
            raise ValueError('Original source RNG/dot JAL changed '+hex(addr))
    return {'owner_original_ELF_SHA256':SHA,
      'retail_fallback_machine_words':173,
      'original_first_candidate_VU0_instructions':20,
      'original_subsequent_candidate_VU0_instructions':19,
      'original_initial_only_VMOVE':True,
      'original_VU0_Q_external_state_needed':True,
      'original_VMULq_per_candidate':2,
      'candidate_count':6,
      'per_candidate_game_RNG_draws':2,
      'per_candidate_VU0_VRINIT_VRXOR_pairs':2,
      'VU0_R_result_after_candidate_hex':'0x3F800000',
      'unrecovered_original_VU0_candidate_transform':True,
      'decoded_original_VU0_ops':ledger}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_USA_elf',type=Path)
    p.add_argument('--output',type=Path)
    a=p.parse_args()
    result=json.dumps(verify(a.original_USA_elf),indent=2)+'\n'
    if a.output:a.output.write_text(result,encoding='utf-8')
    else:print(result,end='')
