#include "ae3_boot_tlb_patch.h"
#include <stddef.h>
static uint64_t rg(const AE3_BootPrefix *s,unsigned r){return r?s->gpr[r][0]:0;}
static void wt(AE3_BootPrefix *s,unsigned r,uint64_t v){if(r)s->gpr[r][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* All entries verified byte-for-byte against the supplied post-boot EE RAM,
 * never dynamically patched or translated as an unbounded MIPS emulator. */
static const struct {uint32_t pc,word;} ops[]={
 {0x0043fe98,0x0c10fd1c},{0x0043fe9c,0x00000000},
 {0x0043f470,0x27bdffc0},{0x0043f474,0x3c02006a},
 {0x0043f478,0xffb20020},{0x0043f47c,0xffb10010},
 {0x0043f480,0x24120003},{0x0043f484,0xffb00000},
 {0x0043f488,0xffbf0030},{0x0043f48c,0x24501bc0},
 {0x0043f490,0x8c441bc0},{0x0043f494,0x26110018},
 {0x0043f498,0x0c10fd18},{0x0043f49c,0x8e050004},
 {0x0043f4a0,0x3c05006a},{0x0043f4a4,0x3c048007},
 {0x0043f4a8,0x24060330},{0x0043f4ac,0x24a51840},
 {0x0043f4b0,0x0c10fd02},{0x0043f4b4,0x34845000},
 {0x0043f4b8,0x0c10d758},{0x0043f4bc,0x0000202d},
 {0x0043f4c0,0x0c10d758},{0x0043f4c4,0x24040002},
 {0x0043f4c8,0x8e040008},{0x0043f4cc,0x0c10fd18},
 {0x0043f4d0,0x8e05000c},{0x0043f4d4,0x8e040010},
 {0x0043f4d8,0x0c10fd18},{0x0043f4dc,0x8e050014},
 {0x0043f4e0,0x8e240000},{0x0043f4e4,0x00000000},
 {0x0043f4e8,0x0c10fd14},{0x0043f4ec,0x26520001},
 {0x0043f408,0x2403005a},{0x0043f40c,0x0000000c},
 {0x0043f460,0x24030074},{0x0043f464,0x0000000c},
 {0x0043f468,0x03e00008},{0x0043f46c,0x00000000},
 {0x00435d60,0x24030064},{0x00435d64,0x0000000c},
 {0x00435d68,0x03e00008},{0x00435d6c,0x00000000},
 {0x0043f450,0x2403005b},{0x0043f454,0x0000000c},
 {0x0043f418,0x00063082},{0x0043f41c,0x10c0000a},
 {0x0043f420,0x0000382d},{0x0043f424,0x00000000},
 {0x0043f428,0x8ca30000},{0x0043f42c,0x24e70001},
 {0x0043f430,0x24a50004},{0x0043f434,0x00e6102b},
 {0x0043f438,0xac830000},{0x0043f43c,0x24840004},
 {0x0043f440,0x1440fff9},{0x0043f444,0x00000000},
 {0x0043f448,0x03e00008},{0x0043f44c,0x0000102d}
};
static AE3_TlbResult boundary(const AE3_BootTlbPatch *s){
 if(s->cpu.in_delay_slot)return AE3_TLB_YIELDED;
 switch(s->cpu.pc){
 case 0x0043f464:return AE3_TLB_SET_PENDING;
 case 0x0043f40c:return AE3_TLB_COPY_PENDING;
 case 0x00435d64:return AE3_TLB_FLUSH_PENDING;
 case AE3_TLB_GUEST_GET_ENTRY:
  return s->copy_done&&s->registrations==3&&s->flushes==2&&
    (uint32_t)rg(&s->cpu,3)==0x5b?AE3_TLB_GET_ENTRY_PENDING:AE3_TLB_INVALID;
 default:return AE3_TLB_YIELDED;
 }
}
AE3_TlbResult ae3_boot_tlb_enter(AE3_BootTlbPatch *out,
 const AE3_BootOsdRoundtrip *prior){
 if(!out||!prior||!prior->active||prior->read_count!=2||
   prior->write_count!=2||prior->cpu.in_delay_slot||
   prior->cpu.pc!=AE3_TLB_GUEST_ENTRY)return AE3_TLB_INVALID;
 *out=(AE3_BootTlbPatch){0};out->cpu=prior->cpu;out->entered=1;
 return AE3_TLB_YIELDED;
}
AE3_TlbResult ae3_boot_tlb_slice(AE3_BootTlbPatch *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->entered||s->cpu.in_delay_slot>1)return AE3_TLB_INVALID;
 AE3_TlbResult event=boundary(s);if(event!=AE3_TLB_YIELDED)return event;
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,word=0;
  size_t j=0;for(;j<sizeof ops/sizeof ops[0];j++)if(ops[j].pc==pc)break;
  if(j==sizeof ops/sizeof ops[0])return AE3_TLB_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_TLB_MEMORY_FAULT;
  if(word!=ops[j].word)return AE3_TLB_UNSUPPORTED;
  AE3_BootTlbPatch n=*s;
  unsigned d=n.cpu.in_delay_slot;
  uint32_t dest=d?n.cpu.branch_target:pc+4u;
  if(d){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t mem=0;unsigned width=0,reg=0,store=0;
  switch(pc){
  case 0x0043fe98:case 0x0043f498:case 0x0043f4b0:
  case 0x0043f4b8:case 0x0043f4c0:case 0x0043f4cc:
  case 0x0043f4d8:case 0x0043f4e8:
   if(d)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=pc==0x0043fe98?0x0043f470u:
     (pc==0x0043f498||pc==0x0043f4cc||pc==0x0043f4d8)?0x0043f460u:
     pc==0x0043f4b0?0x0043f408u:
     (pc==0x0043f4b8||pc==0x0043f4c0)?0x00435d60u:0x0043f450u;
   break;
  case 0x0043f468:case 0x00435d68:case 0x0043f448:
   if(d)return AE3_TLB_UNSUPPORTED;
   if(pc==0x0043f448 && (!n.copy_active||n.copy_words!=AE3_TLB_COPY_BYTES/4u))
     return AE3_TLB_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)rg(&n.cpu,31);break;
  case 0x0043fe9c:case 0x0043f46c:case 0x00435d6c:
  case 0x0043f424:case 0x0043f444:case 0x0043f4e4:
   if((pc==0x0043fe9c||pc==0x0043f46c||pc==0x00435d6c||pc==0x0043f444)!=d)
      return AE3_TLB_UNSUPPORTED;
   break;
  case 0x0043f470:
   if(d)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,29,sx((uint32_t)rg(&n.cpu,29)-0x40u));break;
  case 0x0043f474:case 0x0043f4a0:case 0x0043f4a4:
   if(d)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,pc==0x0043f474?2:pc==0x0043f4a0?5:4,
      sx(pc==0x0043f4a4?0x80070000u:0x006a0000u));break;
  case 0x0043f480:case 0x0043f4a8:case 0x0043f460:
  case 0x0043f408:case 0x00435d60:case 0x0043f450:
   if(d)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,pc==0x0043f480?18:pc==0x0043f4a8?6:3,
     pc==0x0043f480?3:pc==0x0043f4a8?0x330:
     pc==0x0043f460?0x74:pc==0x0043f408?0x5a:
     pc==0x00435d60?0x64:0x5b);break;
  case 0x0043f478:case 0x0043f47c:case 0x0043f484:
  case 0x0043f488:
   if(d)return AE3_TLB_UNSUPPORTED;
   width=8;store=1;
   mem=(uint32_t)rg(&n.cpu,29)+(pc==0x0043f478?0x20u:
      pc==0x0043f47c?0x10u:pc==0x0043f484?0u:0x30u);
   reg=pc==0x0043f478?18:pc==0x0043f47c?17:pc==0x0043f484?16:31;
   break;
  case 0x0043f48c:case 0x0043f494:case 0x0043f4ac:
  case 0x0043f4ec:case 0x0043f42c:case 0x0043f430:
  case 0x0043f43c:
   if(d!=(pc==0x0043f4ec))return AE3_TLB_UNSUPPORTED;
   reg=pc==0x0043f48c?16:pc==0x0043f494?17:
       pc==0x0043f4ac||pc==0x0043f430?5:
       pc==0x0043f4ec?18:pc==0x0043f42c?7:4;
   wt(&n.cpu,reg,sx((uint32_t)rg(&n.cpu,
      pc==0x0043f48c?2:pc==0x0043f494?16:reg)+(pc==0x0043f48c?0x1bc0u:
      pc==0x0043f494?0x18u:pc==0x0043f4ac?0x1840u:
      pc==0x0043f4ec||pc==0x0043f42c?1u:4u)));
   break;
  case 0x0043f490:case 0x0043f49c:case 0x0043f4c8:
  case 0x0043f4d0:case 0x0043f4d4:case 0x0043f4dc:
  case 0x0043f4e0:case 0x0043f428:
   if(d!=(pc==0x0043f49c||pc==0x0043f4d0||pc==0x0043f4dc))
      return AE3_TLB_UNSUPPORTED;
   mem=(uint32_t)rg(&n.cpu,pc==0x0043f490?2:
      pc==0x0043f4e0?17:pc==0x0043f428?5:16)+
     (pc==0x0043f490?0x1bc0u:pc==0x0043f49c?4u:pc==0x0043f4c8?8u:
      pc==0x0043f4d0?12u:pc==0x0043f4d4?16u:
      pc==0x0043f4dc?20u:0u);
   reg=pc==0x0043f49c||pc==0x0043f4d0||pc==0x0043f4dc?5:
      pc==0x0043f428?3:4;
   width=4;break;
  case 0x0043f4b4:
   if(!d||dest!=0x0043f408u)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,4,sx((uint32_t)rg(&n.cpu,4)|0x5000u));break;
  case 0x0043f4bc:
   if(!d||dest!=0x00435d60u)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,4,0);break;
  case 0x0043f4c4:
   if(!d||dest!=0x00435d60u)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,4,2);break;
  case 0x0043f418:
   if(d||!n.copy_active)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,6,(uint32_t)rg(&n.cpu,6)>>2);break;
  case 0x0043f41c:
   if(d||!n.copy_active)return AE3_TLB_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=rg(&n.cpu,6)?0x0043f424u:0x0043f448u;break;
  case 0x0043f420:
   if(!d||!n.copy_active)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,7,0);break;
  case 0x0043f434:
   if(d||!n.copy_active)return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,2,rg(&n.cpu,7)<rg(&n.cpu,6));break;
  case 0x0043f438:
   if(d||!n.copy_active)return AE3_TLB_UNSUPPORTED;
   mem=(uint32_t)rg(&n.cpu,4);width=4;reg=3;store=1;break;
  case 0x0043f440:
   if(d||!n.copy_active)return AE3_TLB_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=rg(&n.cpu,2)?0x0043f428u:0x0043f448u;break;
  case 0x0043f44c:
   if(!d||!n.copy_active||dest!=0x0043f4b8u)
      return AE3_TLB_UNSUPPORTED;
   wt(&n.cpu,2,0);n.copy_active=0;n.copy_done=1;break;
  default:return AE3_TLB_UNSUPPORTED;
  }
  if(width){
   if(mem&(width-1u))return AE3_TLB_MEMORY_FAULT;
   if(store){
    uint8_t bytes[8]={0};uint64_t val=rg(&n.cpu,reg);
    for(unsigned q=0;q<width;q++)bytes[q]=(uint8_t)(val>>(8u*q));
    if(ae3_ee_bus_write(bus,mem,bytes,width)!=AE3_EE_BUS_OK)
       return AE3_TLB_MEMORY_FAULT;
    if(pc==0x0043f438)n.copy_words++;
   }else{
    uint8_t bytes[8]={0};uint64_t val=0;
    if(ae3_ee_bus_read(bus,mem,bytes,width)!=AE3_EE_BUS_OK)
       return AE3_TLB_MEMORY_FAULT;
    for(unsigned q=0;q<width;q++)val|=(uint64_t)bytes[q]<<(8u*q);
    wt(&n.cpu,reg,width==4?sx((uint32_t)val):val);
   }
  }
  n.cpu.pc=dest;n.cpu.retired++;*s=n;
  event=boundary(s);if(event!=AE3_TLB_YIELDED)return event;
 }
 return AE3_TLB_YIELDED;
}
AE3_TlbResult ae3_boot_tlb_service(AE3_BootTlbPatch *s,
 AE3_EEMemoryBus *bus,AE3_EESyscallKernel *kernel){
 if(!s||!bus||!kernel||!s->entered)return AE3_TLB_INVALID;
 AE3_TlbResult evt=boundary(s);uint32_t pc=s->cpu.pc;
 if(evt!=AE3_TLB_SET_PENDING && evt!=AE3_TLB_COPY_PENDING &&
    evt!=AE3_TLB_FLUSH_PENDING)return AE3_TLB_INVALID;
 uint32_t word=0;
 if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_TLB_MEMORY_FAULT;
 if(word!=12u)return AE3_TLB_UNSUPPORTED;
 AE3_BootTlbPatch next=*s;
 if(evt==AE3_TLB_SET_PENDING){
  const uint32_t num=(uint32_t)rg(&s->cpu,4),address=(uint32_t)rg(&s->cpu,5);
  const uint32_t expected_num=s->registrations==0?0x5a:
                            s->registrations==1?0x5b:0x54;
  const uint32_t expected_addr=s->registrations==0?0x0043f418u:
                             s->registrations==1?AE3_TLB_COPY_DST:0x0043f980u;
  if(s->registrations>=3||rg(&s->cpu,3)!=0x74||num!=expected_num||
    address!=expected_addr|| (s->registrations==1 && (!s->copy_done||s->flushes!=2))||
    (s->registrations==2 && s->flushes!=2))return AE3_TLB_UNSUPPORTED;
  uint32_t old=0;
  (void)ae3_ee_syscall_lookup(kernel,num,&old);
  if(ae3_ee_syscall_register_bus(kernel,num,address,bus))return AE3_TLB_MEMORY_FAULT;
  wt(&next.cpu,2,sx(old));next.registrations++;next.cpu.pc=pc+4u;
 }else if(evt==AE3_TLB_COPY_PENDING){
  uint32_t target=0;
  if(s->registrations!=1||s->copy_active||s->copy_done||rg(&s->cpu,3)!=0x5a||
     (uint32_t)rg(&s->cpu,4)!=AE3_TLB_COPY_DST||
     (uint32_t)rg(&s->cpu,5)!=AE3_TLB_COPY_SRC||
     (uint32_t)rg(&s->cpu,6)!=AE3_TLB_COPY_BYTES||
     ae3_ee_syscall_lookup(kernel,0x5a,&target)||target!=0x0043f418u)
       return AE3_TLB_UNSUPPORTED;
  uint8_t scratch[AE3_TLB_COPY_BYTES];
  if(ae3_ee_bus_read(bus,AE3_TLB_COPY_SRC,scratch,sizeof scratch)!=AE3_EE_BUS_OK)
      return AE3_TLB_MEMORY_FAULT;
  if(ae3_ee_bus_read32(bus,AE3_TLB_COPY_DST,&word)!=AE3_EE_BUS_OK)
      return AE3_TLB_MEMORY_FAULT;
  next.copy_active=1;next.cpu.pc=0x0043f418u;
 }else{
  if(rg(&s->cpu,3)!=0x64||s->flushes>=2||!s->copy_done||
      (uint32_t)rg(&s->cpu,4)!=(uint32_t)(s->flushes*2u))
      return AE3_TLB_UNSUPPORTED;
  next.flush_modes[s->flushes]=(uint32_t)rg(&s->cpu,4);
  next.flushes++;next.cpu.pc=pc+4u;
 }
 next.cpu.retired++;*s=next;return AE3_TLB_YIELDED;
}