#include "ae3_boot_main_resume.h"
#include <stddef.h>
static uint64_t reg(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void put(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sext32(uint32_t x){return (uint64_t)(int64_t)(int32_t)x;}
static const struct {uint32_t pc,op;} words[]={
 {0x0042b3d8u,0x03e00008u},{0x0042b3dcu,0x0080102du},
 {0x00100324u,0x0c10d758u},{0x00100328u,0x0000202du},
 {0x00435d60u,0x24030064u},{0x00435d64u,0x0000000cu},
 {0x00435d68u,0x03e00008u},{0x00435d6cu,0x00000000u},
 {0x0010032cu,0x3c100063u},{0x00100330u,0x26108210u},
 {0x00100334u,0x12000003u},{0x00100338u,0x00000000u},
 {0x0010033cu,0x0200f809u}
};
AE3_MainResumeResult ae3_boot_main_resume_enter(AE3_BootMainResume*out,
 const AE3_BootPostTlb*prior){
 if(!out||!prior||!prior->active||prior->zero_stores!=512u||
   prior->global_stores!=2u||prior->cpu.pc!=AE3_POST_TLB_RETURN_PC||
   prior->cpu.in_delay_slot||
   (uint32_t)reg(&prior->cpu,31)!=AE3_MAIN_CALLER_PC)
   return AE3_MAIN_INVALID;
 *out=(AE3_BootMainResume){0};out->cpu=prior->cpu;out->active=1;
 return AE3_MAIN_YIELDED;
}
AE3_MainResumeResult ae3_boot_main_resume_slice(AE3_BootMainResume*s,
 AE3_EEMemoryBus*bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_MAIN_INVALID;
 if(s->cpu.pc==AE3_MAIN_NEXT_JALR&&!s->cpu.in_delay_slot)
   return s->flush_requests==1&&
      (uint32_t)reg(&s->cpu,16)==AE3_MAIN_DYNAMIC_TARGET?
       AE3_MAIN_NEXT_CALL:AE3_MAIN_UNSUPPORTED;
 if(s->cpu.pc==AE3_MAIN_FLUSH_SYSCALL&&!s->cpu.in_delay_slot)
   return AE3_MAIN_FLUSH_PENDING;
 for(uint32_t i=0;i<budget;i++){
   uint32_t pc=s->cpu.pc,w=0;size_t j=0;
   for(;j<sizeof words/sizeof words[0];j++)if(words[j].pc==pc)break;
   if(j==sizeof words/sizeof words[0])return AE3_MAIN_UNSUPPORTED;
   if(ae3_ee_bus_read32(bus,pc,&w)!=AE3_EE_BUS_OK)
     return AE3_MAIN_MEMORY_FAULT;
   if(w!=words[j].op)return AE3_MAIN_UNSUPPORTED;
   AE3_BootMainResume n=*s;
   unsigned d=n.cpu.in_delay_slot;
   uint32_t dest=d?n.cpu.branch_target:pc+4u;
   if(d){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
   switch(pc){
    case 0x0042b3d8u:
      if(d)return AE3_MAIN_UNSUPPORTED;
      n.cpu.in_delay_slot=1;n.cpu.branch_target=AE3_MAIN_CALLER_PC;break;
    case 0x0042b3dcu:
      if(!d||dest!=AE3_MAIN_CALLER_PC)return AE3_MAIN_UNSUPPORTED;
      put(&n.cpu,2,reg(&n.cpu,4));break;
    case 0x00100324u:
      if(d)return AE3_MAIN_UNSUPPORTED;
      put(&n.cpu,31,0x0010032cu);
      n.cpu.in_delay_slot=1;n.cpu.branch_target=0x00435d60u;break;
    case 0x00100328u:
      if(!d||dest!=0x00435d60u)return AE3_MAIN_UNSUPPORTED;
      put(&n.cpu,4,0);break;
    case 0x00435d60u:
      if(d)return AE3_MAIN_UNSUPPORTED;
      put(&n.cpu,3,0x64u);break;
    case 0x00435d68u:
      if(d||n.flush_requests!=1u||reg(&n.cpu,31)!=0x0010032cu)
         return AE3_MAIN_UNSUPPORTED;
      n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0010032cu;break;
    case 0x00435d6cu:
      if(!d||dest!=0x0010032cu)return AE3_MAIN_UNSUPPORTED;
      break;
    case 0x0010032cu:
      if(d)return AE3_MAIN_UNSUPPORTED;
      put(&n.cpu,16,0x00630000u);break;
    case 0x00100330u:
      if(d)return AE3_MAIN_UNSUPPORTED;
      put(&n.cpu,16,sext32((uint32_t)reg(&n.cpu,16)-0x7df0u));break;
    case 0x00100334u:
      if(d||reg(&n.cpu,16)!=AE3_MAIN_DYNAMIC_TARGET)
         return AE3_MAIN_UNSUPPORTED;
      n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0010033cu;break;
    case 0x00100338u:
      if(!d||dest!=0x0010033cu)return AE3_MAIN_UNSUPPORTED;
      break;
    default:return AE3_MAIN_UNSUPPORTED;
   }
   n.cpu.pc=dest;n.cpu.retired++;*s=n;
   if(dest==AE3_MAIN_FLUSH_SYSCALL&&!n.cpu.in_delay_slot)
     return AE3_MAIN_FLUSH_PENDING;
   if(dest==AE3_MAIN_NEXT_JALR&&!n.cpu.in_delay_slot)
     return n.flush_requests==1u ? AE3_MAIN_NEXT_CALL:AE3_MAIN_UNSUPPORTED;
 }
 return AE3_MAIN_YIELDED;
}
AE3_MainResumeResult ae3_boot_main_resume_flush(AE3_BootMainResume*s,
 const AE3_EEMemoryBus*bus){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot||
     s->cpu.pc!=AE3_MAIN_FLUSH_SYSCALL||reg(&s->cpu,3)!=0x64u||
     reg(&s->cpu,4)!=0||s->flush_requests)return AE3_MAIN_INVALID;
 uint32_t word=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&word)!=AE3_EE_BUS_OK)
   return AE3_MAIN_MEMORY_FAULT;
 if(word!=12u)return AE3_MAIN_UNSUPPORTED;
 /* One compatibility-mode kernel service: records a request, not caches. */
 AE3_BootMainResume n=*s;
 n.cpu.pc+=4u;n.cpu.retired++;n.flush_requests=1u;n.flush_mode=0u;
 *s=n;return AE3_MAIN_YIELDED;
}