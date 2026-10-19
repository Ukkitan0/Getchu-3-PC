#include "ae3_guest_get_entry.h"
static uint64_t r(const AE3_BootPrefix*s,unsigned i){return i?s->gpr[i][0]:0;}
static void wr(AE3_BootPrefix*s,unsigned i,uint64_t v){if(i)s->gpr[i][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
AE3_GetEntryResult ae3_guest_get_entry_init(AE3_GuestGetEntry*out,
 const AE3_BootPrefix*caller,uint32_t return_pc){
 if(!out||!caller||caller->pc!=UINT32_C(0x0044032C)||
    caller->in_delay_slot||r(caller,3)!=0x5b||
    return_pc!=UINT32_C(0x00440330))return AE3_GET_ENTRY_INVALID;
 *out=(AE3_GuestGetEntry){0};out->cpu=*caller;
 out->cpu.pc=AE3_GET_ENTRY_GUEST_PC;
 out->cpu.gpr[31][0]=return_pc;out->cpu.retired=0;
 out->return_pc=return_pc;return AE3_GET_ENTRY_YIELDED;
}
AE3_GetEntryResult ae3_guest_get_entry_slice(AE3_GuestGetEntry *s,
 const AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!bus->ram||s->cpu.in_delay_slot>1||s->done>1||
    s->return_pc!=0x440330 ||(!s->done&&
    (s->cpu.pc<AE3_GET_ENTRY_GUEST_PC||
     s->cpu.pc>=AE3_GET_ENTRY_GUEST_END||
     (s->cpu.pc&3))))return AE3_GET_ENTRY_INVALID;
 if(s->done)return AE3_GET_ENTRY_RETURNED;
 for(uint32_t n=0;n<budget;n++){
    const uint32_t pc=s->cpu.pc;
    uint32_t w=0;
    if(ae3_ee_bus_read32(bus,pc,&w)!=AE3_EE_BUS_OK)return AE3_GET_ENTRY_MEMORY_FAULT;
    AE3_BootPrefix next=s->cpu;
    const int delay=next.in_delay_slot;
    uint32_t advance=delay?next.branch_target:pc+4;
    if(delay){next.in_delay_slot=0;next.branch_target=0;}
    uint32_t reads=0;
    switch(pc){
    case 0x80076000:
       if(delay||w!=0x3c028007u)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,2,sx(0x80070000u));break;
    case 0x80076004:
       if(delay||w!=0x0000282du)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,5,0);break;
    case 0x80076008:
       if(delay||w!=0x24436710u)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,3,sx((uint32_t)r(&next,2)+0x6710u));break;
    case 0x8007600c:
       if(delay||w)return AE3_GET_ENTRY_UNSUPPORTED;break;
    case 0x80076010:case 0x80076020:{
       if(w!=(pc==0x80076010?0x8c620000u:0x8c620004u)||
          (pc==0x80076020&&!delay))return AE3_GET_ENTRY_UNSUPPORTED;
       uint32_t value=0;
       if(ae3_ee_bus_read32(bus,(uint32_t)r(&next,3)+
             (pc==0x80076020?4u:0u),&value)!=AE3_EE_BUS_OK)
          return AE3_GET_ENTRY_MEMORY_FAULT;
       wr(&next,2,sx(value));reads++;break;}
    case 0x80076014:
       if(delay||w!=0x14820003u)return AE3_GET_ENTRY_UNSUPPORTED;
       next.in_delay_slot=1;
       next.branch_target=(r(&next,4)!=r(&next,2))?0x80076024u:0x8007601cu;break;
    case 0x80076018:
       if(!delay||w!=0x24a50001u)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,5,sx((uint32_t)r(&next,5)+1u));break;
    case 0x8007601c:case 0x80076030:
       if(delay||w!=0x03e00008u||r(&next,31)!=s->return_pc)
          return AE3_GET_ENTRY_UNSUPPORTED;
       next.in_delay_slot=1;next.branch_target=s->return_pc;break;
    case 0x80076024:
       if(delay||w!=0x2ca20006u)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,2,r(&next,5)<6u);break;
    case 0x80076028:
       if(delay||w!=0x1440fff9u)return AE3_GET_ENTRY_UNSUPPORTED;
       next.in_delay_slot=1;
       next.branch_target=r(&next,2)!=0?0x80076010u:0x80076030u;break;
    case 0x8007602c:
       if(!delay||w!=0x24630008u)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,3,sx((uint32_t)r(&next,3)+8u));break;
    case 0x80076034:
       if(!delay||w!=0x0000102du)return AE3_GET_ENTRY_UNSUPPORTED;
       wr(&next,2,0);break;
    default:return AE3_GET_ENTRY_UNSUPPORTED;
    }
    if(advance!=s->return_pc&&
      (advance<AE3_GET_ENTRY_GUEST_PC||advance>=AE3_GET_ENTRY_GUEST_END||
      (advance&3)))return AE3_GET_ENTRY_UNSUPPORTED;
    next.pc=advance;next.retired++;
    s->cpu=next;s->table_reads+=reads;
    if(advance==s->return_pc){s->done=1;return AE3_GET_ENTRY_RETURNED;}
 }
 return AE3_GET_ENTRY_YIELDED;
}