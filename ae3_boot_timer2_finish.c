#include "ae3_boot_timer2_finish.h"
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* Exact words from US retail SCUS_975.01 text inventory. */
static const struct {uint32_t pc,word;} words[]={
 {0x00440710,0x8c420000},{0x00440714,0x30430080},
 {0x00440718,0x10600007},{0x0044071c,0x00000000},
 {0x00440720,0x12000013},{0x00440724,0x24020001},
 {0x00440738,0x3c04ffff},{0x0044073c,0x3484f3ff},
 {0x00440740,0x00442024},
 {0x00440770,0xdfbf0010},{0x00440774,0xdfb00000},
 {0x00440778,0x03e00008},{0x0044077c,0x27bd0020}
};
static AE3_T2FinishResult pending(const AE3_BootTimer2Finish *s){
 if(s->cpu.in_delay_slot)return AE3_T2F_YIELDED;
 switch(s->cpu.pc){
 case AE3_T2_FINISH_PARENT_NEXT:return AE3_T2F_RETURNED;
 case AE3_T2_FINISH_EINTR_CALL:return AE3_T2F_EINTR_PENDING;
 case AE3_T2_FINISH_MODE_CALL:return AE3_T2F_SET_MODE_PENDING;
 case AE3_T2_FINISH_NEXT_SERVICE:return AE3_T2F_NEXT_SERVICE_PENDING;
 default:return AE3_T2F_YIELDED;
 }
}
AE3_T2FinishResult ae3_boot_timer2_finish_enter(AE3_BootTimer2Finish *s,
 const AE3_BootTimer2Recheck *prior){
 if(!s||!prior||!prior->active||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_T2_RECHECK_MMIO_LW||
    (uint32_t)r(&prior->cpu,2)!=AE3_T2_MODE||
    (prior->cop0_status&UINT32_C(0x10000)))return AE3_T2F_INVALID;
 *s=(AE3_BootTimer2Finish){0};s->cpu=prior->cpu;
 s->cop0_status=prior->cop0_status;s->entered=1;
 return AE3_T2F_YIELDED;
}
AE3_T2FinishResult ae3_boot_timer2_finish_slice(AE3_BootTimer2Finish *s,
 const AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->entered||s->cpu.in_delay_slot>1)return AE3_T2F_INVALID;
 if(pending(s)!=AE3_T2F_YIELDED)return pending(s);
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,word=0;size_t j=0;
  for(;j<sizeof(words)/sizeof(words[0]);j++)if(words[j].pc==pc)break;
  if(j==sizeof(words)/sizeof(words[0]))return AE3_T2F_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_T2F_MEMORY_FAULT;
  if(word!=words[j].word)return AE3_T2F_UNSUPPORTED;
  AE3_BootTimer2Finish n=*s;
  int delay=n.cpu.in_delay_slot;
  uint32_t target=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  switch(pc){
  case 0x00440710:{
   uint32_t v=0;
   if(delay||r(&n.cpu,2)!=AE3_T2_MODE)return AE3_T2F_UNSUPPORTED;
   if(ae3_ee_bus_read32(bus,(uint32_t)r(&n.cpu,2),&v)!=AE3_EE_BUS_OK)
       return AE3_T2F_MEMORY_FAULT;
   w(&n.cpu,2,sx(v));n.observed_mode=v;n.read_performed=1;break;
  }
  case 0x00440714:
   if(delay||!n.read_performed)return AE3_T2F_UNSUPPORTED;
   w(&n.cpu,3,r(&n.cpu,2)&0x80u);break;
  case 0x00440718:
   if(delay)return AE3_T2F_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=r(&n.cpu,3)?0x00440720u:0x00440738u;
   break;
  case 0x0044071c:
   if(!delay||(target!=0x00440720u&&target!=0x00440738u))
       return AE3_T2F_UNSUPPORTED;
   break;
  case 0x00440720:
   if(delay)return AE3_T2F_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=r(&n.cpu,16)?AE3_T2_FINISH_EINTR_CALL:0x00440770u;
   break;
  case 0x00440724:
   if(!delay||(target!=0x00440770u&&target!=AE3_T2_FINISH_EINTR_CALL))
      return AE3_T2F_UNSUPPORTED;
   w(&n.cpu,2,1u);break;
  case 0x00440738:
   if(delay)return AE3_T2F_UNSUPPORTED;
   w(&n.cpu,4,sx(0xffff0000u));break;
  case 0x0044073c:
   if(delay)return AE3_T2F_UNSUPPORTED;
   w(&n.cpu,4,sx((uint32_t)r(&n.cpu,4)|0xf3ffu));break;
  case 0x00440740:
   if(delay)return AE3_T2F_UNSUPPORTED;
   w(&n.cpu,4,r(&n.cpu,2)&r(&n.cpu,4));break;
  case 0x00440770:case 0x00440774:{
   if(delay)return AE3_T2F_UNSUPPORTED;
   uint32_t addr=(uint32_t)r(&n.cpu,29)+(pc==0x00440770?0x10u:0);
   if(addr&7u)return AE3_T2F_MEMORY_FAULT;
   uint8_t raw[8];
   if(ae3_ee_bus_read(bus,addr,raw,8)!=AE3_EE_BUS_OK)return AE3_T2F_MEMORY_FAULT;
   uint64_t value=0;
   for(unsigned k=0;k<8;k++)value|=(uint64_t)raw[k]<<(8u*k);
   w(&n.cpu,pc==0x00440770?31u:16u,value);
   break;
  }
  case 0x00440778:
   if(delay||r(&n.cpu,31)!=AE3_T2_FINISH_PARENT_NEXT)
      return AE3_T2F_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x0044077c:
   if(!delay||target!=AE3_T2_FINISH_PARENT_NEXT)
      return AE3_T2F_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)+0x20u));break;
  default:return AE3_T2F_UNSUPPORTED;
  }
  n.cpu.pc=target;n.cpu.retired++;*s=n;
  if(pending(s)!=AE3_T2F_YIELDED)return pending(s);
 }
 return AE3_T2F_YIELDED;
}
AE3_T2FinishResult ae3_boot_timer2_finish_resume_mode(AE3_BootTimer2Finish *s,
 const AE3_BootTimer2Setup *done){
 if(!s||!done||!s->entered||s->cpu.pc!=AE3_T2_FINISH_MODE_CALL||
    s->cpu.in_delay_slot||!done->active||done->cpu.in_delay_slot||
    done->cpu.pc!=AE3_T2_FINISH_NEXT_SERVICE||done->io_stores!=1u||
    (done->cop0_status&UINT32_C(0x10000)))return AE3_T2F_INVALID;
 s->cpu=done->cpu;s->cop0_status=done->cop0_status;
 return AE3_T2F_NEXT_SERVICE_PENDING;
}
