#include "ae3_boot_second_constructor.h"
#include <stddef.h>
/* Explicit pinned retail opcode allowlist. Never execute synthetic or unknown
   instructions; the bytes are fetched again through the caller's guest bus. */
static const struct {uint32_t pc, word;} code[]={
 {UINT32_C(0x004173B0),UINT32_C(0x03E00008)},
 {UINT32_C(0x004173B4),UINT32_C(0x00000000)},
 {UINT32_C(0x00417B50),UINT32_C(0x27BDFFE0)},
 {UINT32_C(0x00417B54),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417B58),UINT32_C(0xFFB10008)},
 {UINT32_C(0x00417B5C),UINT32_C(0xFFB20010)},
 {UINT32_C(0x00417B60),UINT32_C(0xFFB00000)},
 {UINT32_C(0x00417B64),UINT32_C(0x00A0882D)},
 {UINT32_C(0x00417B68),UINT32_C(0xFFBF0018)},
 {UINT32_C(0x00417B6C),UINT32_C(0x14AF0008)},
 {UINT32_C(0x00417B70),UINT32_C(0x0080902D)},
 {UINT32_C(0x00417B74),UINT32_C(0x240F0001)},
 {UINT32_C(0x00417B78),UINT32_C(0x148F0005)},
 {UINT32_C(0x00417B7C),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417B80),UINT32_C(0x3C04006A)},
 {UINT32_C(0x00417B84),UINT32_C(0x0C105CEC)},
 {UINT32_C(0x00417B88),UINT32_C(0x2484E628)},
 {UINT32_C(0x00417B8C),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417B90),UINT32_C(0x162F0008)},
 {UINT32_C(0x00417B94),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417B98),UINT32_C(0x240F0001)},
 {UINT32_C(0x00417B9C),UINT32_C(0x164F0005)},
 {UINT32_C(0x00417BA0),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417BA4),UINT32_C(0x3C04006A)},
 {UINT32_C(0x00417BA8),UINT32_C(0x0C105CEC)},
 {UINT32_C(0x00417BAC),UINT32_C(0x2484E630)},
 {UINT32_C(0x00417BB0),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417BB4),UINT32_C(0x162F0011)},
 {UINT32_C(0x00417BB8),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417BBC),UINT32_C(0x240F0001)},
 {UINT32_C(0x00417BC0),UINT32_C(0x164F000E)},
 {UINT32_C(0x00417BC4),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417BC8),UINT32_C(0x24041009)},
 {UINT32_C(0x00417BCC),UINT32_C(0x0C1075AE)},
 {UINT32_C(0x00417BD0),UINT32_C(0x000424BC)},
 {UINT32_C(0x00417BD4),UINT32_C(0x24040403)},
 {UINT32_C(0x00417BD8),UINT32_C(0x0040802D)},
 {UINT32_C(0x00417BDC),UINT32_C(0x0C1075AE)},
 {UINT32_C(0x00417BE0),UINT32_C(0x0004253C)},
 {UINT32_C(0x00417BE4),UINT32_C(0x0040282D)},
 {UINT32_C(0x00417BE8),UINT32_C(0x0C109036)},
 {UINT32_C(0x00417BEC),UINT32_C(0x0200202D)},
 {UINT32_C(0x00417BF0),UINT32_C(0x3C0F006A)},
 {UINT32_C(0x00417BF4),UINT32_C(0xFDE2E678)},
 {UINT32_C(0x00417BF8),UINT32_C(0x340FFFFF)},
 {UINT32_C(0x00417BFC),UINT32_C(0x162F0011)},
 {UINT32_C(0x00417C00),UINT32_C(0xDFB00000)},
 {UINT32_C(0x00417C04),UINT32_C(0x240F0001)},
 {UINT32_C(0x00417C08),UINT32_C(0x164F000F)},
 {UINT32_C(0x00417C0C),UINT32_C(0xDFB10008)},
 {UINT32_C(0x00417C10),UINT32_C(0x24041009)},
 {UINT32_C(0x00417C14),UINT32_C(0x0C1075AE)},
 {UINT32_C(0x00417C18),UINT32_C(0x000424BC)},
 {UINT32_C(0x00417C1C),UINT32_C(0x24040201)},
 {UINT32_C(0x00417C20),UINT32_C(0x0040802D)},
 {UINT32_C(0x00417C24),UINT32_C(0x0C1075AE)},
 {UINT32_C(0x00417C28),UINT32_C(0x0004257C)},
 {UINT32_C(0x00417C2C),UINT32_C(0x0040282D)},
 {UINT32_C(0x00417C30),UINT32_C(0x0C109036)},
 {UINT32_C(0x00417C34),UINT32_C(0x0200202D)},
 {UINT32_C(0x00417C38),UINT32_C(0x3C0F006A)},
 {UINT32_C(0x00417C3C),UINT32_C(0xFDE2E680)},
 {UINT32_C(0x00417C40),UINT32_C(0xDFB00000)},
 {UINT32_C(0x00417C44),UINT32_C(0xDFB10008)},
 {UINT32_C(0x00417C48),UINT32_C(0xDFB20010)},
 {UINT32_C(0x00417C4C),UINT32_C(0xDFBF0018)},
 {UINT32_C(0x00417C50),UINT32_C(0x03E00008)},
 {UINT32_C(0x00417C54),UINT32_C(0x27BD0020)},
 {UINT32_C(0x00417C58),UINT32_C(0x27BDFFF0)},
 {UINT32_C(0x00417C5C),UINT32_C(0x24040001)},
 {UINT32_C(0x00417C60),UINT32_C(0xFFBF0000)},
 {UINT32_C(0x00417C64),UINT32_C(0x3405FFFF)},
 {UINT32_C(0x00417C68),UINT32_C(0xDFBF0000)},
 {UINT32_C(0x00417C6C),UINT32_C(0x08105ED4)},
 {UINT32_C(0x00417C70),UINT32_C(0x27BD0010)},
 {UINT32_C(0x0041D6B8),UINT32_C(0x27BDFFF0)},
 {UINT32_C(0x0041D6BC),UINT32_C(0xFFBF0000)},
 {UINT32_C(0x0041D6C0),UINT32_C(0xDFBF0000)},
 {UINT32_C(0x0041D6C4),UINT32_C(0x08107BF4)},
 {UINT32_C(0x0041D6C8),UINT32_C(0x27BD0010)},
 {UINT32_C(0x00428AE8),UINT32_C(0x01C0F809)},
 {UINT32_C(0x00428AEC),UINT32_C(0x2610FFFC)},
};
static uint64_t reg(const AE3_BootPrefix *s,unsigned id){return id?s->gpr[id][0]:0;}
static void put(AE3_BootPrefix *s,unsigned id,uint64_t v){if(id)s->gpr[id][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static uint32_t jt(uint32_t pc,uint32_t w){
 return ((pc+4u)&UINT32_C(0xF0000000))|((w&UINT32_C(0x03FFFFFF))<<2);
}
static AE3_EEBusStatus load64(const AE3_EEMemoryBus *bus,uint32_t addr,uint64_t *v){
 uint8_t b[8];if(addr&7u)return AE3_EE_BUS_MISALIGNED;
 AE3_EEBusStatus result=ae3_ee_bus_read(bus,addr,b,8);if(result!=AE3_EE_BUS_OK)return result;
 *v=0;for(unsigned i=0;i<8;i++)*v|=(uint64_t)b[i]<<(8*i);
 return AE3_EE_BUS_OK;
}
static AE3_EEBusStatus store64(AE3_EEMemoryBus *bus,uint32_t addr,uint64_t v){
 uint8_t b[8];if(addr&7u)return AE3_EE_BUS_MISALIGNED;
 for(unsigned i=0;i<8;i++)b[i]=(uint8_t)(v>>(8*i));
 return ae3_ee_bus_write(bus,addr,b,8);
}
AE3_SecondCtorResult ae3_boot_second_ctor_enter(AE3_BootSecondConstructor *out,
 const AE3_BootFirstConstructor *first){
 if(!out||!first||!first->active||first->cpu.in_delay_slot||
    first->cpu.pc!=UINT32_C(0x00428AE8)||
    first->next_constructor!=AE3_SECOND_CTOR_PC||
    (uint32_t)reg(&first->cpu,14)!=AE3_SECOND_CTOR_PC||
    (uint32_t)reg(&first->cpu,31)!=UINT32_C(0x00428AF0))
    return AE3_SC_INVALID;
 *out=(AE3_BootSecondConstructor){0};out->cpu=first->cpu;
 out->saved_first_stores=first->reference_count_stores;
 out->active=1;
 return AE3_SC_YIELDED;
}
AE3_SecondCtorResult ae3_boot_second_ctor_slice(AE3_BootSecondConstructor *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_SC_INVALID;
 for(uint32_t i=0;i<budget;i++){
  const uint32_t pc=s->cpu.pc;
  if(pc==AE3_SECOND_NUMERIC_BOUNDARY && !s->cpu.in_delay_slot)
    return AE3_SC_NUMERIC_PENDING;
  size_t j=0;for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
  if(j==sizeof(code)/sizeof(code[0]))return AE3_SC_UNSUPPORTED;
  uint32_t op=0;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)return AE3_SC_MEMORY_FAULT;
  if(op!=code[j].word)return AE3_SC_UNSUPPORTED;
  AE3_BootSecondConstructor next=*s;
  const unsigned delay=next.cpu.in_delay_slot;
  uint32_t target=delay?next.cpu.branch_target:pc+4u;
  if(delay){next.cpu.in_delay_slot=0;next.cpu.branch_target=0;}
  const unsigned major=op>>26,rs=(op>>21)&31u,rt=(op>>16)&31u;
  const unsigned rd=(op>>11)&31u,shamt=(op>>6)&31u,fn=op&63u;
  const int32_t imm=(int16_t)(op&0xffffu);
  uint32_t address=0;uint64_t value=0;
  unsigned do_read=0,do_write=0;
  if(!op){/* retail NOP */}
  else if(!major){
   if(fn==0x08 && rt==0 && rd==0 && !delay){
    next.cpu.in_delay_slot=1;next.cpu.branch_target=(uint32_t)reg(&next.cpu,rs);
   }else if(fn==0x09 && !delay && rt==0){
    next.cpu.in_delay_slot=1;next.cpu.branch_target=(uint32_t)reg(&next.cpu,rs);
    put(&next.cpu,rd,pc+8u);
   }else if(fn==0x2d && (op&UINT32_C(0x7C0))==0){
    put(&next.cpu,rd,reg(&next.cpu,rs)+reg(&next.cpu,rt));
   }else if(fn==0x3c && rs==0){
    put(&next.cpu,rd,reg(&next.cpu,rt)<<(shamt+32u));
   }else return AE3_SC_UNSUPPORTED;
  }else if(major==0x09){
    put(&next.cpu,rt,sx((uint32_t)reg(&next.cpu,rs)+(uint32_t)imm));
  }else if(major==0x19){
    put(&next.cpu,rt,reg(&next.cpu,rs)+(uint64_t)(int64_t)imm);
  }else if(major==0x0d){
    put(&next.cpu,rt,reg(&next.cpu,rs)|(op&UINT32_C(0xFFFF)));
  }else if(major==0x0f && rs==0){
    put(&next.cpu,rt,sx((op&UINT32_C(0xFFFF))<<16));
  }else if((major==0x04||major==0x05) && !delay){
    const int taken=(major==4)?reg(&next.cpu,rs)==reg(&next.cpu,rt):
                              reg(&next.cpu,rs)!=reg(&next.cpu,rt);
    next.cpu.in_delay_slot=1;
    next.cpu.branch_target=taken?
       (uint32_t)((int64_t)(pc+4u)+(int64_t)imm*4):pc+8u;
  }else if((major==0x02||major==0x03) && !delay){
    next.cpu.in_delay_slot=1;next.cpu.branch_target=jt(pc,op);
    if(major==3)put(&next.cpu,31,pc+8u);
  }else if(major==0x37||major==0x3f){
    address=(uint32_t)(reg(&next.cpu,rs)+(uint64_t)(int64_t)imm);
    do_read=major==0x37;do_write=!do_read;
  }else return AE3_SC_UNSUPPORTED;
  if(do_read){
    if(load64(bus,address,&value)!=AE3_EE_BUS_OK)return AE3_SC_MEMORY_FAULT;
    put(&next.cpu,rt,value);
  }
  if(do_write && store64(bus,address,reg(&next.cpu,rt))!=AE3_EE_BUS_OK)
     return AE3_SC_MEMORY_FAULT;
  if(pc==AE3_FIRST_CTOR_STUB)next.stub_calls++;
  if(pc==AE3_SECOND_NUMERIC_WRAPPER)next.wrapper_visits++;
  next.cpu.pc=target;next.cpu.retired++;
  *s=next;
  if(!next.cpu.in_delay_slot && next.cpu.pc==AE3_SECOND_NUMERIC_BOUNDARY)
    return AE3_SC_NUMERIC_PENDING;
 }
 return AE3_SC_YIELDED;
}