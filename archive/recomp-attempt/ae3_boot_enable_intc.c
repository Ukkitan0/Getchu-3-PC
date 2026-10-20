#include "ae3_boot_enable_intc.h"
static uint64_t reg(const AE3_BootPrefix *c,unsigned n){return n?c->gpr[n][0]:0;}
static void set(AE3_BootPrefix *c,unsigned n,uint64_t v){if(n)c->gpr[n][0]=v;}
static uint64_t sx32(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* Bytes pinned from the original SCUS_975.01 instruction inventory.
 * Unknown opcodes fail closed. No game binary is distributed. */
static const struct {uint32_t pc,word;} ops[]={
 {0x004405c8,0x0c10d98a},{0x004405cc,0x2404000b},
 {0x00436628,0x27bdffd0},{0x0043662c,0xffb10010},
 {0x00436630,0xffbf0020},{0x00436634,0x0080882d},
 {0x00436638,0xffb00000},{0x0043663c,0x40106000},
 {0x00436640,0x3c020001},{0x00436644,0x02028024},
 {0x00436648,0x12000003},{0x0043664c,0x00000000},
 {0x00436658,0x0c10d608},{0x0043665c,0x0220202d},
 {0x00435820,0x24030014},{0x00435824,0x0000000c},
 {0x00435828,0x03e00008},{0x0043582c,0x00000000},
 {0x00436660,0x0040882d},{0x00436664,0x0000000f},
 {0x00436668,0x12000004},{0x0043666c,0x0220102d},
 {0x0043667c,0xdfbf0020},{0x00436680,0xdfb10010},
 {0x00436684,0xdfb00000},{0x00436688,0x03e00008},
 {0x0043668c,0x27bd0030},{0x004405d0,0x12200004},
 {0x004405d4,0x0000102d},{0x004405e4,0xdfbf0050},
 {0x004405e8,0xdfb40040},{0x004405ec,0xdfb30030},
 {0x004405f0,0xdfb20020},{0x004405f4,0xdfb10010},
 {0x004405f8,0xdfb00000},{0x004405fc,0x03e00008},
 {0x00440600,0x27bd0060}
};
static AE3_EnableIntcResult event(const AE3_BootEnableIntc *s){
 if(s->cpu.in_delay_slot)return AE3_EINTC_YIELDED;
 if(s->cpu.pc==AE3_ENABLE_INTC_SYSCALL)return AE3_EINTC_SYSCALL_PENDING;
 if(s->cpu.pc==AE3_ENABLE_INTC_PARENT_RETURN)return AE3_EINTC_PARENT_RETURNED;
 if(s->cpu.pc==AE3_ENABLE_INTC_EI_BOUNDARY)return AE3_EINTC_EI_PENDING;
 return AE3_EINTC_YIELDED;
}
AE3_EnableIntcResult ae3_boot_enable_intc_enter(AE3_BootEnableIntc *s,
    const AE3_BootTimer2Setup *prior){
 if(!s||!prior||!prior->active||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_ENABLE_INTC_CALL||prior->cop0_status&UINT32_C(0x10000))
    return AE3_EINTC_INVALID;
 *s=(AE3_BootEnableIntc){0};s->cpu=prior->cpu;
 s->cop0_status=prior->cop0_status;s->active=1;
 return AE3_EINTC_YIELDED;
}
AE3_EnableIntcResult ae3_boot_enable_intc_slice(AE3_BootEnableIntc *s,
    AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1u)
    return AE3_EINTC_INVALID;
 if(event(s)!=AE3_EINTC_YIELDED)return event(s);
 for(uint32_t i=0;i<budget;i++){
    const uint32_t pc=s->cpu.pc;
    unsigned j=0;
    while(j<sizeof(ops)/sizeof(ops[0])&&ops[j].pc!=pc)j++;
    if(j==sizeof(ops)/sizeof(ops[0]))return AE3_EINTC_UNSUPPORTED;
    uint32_t word=0;
    if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)
        return AE3_EINTC_MEMORY_FAULT;
    if(word!=ops[j].word)return AE3_EINTC_UNSUPPORTED;
    AE3_BootPrefix n=s->cpu;
    const int delay=n.in_delay_slot;
    uint32_t nextpc=delay?n.branch_target:pc+4u;
    if(delay){n.in_delay_slot=0;n.branch_target=0;}
    uint32_t access=0;unsigned access_reg=0;int access_store=0,access_load=0;
    switch(pc){
      case 0x004405c8:case 0x00436658:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,31,pc+8u);n.in_delay_slot=1;
         n.branch_target=pc==0x004405c8?0x00436628u:0x00435820u;
         break;
      case 0x004405cc:
         if(!delay||nextpc!=0x00436628u)return AE3_EINTC_UNSUPPORTED;
         set(&n,4,11);break;
      case 0x00436628:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,29,sx32((uint32_t)reg(&n,29)-0x30u));break;
      case 0x0043662c:case 0x00436630:case 0x00436638:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         access=(uint32_t)reg(&n,29)+(pc==0x0043662c?0x10u:pc==0x00436630?0x20u:0);
         access_reg=pc==0x0043662c?17u:pc==0x00436630?31u:16u;
         access_store=1;break;
      case 0x00436634:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,17,reg(&n,4));break;
      case 0x0043663c:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,16,sx32(s->cop0_status));break;
      case 0x00436640:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,2,sx32(0x10000u));break;
      case 0x00436644:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,16,reg(&n,16)&reg(&n,2));break;
      case 0x00436648:
         if(delay||reg(&n,16)!=0)return AE3_EINTC_UNSUPPORTED;
         n.in_delay_slot=1;n.branch_target=0x00436658u;break;
      case 0x0043664c:case 0x0043582c:
         if(!delay)return AE3_EINTC_UNSUPPORTED;
         break;
      case 0x0043665c:
         if(!delay||nextpc!=0x00435820u)return AE3_EINTC_UNSUPPORTED;
         set(&n,4,reg(&n,17));break;
      case 0x00435820:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,3,0x14u);break;
      case 0x00435824:return AE3_EINTC_SYSCALL_PENDING;
      case 0x00435828:case 0x00436688:case 0x004405fc:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         n.in_delay_slot=1;
         n.branch_target=(uint32_t)reg(&n,31);
         if((pc==0x00435828&&n.branch_target!=0x00436660u)||
            (pc==0x00436688&&n.branch_target!=0x004405d0u)||
            (pc==0x004405fc&&n.branch_target!=AE3_ENABLE_INTC_PARENT_RETURN))
            return AE3_EINTC_UNSUPPORTED;
         break;
      case 0x00436660:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,17,reg(&n,2));break;
      case 0x00436664:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         break;/* SYNC (only single-threaded test kernel). */
      case 0x00436668:
         if(delay||reg(&n,16)!=0)return AE3_EINTC_UNSUPPORTED;
         n.in_delay_slot=1;n.branch_target=0x0043667cu;break;
      case 0x0043666c:
         if(!delay||nextpc!=0x0043667cu)return AE3_EINTC_UNSUPPORTED;
         set(&n,2,reg(&n,17));break;
      case 0x0043667c:case 0x00436680:case 0x00436684:
      case 0x004405e4:case 0x004405e8:case 0x004405ec:
      case 0x004405f0:case 0x004405f4:case 0x004405f8:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         if(pc>=0x004405e4u){
             const unsigned idx=(pc-0x004405e4u)/4u;
             static const unsigned rr[]={31,20,19,18,17,16};
             access_reg=rr[idx];access=(uint32_t)reg(&n,29)+0x50u-idx*0x10u;
         }else{
             access_reg=pc==0x0043667c?31u:pc==0x00436680?17u:16u;
             access=(uint32_t)reg(&n,29)+(pc==0x0043667c?0x20u:
                    pc==0x00436680?0x10u:0);
         }
         access_load=1;break;
      case 0x0043668c:
         if(!delay||nextpc!=0x004405d0u)return AE3_EINTC_UNSUPPORTED;
         set(&n,29,sx32((uint32_t)reg(&n,29)+0x30u));break;
      case 0x004405d0:
         if(delay)return AE3_EINTC_UNSUPPORTED;
         n.in_delay_slot=1;n.branch_target=reg(&n,17)==0?
                 0x004405e4u:AE3_ENABLE_INTC_EI_BOUNDARY;
         break;
      case 0x004405d4:
         if(!delay)return AE3_EINTC_UNSUPPORTED;
         set(&n,2,0);break;
      case 0x00440600:
         if(!delay||nextpc!=AE3_ENABLE_INTC_PARENT_RETURN)
            return AE3_EINTC_UNSUPPORTED;
         set(&n,29,sx32((uint32_t)reg(&n,29)+0x60u));break;
      default:return AE3_EINTC_UNSUPPORTED;
    }
    if(access_load||access_store){
        uint8_t raw[8];
        if(access&7u)return AE3_EINTC_MEMORY_FAULT;
        if(access_store){
            const uint64_t v=reg(&n,access_reg);
            for(unsigned k=0;k<8;k++)raw[k]=(uint8_t)(v>>(k*8u));
            if(ae3_ee_bus_write(bus,access,raw,8)!=AE3_EE_BUS_OK)
                return AE3_EINTC_MEMORY_FAULT;
        }else{
            if(ae3_ee_bus_read(bus,access,raw,8)!=AE3_EE_BUS_OK)
                return AE3_EINTC_MEMORY_FAULT;
            uint64_t v=0;
            for(unsigned k=0;k<8;k++)v|=(uint64_t)raw[k]<<(k*8u);
            set(&n,access_reg,v);
        }
    }
    n.pc=nextpc;n.retired++;s->cpu=n;
    if(event(s)!=AE3_EINTC_YIELDED)return event(s);
 }
 return AE3_EINTC_YIELDED;
}
AE3_EnableIntcResult ae3_boot_enable_intc_service(AE3_BootEnableIntc *s,
    AE3_EEMemoryBus *bus,AE3_EEIntcKernel *kernel){
 if(!s||!bus||!kernel||!s->active||s->serviced_count||
    s->cpu.in_delay_slot||s->cpu.pc!=AE3_ENABLE_INTC_SYSCALL||
    reg(&s->cpu,3)!=0x14u||reg(&s->cpu,4)!=11u)
    return AE3_EINTC_INVALID;
 uint32_t original=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&original)!=AE3_EE_BUS_OK)
    return AE3_EINTC_MEMORY_FAULT;
 if(original!=0x0000000cu)return AE3_EINTC_UNSUPPORTED;
 /* Explicitly experimental kernel choice: success 0, failure -1.
    No hardware INTC_MASK or Sony kernel syscall is emulated. */
 const int32_t answer=ae3_ee_intc_enable(kernel,(uint32_t)reg(&s->cpu,4));
 set(&s->cpu,2,sx32((uint32_t)answer));
 s->cpu.pc+=4u;s->cpu.retired++;
 s->last_return=answer;s->serviced_count++;
 return AE3_EINTC_YIELDED;
}
