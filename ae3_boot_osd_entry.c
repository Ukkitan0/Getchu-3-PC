#include "ae3_boot_osd_entry.h"
#include <stddef.h>
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static const struct {uint32_t pc,op;} code[]={
 {0x0043FE90,0x0C10FFE0},{0x0043FE94,0x00000000},
 {0x0043FF80,0x27BDFFC0},{0x0043FF84,0xFFBF0030},
 {0x0043FF88,0xFFB20020},{0x0043FF8C,0xFFB10010},
 {0x0043FF90,0x0C10FFC6},{0x0043FF94,0xFFB00000},
 {0x0043FF18,0x27BDFFD0},{0x0043FF1C,0xFFB00010},
 {0x0043FF20,0xFFBF0020},{0x0043FF24,0x0C10D6E4},
 {0x0043FF28,0x03A0202D},{0x00435B90,0x2403004B}
};
AE3_OsdEntryResult ae3_boot_osd_enter(AE3_BootOsdEntry *out,
 const AE3_BootThreadPriority *prior){
 if(!out||!prior||!prior->active||!prior->getid_serviced||
    !prior->priority_serviced||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_TPR_PARENT_NEXT)return AE3_OSD_INVALID;
 *out=(AE3_BootOsdEntry){0};out->cpu=prior->cpu;out->active=1;
 return AE3_OSD_YIELDED;
}
AE3_OsdEntryResult ae3_boot_osd_slice(AE3_BootOsdEntry *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_OSD_INVALID;
 if(s->cpu.pc==AE3_OSD_SYSCALL_PC&&!s->cpu.in_delay_slot)
  return AE3_OSD_SYSCALL_PENDING;
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,op=0;size_t j=0;
  for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
  if(j==sizeof(code)/sizeof(code[0]))return AE3_OSD_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)
   return AE3_OSD_MEMORY_FAULT;
  if(op!=code[j].op)return AE3_OSD_UNSUPPORTED;
  AE3_BootOsdEntry n=*s;
  int delay=n.cpu.in_delay_slot;
  uint32_t dest=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t address=0;unsigned reg=0;int store=0;
  switch(pc){
  case 0x0043FE90:case 0x0043FF90:case 0x0043FF24:
   if(delay)return AE3_OSD_UNSUPPORTED;
   w(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=pc==0x0043fe90?0x0043ff80u:
     pc==0x0043ff90?0x0043ff18u:0x00435b90u;break;
  case 0x0043FE94:
   if(!delay||dest!=0x0043ff80u)return AE3_OSD_UNSUPPORTED;
   break;
  case 0x0043FF80:case 0x0043FF18:
   if(delay)return AE3_OSD_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)-
      (pc==0x0043ff80?0x40u:0x30u)));break;
  case 0x0043FF84:case 0x0043FF88:case 0x0043FF8C:
  case 0x0043FF94:case 0x0043FF1C:case 0x0043FF20:
   if(delay!=(pc==0x0043ff94))return AE3_OSD_UNSUPPORTED;
   if(pc==0x0043ff94&&dest!=0x0043ff18u)return AE3_OSD_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,29)+(pc==0x0043ff84?0x30u:
    pc==0x0043ff88?0x20u:pc==0x0043ff8c?0x10u:
    pc==0x0043ff94?0x00u:pc==0x0043ff1c?0x10u:0x20u);
   reg=pc==0x0043ff84||pc==0x0043ff20?31u:
     pc==0x0043ff88?18u:pc==0x0043ff8c?17u:16u;
   store=1;break;
  case 0x0043FF28:
   if(!delay||dest!=0x00435b90u)return AE3_OSD_UNSUPPORTED;
   w(&n.cpu,4,r(&n.cpu,29));break;
  case 0x00435B90:
   if(delay)return AE3_OSD_UNSUPPORTED;
   w(&n.cpu,3,0x4bu);break;
  default:return AE3_OSD_UNSUPPORTED;
  }
  if(store){
   if(address&7u)return AE3_OSD_MEMORY_FAULT;
   uint64_t val=r(&n.cpu,reg);uint8_t bytes[8];
   for(unsigned j=0;j<8;j++)bytes[j]=(uint8_t)(val>>(8*j));
   if(ae3_ee_bus_write(bus,address,bytes,8)!=AE3_EE_BUS_OK)
    return AE3_OSD_MEMORY_FAULT;
  }
  n.cpu.pc=dest;n.cpu.retired++;*s=n;
  if(dest==AE3_OSD_SYSCALL_PC&&!s->cpu.in_delay_slot)
   return AE3_OSD_SYSCALL_PENDING;
 }
 return AE3_OSD_YIELDED;
}