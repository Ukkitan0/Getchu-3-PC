#include "ae3_boot_tlb_finalize.h"
#include <stddef.h>
static uint64_t rr(const AE3_BootPrefix*s,unsigned r){return r?s->gpr[r][0]:0;}
static void ww(AE3_BootPrefix*s,unsigned r,uint64_t v){if(r)s->gpr[r][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* KSEG0 guest code was copied from 0x006A1840. See verifier TSV for
 * source-address reference words; runtime FETCHES the actual copied bytes. */
static const struct {uint32_t pc,word;} code[]={
 {0x80075000,0x3c028007},{0x80075004,0x0000282d},
 {0x80075008,0x24435300},{0x8007500c,0x00000000},
 {0x80075010,0x8c620000},{0x80075014,0x14820003},
 {0x80075018,0x24a50001},{0x8007501c,0x03e00008},
 {0x80075020,0x8c620004},{0x80075024,0x2ca20006},
 {0x80075028,0x1440fff9},{0x8007502c,0x24630008},
 {0x80075030,0x03e00008},{0x80075034,0x0000102d},
 {0x0043f458,0x03e00008},{0x0043f45c,0x00000000},
 {0x0043f460,0x24030074},{0x0043f464,0x0000000c},
 {0x0043f468,0x03e00008},{0x0043f46c,0x00000000},
 {0x0043f450,0x2403005b},{0x0043f454,0x0000000c},
 {0x0043f4e8,0x0c10fd14},{0x0043f4ec,0x26520001},
 {0x0043f4f0,0x8e240000},{0x0043f4f4,0x0040282d},
 {0x0043f4f8,0x0c10fd18},{0x0043f4fc,0x26310008},
 {0x0043f500,0x2e420008},{0x0043f504,0x5440fff8},
 {0x0043f508,0x8e240000},{0x0043f50c,0x0c10fd14},
 {0x0043f510,0x24040003},{0x0043f514,0x3c03006a},
 {0x0043f518,0xdfbf0030},{0x0043f51c,0xdfb20020},
 {0x0043f520,0xdfb10010},{0x0043f524,0xdfb00000},
 {0x0043f528,0xac621bb8},{0x0043f52c,0x03e00008},
 {0x0043f530,0x27bd0040}
};
static AE3_TlbFinishResult boundary(const AE3_BootTlbFinalize*s){
 if(s->done)return AE3_TLBF_RETURNED;
 if(s->cpu.in_delay_slot)return AE3_TLBF_YIELDED;
 if(s->cpu.pc==0x0043f454)return AE3_TLBF_GET_ENTRY;
 if(s->cpu.pc==0x0043f464)return AE3_TLBF_SET_SYSCALL;
 return AE3_TLBF_YIELDED;
}
AE3_TlbFinishResult ae3_boot_tlb_finish_enter(AE3_BootTlbFinalize*out,
 const AE3_BootTlbPatch*p){
 if(!out||!p||!p->entered||!p->copy_done||p->copy_active||
    p->registrations!=3||p->flushes!=2||p->copy_words!=204||
    p->cpu.pc!=AE3_TLB_GUEST_GET_ENTRY||p->cpu.in_delay_slot||
    (uint32_t)rr(&p->cpu,3)!=0x5b)return AE3_TLBF_INVALID;
 *out=(AE3_BootTlbFinalize){0};out->cpu=p->cpu;out->active=1;
 return AE3_TLBF_GET_ENTRY;
}
AE3_TlbFinishResult ae3_boot_tlb_finish_service(AE3_BootTlbFinalize*s,
 AE3_EEMemoryBus*bus,AE3_EESyscallKernel*kernel){
 if(!s||!s->active||!bus||!kernel||s->cpu.in_delay_slot)
    return AE3_TLBF_INVALID;
 AE3_TlbFinishResult evt=boundary(s);
 if(evt!=AE3_TLBF_GET_ENTRY&&evt!=AE3_TLBF_SET_SYSCALL)
    return AE3_TLBF_INVALID;
 uint32_t word=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&word)!=AE3_EE_BUS_OK)
    return AE3_TLBF_MEMORY_FAULT;
 if(word!=12u)return AE3_TLBF_UNSUPPORTED;
 AE3_BootTlbFinalize n=*s;
 if(evt==AE3_TLBF_GET_ENTRY){
  uint32_t target=0;
  if(s->entry_active||s->entries>=6||rr(&s->cpu,3)!=0x5b||
     (uint32_t)rr(&s->cpu,4)!=(s->entries<5?0x55u+s->entries:3u)||
     ae3_ee_syscall_lookup(kernel,0x5b,&target)!=0||target!=AE3_TLB_COPY_DST)
       return AE3_TLBF_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,target,&word)!=AE3_EE_BUS_OK)
      return AE3_TLBF_MEMORY_FAULT;
  if(word!=code[0].word)return AE3_TLBF_UNSUPPORTED;
  n.cpu.pc=target;n.entry_active=1;n.entries++;
 }else{
  uint32_t num=(uint32_t)rr(&s->cpu,4);
  uint32_t handler=(uint32_t)rr(&s->cpu,5);
  uint32_t original_num=0,original_handler=0;
  if(s->entry_active||s->sets>=5||s->sets>=s->entries||
     rr(&s->cpu,3)!=0x74||num!=0x55u+s->sets||
     ae3_ee_bus_read32(bus,0x80075300u+s->sets*8u,&original_num)!=AE3_EE_BUS_OK||
     ae3_ee_bus_read32(bus,0x80075304u+s->sets*8u,&original_handler)!=AE3_EE_BUS_OK)
      return AE3_TLBF_MEMORY_FAULT;
  if(num!=original_num||handler!=original_handler)return AE3_TLBF_UNSUPPORTED;
  uint32_t old=0;
  (void)ae3_ee_syscall_lookup(kernel,num,&old);
  if(ae3_ee_syscall_register_bus(kernel,num,handler,bus)!=0)
      return AE3_TLBF_MEMORY_FAULT;
  ww(&n.cpu,2,sx(old));n.cpu.pc+=4u;n.sets++;
 }
 n.cpu.retired++;*s=n;
 return AE3_TLBF_YIELDED;
}
AE3_TlbFinishResult ae3_boot_tlb_finish_slice(AE3_BootTlbFinalize*s,
 AE3_EEMemoryBus*bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)
  return AE3_TLBF_INVALID;
 AE3_TlbFinishResult evt=boundary(s);if(evt!=AE3_TLBF_YIELDED)return evt;
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,op=0;size_t j=0;
  for(;j<sizeof code/sizeof code[0];j++)if(code[j].pc==pc)break;
  if(j==sizeof code/sizeof code[0])return AE3_TLBF_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)return AE3_TLBF_MEMORY_FAULT;
  if(op!=code[j].word)return AE3_TLBF_UNSUPPORTED;
  AE3_BootTlbFinalize n=*s;const unsigned d=n.cpu.in_delay_slot;
  uint32_t dest=d?n.cpu.branch_target:pc+4u;
  if(d){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t address=0;unsigned reg=0,width=0,store=0;
  switch(pc){
  case 0x80075000:case 0x0043f514:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,pc==0x80075000?2:3,sx(pc==0x80075000?0x80070000u:0x006a0000u));break;
  case 0x80075004:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,5,0);break;
  case 0x80075008:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,3,sx((uint32_t)rr(&n.cpu,2)+0x5300u));break;
  case 0x8007500c:
   if(d)return AE3_TLBF_UNSUPPORTED;
   break;
  case 0x80075010:case 0x80075020:
   if(d!=(pc==0x80075020))return AE3_TLBF_UNSUPPORTED;
   address=(uint32_t)rr(&n.cpu,3)+(pc==0x80075020?4u:0u);
   reg=2;width=4;n.table_reads++;break;
  case 0x80075014:
   if(d)return AE3_TLBF_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=rr(&n.cpu,4)!=rr(&n.cpu,2)?0x80075024u:0x8007501cu;break;
  case 0x80075018:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,5,sx((uint32_t)rr(&n.cpu,5)+1u));break;
  case 0x8007501c:case 0x80075030:
   if(d||!n.entry_active||
     (uint32_t)rr(&n.cpu,31)!=(n.entries<6?0x0043f4f0u:0x0043f514u))
      return AE3_TLBF_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)rr(&n.cpu,31);break;
  case 0x80075024:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,2,rr(&n.cpu,5)<6u);break;
  case 0x80075028:
   if(d)return AE3_TLBF_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=rr(&n.cpu,2)?0x80075010u:0x80075030u;break;
  case 0x8007502c:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,3,sx((uint32_t)rr(&n.cpu,3)+8u));break;
  case 0x80075034:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,2,0);break;
  case 0x0043f4f0:case 0x0043f508:
   if(d!=(pc==0x0043f508))return AE3_TLBF_UNSUPPORTED;
   address=(uint32_t)rr(&n.cpu,17);reg=4;width=4;break;
  case 0x0043f4f4:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,5,rr(&n.cpu,2));break;
  case 0x0043f4f8:case 0x0043f4e8:case 0x0043f50c:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=pc==0x0043f4f8?0x0043f460u:0x0043f450u;break;
  case 0x0043f4fc:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,17,sx((uint32_t)rr(&n.cpu,17)+8u));break;
  case 0x0043f500:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,2,(uint32_t)rr(&n.cpu,18)<8u);break;
  case 0x0043f504:
   if(d)return AE3_TLBF_UNSUPPORTED;
   if(rr(&n.cpu,2)){n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0043f4e8u;}
   else dest=0x0043f50cu; /* BNEZL: untaken slot is annulled. */
   break;
  case 0x0043f4ec:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,18,sx((uint32_t)rr(&n.cpu,18)+1u));break;
  case 0x0043f510:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,4,3);break;
  case 0x0043f450:case 0x0043f460:
   if(d)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,3,pc==0x0043f450?0x5b:0x74);break;
  case 0x0043f468:
   if(d)return AE3_TLBF_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)rr(&n.cpu,31);break;
  case 0x0043f46c:
   if(!d)return AE3_TLBF_UNSUPPORTED;
   break;
  case 0x0043f518:case 0x0043f51c:case 0x0043f520:case 0x0043f524:
   if(d)return AE3_TLBF_UNSUPPORTED;
   address=(uint32_t)rr(&n.cpu,29)+(pc==0x0043f518?0x30u:
      pc==0x0043f51c?0x20u:pc==0x0043f520?0x10u:0u);
   reg=pc==0x0043f518?31:pc==0x0043f51c?18:
       pc==0x0043f520?17:16;width=8;break;
  case 0x0043f528:
   if(d)return AE3_TLBF_UNSUPPORTED;
   address=(uint32_t)rr(&n.cpu,3)+0x1bb8u;reg=2;width=4;store=1;break;
  case 0x0043f52c:
   if(d||n.entries!=6||n.sets!=5)return AE3_TLBF_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)rr(&n.cpu,31);break;
  case 0x0043f530:
   if(!d||dest!=AE3_TLB_FINAL_RETURN)return AE3_TLBF_UNSUPPORTED;
   ww(&n.cpu,29,sx((uint32_t)rr(&n.cpu,29)+0x40u));break;
  default:return AE3_TLBF_UNSUPPORTED;
  }
  if(width){
    if(address&(width-1u))return AE3_TLBF_MEMORY_FAULT;
    if(store){
      uint8_t raw[8]={0};uint64_t v=rr(&n.cpu,reg);
      for(unsigned q=0;q<width;q++)raw[q]=(uint8_t)(v>>(8*q));
      if(ae3_ee_bus_write(bus,address,raw,width)!=AE3_EE_BUS_OK)
       return AE3_TLBF_MEMORY_FAULT;
    }else{
      uint8_t raw[8]={0};uint64_t v=0;
      if(ae3_ee_bus_read(bus,address,raw,width)!=AE3_EE_BUS_OK)
       return AE3_TLBF_MEMORY_FAULT;
      for(unsigned q=0;q<width;q++)v|=(uint64_t)raw[q]<<(q*8);
      ww(&n.cpu,reg,width==4?sx((uint32_t)v):v);
    }
  }
  n.cpu.pc=dest;n.cpu.retired++;
  if(s->entry_active && n.cpu.pc==(n.entries<6?0x0043f4f0u:0x0043f514u))
     n.entry_active=0;
  if(n.cpu.pc==AE3_TLB_FINAL_RETURN)n.done=1;
  *s=n;
  evt=boundary(s);if(evt!=AE3_TLBF_YIELDED)return evt;
 }
 return AE3_TLBF_YIELDED;
}