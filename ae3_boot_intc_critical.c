#include "ae3_boot_intc_critical.h"
#include <stddef.h>
static uint64_t reg(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void put(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t n){return (uint64_t)(int64_t)(int32_t)n;}
static const struct {uint32_t pc,op;} instructions[]={
 {0x00440578,0x0c10fee8},{0x0044057c,0xac430008},
 {0x0043fba0,0x40036000},{0x0043fba4,0x3c020001},
 {0x0043fba8,0x00621824},{0x0043fbac,0x1060000d},
 {0x0043fbb0,0x0003202b},
 {0x0043fbe4,0x0000202d},{0x0043fbe8,0x03e00008},
 {0x0043fbec,0x0080102d},
 {0x00440580,0x3c031000},{0x00440584,0x0040882d},
 {0x00440588,0x34631010},{0x0044058c,0x3c02ffff}
};
AE3_IntcCriticalResult ae3_boot_intc_critical_enter(AE3_BootIntcCritical *s,
 const AE3_BootPostIntc *prior,uint32_t cp0_status){
 if(!s||!prior||!prior->serviced||!prior->handler_id||
    prior->cpu.pc!=AE3_POST_INTC_NEXT_PC||prior->cpu.in_delay_slot)
    return AE3_INTC_CRITICAL_INVALID;
 /* The EIE-enabled path includes EI and SYNC and requires a separate
    COP0 implementation. Refuse it, rather than approximating timing. */
 if(cp0_status&UINT32_C(0x10000))return AE3_INTC_CRITICAL_UNSUPPORTED;
 *s=(AE3_BootIntcCritical){0};s->cpu=prior->cpu;
 s->cop0_status=cp0_status;s->enabled=1;
 return AE3_INTC_CRITICAL_YIELDED;
}
AE3_IntcCriticalResult ae3_boot_intc_critical_slice(AE3_BootIntcCritical *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->enabled||s->cpu.in_delay_slot>1)
    return AE3_INTC_CRITICAL_INVALID;
 if(s->cpu.pc==AE3_T2_MODE_GUEST_LW)return AE3_INTC_CRITICAL_T2_MODE_PENDING;
 for(uint32_t i=0;i<budget;i++){
    uint32_t pc=s->cpu.pc,op=0;
    size_t j=0;
    for(;j<sizeof(instructions)/sizeof(instructions[0]);j++)
      if(instructions[j].pc==pc)break;
    if(j==sizeof(instructions)/sizeof(instructions[0]))return AE3_INTC_CRITICAL_UNSUPPORTED;
    if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)return AE3_INTC_CRITICAL_MEMORY_FAULT;
    if(op!=instructions[j].op)return AE3_INTC_CRITICAL_UNSUPPORTED;
    AE3_BootPrefix next=s->cpu;
    int delay=next.in_delay_slot;
    uint32_t dest=delay?next.branch_target:pc+4;
    if(delay){next.in_delay_slot=0;next.branch_target=0;}
    switch(pc){
      case 0x00440578:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,31,0x00440580);next.in_delay_slot=1;
        next.branch_target=0x0043fba0;break;
      case 0x0044057c:{
        if(!delay||dest!=0x0043fba0)return AE3_INTC_CRITICAL_UNSUPPORTED;
        const uint32_t a=(uint32_t)reg(&next,2)+8u;
        if((a&3u)||ae3_ee_bus_write32(bus,a,(uint32_t)reg(&next,3))!=AE3_EE_BUS_OK)
          return AE3_INTC_CRITICAL_MEMORY_FAULT;
        break;}
      case 0x0043fba0:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,3,s->cop0_status);break;
      case 0x0043fba4:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,2,sx(0x10000));break;
      case 0x0043fba8:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,3,reg(&next,3)&reg(&next,2));break;
      case 0x0043fbac:
        if(delay||reg(&next,3)!=0)return AE3_INTC_CRITICAL_UNSUPPORTED;
        next.in_delay_slot=1;next.branch_target=0x0043fbe4;break;
      case 0x0043fbb0:
        if(!delay||dest!=0x0043fbe4)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,4,reg(&next,3)!=0);break;
      case 0x0043fbe4:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,4,0);break;
      case 0x0043fbe8:
        if(delay||reg(&next,31)!=0x00440580u)return AE3_INTC_CRITICAL_UNSUPPORTED;
        next.in_delay_slot=1;next.branch_target=(uint32_t)reg(&next,31);break;
      case 0x0043fbec:
        if(!delay||dest!=0x00440580u)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,2,reg(&next,4));break;
      case 0x00440580:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,3,sx(0x10000000u));break;
      case 0x00440584:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,17,reg(&next,2));break;
      case 0x00440588:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,3,reg(&next,3)|0x1010u);break;
      case 0x0044058c:
        if(delay)return AE3_INTC_CRITICAL_UNSUPPORTED;
        put(&next,2,sx(0xffff0000u));break;
      default:return AE3_INTC_CRITICAL_UNSUPPORTED;
    }
    next.pc=dest;next.retired++;
    s->cpu=next;
    if(dest==AE3_T2_MODE_GUEST_LW){
      if((uint32_t)reg(&s->cpu,3)!=AE3_T2_MODE_REGISTER)
        return AE3_INTC_CRITICAL_UNSUPPORTED;
      return AE3_INTC_CRITICAL_T2_MODE_PENDING;
    }
 }
 return AE3_INTC_CRITICAL_YIELDED;
}