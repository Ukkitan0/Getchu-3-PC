#include "ae3_boot_post_tlb.h"
#include <stddef.h>

static uint64_t reg(const AE3_BootPrefix *s, unsigned n){return n?s->gpr[n][0]:0;}
static void put(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sext32(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* Pinned to the owner's post-boot retail EE memory snapshot. Deliberately
 * excludes the alignment-fast-path MMI instructions not taken by this call. */
static const struct {uint32_t pc,word;} original[]={
 {0x0043fea0u,0xdfbf0000u},{0x0043fea4u,0x081106dcu},
 {0x0043fea8u,0x27bd0010u},
 {0x00441b70u,0x3c02006au},{0x00441b74u,0x3c03006au},
 {0x00441b78u,0x3c040077u},{0x00441b7cu,0xac402e40u},
 {0x00441b80u,0xac602e44u},{0x00441b84u,0x24841288u},
 {0x00441b88u,0x0000282du},{0x00441b8cu,0x0810acc8u},
 {0x00441b90u,0x24060200u},
 {0x0042b320u,0x2cc20008u},{0x0042b324u,0x1440001eu},
 {0x0042b328u,0x0080182du},{0x0042b32cu,0x3082000fu},
 {0x0042b330u,0x1440001bu},{0x0042b334u,0x0080382du},
 {0x0042b3a0u,0x3c02ffffu},{0x0042b3a4u,0x24c6ffffu},
 {0x0042b3a8u,0x3442ffffu},{0x0042b3acu,0x10c2000au},
 {0x0042b3b0u,0x00000000u},{0x0042b3b4u,0x3c02ffffu},
 {0x0042b3b8u,0x3442ffffu},{0x0042b3bcu,0xa0650000u},
 {0x0042b3c0u,0x24c6ffffu},{0x0042b3c4u,0x24630001u},
 {0x0042b3c8u,0x00000000u},{0x0042b3ccu,0x00000000u},
 {0x0042b3d0u,0x14c2fffau},{0x0042b3d4u,0x00000000u},
};
AE3_PostTlbResult ae3_boot_post_tlb_enter(AE3_BootPostTlb *out,
 const AE3_BootTlbFinalize *prior){
 if(!out||!prior||!prior->active||!prior->done||
    prior->entries!=6||prior->sets!=5||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_POST_TLB_ENTRY)return AE3_PTL_INVALID;
 *out=(AE3_BootPostTlb){0};out->active=1;out->cpu=prior->cpu;
 return AE3_PTL_YIELDED;
}
AE3_PostTlbResult ae3_boot_post_tlb_slice(AE3_BootPostTlb *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)
   return AE3_PTL_INVALID;
 if(s->cpu.pc==AE3_POST_TLB_RETURN_PC && !s->cpu.in_delay_slot)
   return (s->zero_stores==AE3_POST_TLB_ZERO_BYTES&&s->global_stores==2)?
       AE3_PTL_RETURN_PENDING:AE3_PTL_UNSUPPORTED;
 for(uint32_t i=0;i<budget;i++){
   uint32_t pc=s->cpu.pc,word=0;
   size_t j=0;for(;j<sizeof original/sizeof original[0];j++)
      if(original[j].pc==pc)break;
   if(j==sizeof original/sizeof original[0])return AE3_PTL_UNSUPPORTED;
   if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)
     return AE3_PTL_MEMORY_FAULT;
   if(word!=original[j].word)return AE3_PTL_UNSUPPORTED;
   AE3_BootPostTlb n=*s;
   unsigned d=n.cpu.in_delay_slot;
   uint32_t dest=d?n.cpu.branch_target:pc+4u;
   if(d){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
   uint32_t address=0;uint32_t width=0,store=0,load=0;
   uint64_t value=0;
   switch(pc){
    case 0x0043fea0u: /* LD ra,0(sp) */
      if(d)return AE3_PTL_UNSUPPORTED;
      address=(uint32_t)reg(&n.cpu,29);width=8;load=1;break;
    case 0x0043fea4u:case 0x00441b8cu:
      if(d)return AE3_PTL_UNSUPPORTED;
      n.cpu.in_delay_slot=1;
      n.cpu.branch_target=(pc==0x0043fea4u)?0x00441b70u:0x0042b320u;
      break;
    case 0x0043fea8u:
      if(!d||dest!=0x00441b70u)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,29,sext32((uint32_t)reg(&n.cpu,29)+0x10u));break;
    case 0x00441b70u:case 0x00441b74u:case 0x00441b78u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,pc==0x00441b70u?2:pc==0x00441b74u?3:4,
        (pc==0x00441b78u)?0x00770000u:0x006a0000u);break;
    case 0x00441b7cu:case 0x00441b80u:
      if(d)return AE3_PTL_UNSUPPORTED;
      address=(uint32_t)reg(&n.cpu,pc==0x00441b7cu?2:3)+
                       (pc==0x00441b7cu?0x2e40u:0x2e44u);
      width=4;store=1;value=0;break;
    case 0x00441b84u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,4,sext32((uint32_t)reg(&n.cpu,4)+0x1288u));break;
    case 0x00441b88u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,5,0);break;
    case 0x00441b90u:
      if(!d||dest!=0x0042b320u)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,6,512);break;
    case 0x0042b320u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,2,reg(&n.cpu,6)<8u);break;
    case 0x0042b324u:
      if(d||reg(&n.cpu,2)!=0)return AE3_PTL_UNSUPPORTED;
      n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0042b32cu;break;
    case 0x0042b328u:
      if(!d||dest!=0x0042b32cu)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,3,reg(&n.cpu,4));break;
    case 0x0042b32cu:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,2,reg(&n.cpu,4)&15u);break;
    case 0x0042b330u:
      if(d||reg(&n.cpu,2)!=8u)return AE3_PTL_UNSUPPORTED;
      n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0042b3a0u;break;
    case 0x0042b334u:
      if(!d||dest!=0x0042b3a0u)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,7,reg(&n.cpu,4));break;
    case 0x0042b3a0u:case 0x0042b3b4u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,2,sext32(0xffff0000u));break;
    case 0x0042b3a4u:case 0x0042b3c0u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,6,sext32((uint32_t)reg(&n.cpu,6)-1u));break;
    case 0x0042b3a8u:case 0x0042b3b8u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,2,reg(&n.cpu,2)|0xffffu);break;
    case 0x0042b3acu:
      if(d||reg(&n.cpu,6)==reg(&n.cpu,2))return AE3_PTL_UNSUPPORTED;
      n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0042b3b4u;break;
    case 0x0042b3b0u:case 0x0042b3c8u:case 0x0042b3ccu:case 0x0042b3d4u:
      if(d!=(pc==0x0042b3b0u||pc==0x0042b3d4u))
         return AE3_PTL_UNSUPPORTED;
      break;
    case 0x0042b3bcu:
      if(d||n.zero_stores>=AE3_POST_TLB_ZERO_BYTES||
         (uint32_t)reg(&n.cpu,3)!=AE3_POST_TLB_ZERO_START+n.zero_stores||
         reg(&n.cpu,5)!=0)return AE3_PTL_UNSUPPORTED;
      address=(uint32_t)reg(&n.cpu,3);width=1;store=1;value=reg(&n.cpu,5);break;
    case 0x0042b3c4u:
      if(d)return AE3_PTL_UNSUPPORTED;
      put(&n.cpu,3,sext32((uint32_t)reg(&n.cpu,3)+1u));break;
    case 0x0042b3d0u:
      if(d)return AE3_PTL_UNSUPPORTED;
      n.cpu.in_delay_slot=1;
      n.cpu.branch_target=reg(&n.cpu,6)!=reg(&n.cpu,2)?0x0042b3bcu:0x0042b3d8u;
      break;
    default:return AE3_PTL_UNSUPPORTED;
   }
   if(width){
     if(address&(width-1u))return AE3_PTL_MEMORY_FAULT;
     if(load){
       uint8_t raw[8]={0};uint64_t result=0;
       if(ae3_ee_bus_read(bus,address,raw,width)!=AE3_EE_BUS_OK)
         return AE3_PTL_MEMORY_FAULT;
       for(unsigned x=0;x<width;x++)result|=(uint64_t)raw[x]<<(8u*x);
       put(&n.cpu,31,result);
     }
     if(store){
       uint8_t raw[8]={0};
       for(unsigned x=0;x<width;x++)raw[x]=(uint8_t)(value>>(8u*x));
       if(ae3_ee_bus_write(bus,address,raw,width)!=AE3_EE_BUS_OK)
         return AE3_PTL_MEMORY_FAULT;
       if(width==1)n.zero_stores++;else n.global_stores++;
     }
   }
   n.cpu.pc=dest;n.cpu.retired++;
   *s=n;
   if(n.cpu.pc==AE3_POST_TLB_RETURN_PC&&!n.cpu.in_delay_slot)
      return (n.zero_stores==AE3_POST_TLB_ZERO_BYTES && n.global_stores==2)?
        AE3_PTL_RETURN_PENDING:AE3_PTL_UNSUPPORTED;
 }
 return AE3_PTL_YIELDED;
}