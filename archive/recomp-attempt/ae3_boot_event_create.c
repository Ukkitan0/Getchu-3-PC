#include "ae3_boot_event_create.h"
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* Byte-exact original US retail guest instructions. */
static const struct {uint32_t pc,word;} code[]={
 {0x0043fe88,0x0c10da2e},{0x0043fe8c,0x00000000},
 {0x004368b8,0x27bdff70},{0x004368bc,0xffb00050},
 {0x004368c0,0x3c10006a},{0x004368c4,0xffbf0080},
 {0x004368c8,0x8e021788},{0x004368cc,0xffb20070},
 {0x004368d0,0x1c400020},{0x004368d4,0xffb10060},
 {0x004368d8,0x3c020073},{0x004368dc,0x240300ff},
 {0x004368e0,0x2451b330},{0x004368e4,0xafa30034},
 {0x004368e8,0xafa00038},{0x004368ec,0x27a40030},
 {0x004368f0,0xafb10044},{0x004368f4,0x0c10d6b8},
 {0x004368f8,0x3c120077},
 {0x00435ae4,0x0000000c},{0x00435ae8,0x03e00008},{0x00435aec,0x00000000},
 {0x004368fc,0x04400015},{0x00436900,0xae429b90},
 {0x00436904,0x3c020043},{0x00436908,0x3c030077},
 {0x0043690c,0x3c050073},{0x00436910,0x244267e0},
 {0x00436914,0x24639790},{0x00436918,0x24a54070},
 {0x0043691c,0x24060400},{0x00436920,0xafa20004},
 {0x00436924,0xafa30008},{0x00436928,0x03a0202d},
 {0x0043692c,0xafa6000c},{0x00436930,0xafa50010},
 {0x00436934,0xafb10020},{0x00436938,0x0c10d638},
 {0x0043693c,0xafa00014},{0x004358e0,0x24030020},{0x00435ae0,0x24030040},
 {0x00436954,0x1000000d},{0x00436958,0x2402ffff},
 {0x0043698c,0xdfbf0080},{0x00436990,0xdfb20070},
 {0x00436994,0xdfb10060},{0x00436998,0xdfb00050},
 {0x0043699c,0x03e00008},{0x004369a0,0x27bd0090}
};
static AE3_EventInitResult event(const AE3_BootEventInit *s){
 if(s->cpu.in_delay_slot)return AE3_EVT_YIELDED;
 if(s->cpu.pc==AE3_EVENT_GUEST_SYSCALL)return AE3_EVT_SYSCALL_PENDING;
 if(s->cpu.pc==AE3_EVENT_THREAD_SYSCALL)return AE3_EVT_THREAD_SYSCALL_PENDING;
 if(s->cpu.pc==AE3_EVENT_PARENT_NEXT)return AE3_EVT_RETURNED;
 return AE3_EVT_YIELDED;
}
AE3_EventInitResult ae3_boot_event_enter(AE3_BootEventInit *s,
 const AE3_BootTimer2Finish *prior){
 if(!s||!prior||!prior->entered||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_T2_FINISH_PARENT_NEXT)return AE3_EVT_INVALID;
 *s=(AE3_BootEventInit){0};s->cpu=prior->cpu;s->entered=1;
 return AE3_EVT_YIELDED;
}
AE3_EventInitResult ae3_boot_event_slice(AE3_BootEventInit *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->entered||s->cpu.in_delay_slot>1)return AE3_EVT_INVALID;
 if(event(s)!=AE3_EVT_YIELDED)return event(s);
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,word=0;size_t j=0;
  for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
  if(j==sizeof(code)/sizeof(code[0]))return AE3_EVT_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_EVT_MEMORY_FAULT;
  if(word!=code[j].word)return AE3_EVT_UNSUPPORTED;
  AE3_BootEventInit n=*s;
  int delay=n.cpu.in_delay_slot;
  uint32_t next=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t access=0;unsigned areg=0,asize=0;int write=0;
  switch(pc){
  case 0x0043fe88:case 0x004368f4:case 0x00436938:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=pc==0x0043fe88?0x004368b8u:
                         pc==0x004368f4?0x00435ae0u:0x004358e0u;
   break;
  case 0x0043fe8c:
   if(!delay||next!=0x004368b8u)return AE3_EVT_UNSUPPORTED;
   break;
  case 0x004368b8:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)-0x90u));break;
  case 0x004368bc:case 0x004368c4:case 0x004368cc:
  case 0x004368d4:
   if(pc!=0x004368d4 && delay)return AE3_EVT_UNSUPPORTED;
   if(pc==0x004368d4&&(!delay||(next!=0x004368d8u&&next!=0x00436954u)))
      return AE3_EVT_UNSUPPORTED;
   access=(uint32_t)r(&n.cpu,29)+(pc==0x004368bc?0x50u:
          pc==0x004368c4?0x80u:pc==0x004368cc?0x70u:0x60u);
   areg=pc==0x004368bc?16u:pc==0x004368c4?31u:
        pc==0x004368cc?18u:17u;asize=8;write=1;break;
  case 0x004368c0:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,16,sx(0x006a0000u));break;
  case 0x004368c8:
   if(delay||r(&n.cpu,16)!=0x006a0000u)return AE3_EVT_UNSUPPORTED;
   access=(uint32_t)r(&n.cpu,16)+0x1788u;
   areg=2;asize=4;break;
  case 0x004368d0:
   if(delay||!n.global_read)return AE3_EVT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=(int32_t)(uint32_t)r(&n.cpu,2)>0?
       0x00436954u:0x004368d8u;
   break;
  case 0x004368d8:case 0x004368dc:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,pc==0x004368d8?2u:3u,
     pc==0x004368d8?sx(0x00730000u):0xffu);break;
  case 0x004368e0:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,17,sx((uint32_t)r(&n.cpu,2)-0x4cd0u));break;
  case 0x004368e4:case 0x004368e8:case 0x004368f0:
   if(delay)return AE3_EVT_UNSUPPORTED;
   access=(uint32_t)r(&n.cpu,29)+(pc==0x004368e4?0x34u:
         pc==0x004368e8?0x38u:0x44u);
   areg=pc==0x004368e4?3u:pc==0x004368e8?0u:17u;
   asize=4;write=1;break;
  case 0x004368ec:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,4,sx((uint32_t)r(&n.cpu,29)+0x30u));break;
  case 0x004368f8:
   if(!delay||next!=0x00435ae0u)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,18,sx(0x00770000u));break;
  case 0x00435ae0:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,3,0x40u);break;
  case 0x00435ae8:
   if(delay||!n.sema_serviced||r(&n.cpu,31)!=0x004368fcu)
      return AE3_EVT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x00435aec:
   if(!delay||next!=0x004368fcu)return AE3_EVT_UNSUPPORTED;
   break;
  case 0x004368fc:
   if(delay||!n.sema_serviced)return AE3_EVT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=(int32_t)(uint32_t)r(&n.cpu,2)<0?
       0x00436954u:0x00436904u;break;
  case 0x00436900:
   if(!delay||(next!=0x00436904u&&next!=0x00436954u))
      return AE3_EVT_UNSUPPORTED;
   access=(uint32_t)r(&n.cpu,16)+0x9b90u;
   areg=2;asize=4;write=1;break;
  case 0x00436904:case 0x00436908:case 0x0043690c:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,pc==0x00436904?2u:pc==0x00436908?3u:5u,
      sx(pc==0x00436904?0x00430000u:pc==0x00436908?0x00770000u:0x00730000u));break;
  case 0x00436910:case 0x00436914:case 0x00436918:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,pc==0x00436910?2u:pc==0x00436914?3u:5u,
      sx((uint32_t)r(&n.cpu,pc==0x00436910?2u:pc==0x00436914?3u:5u)+
        (pc==0x00436910?0x67e0u:pc==0x00436914?0x9790u:0x4070u)));break;
  case 0x0043691c:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,6,0x400u);break;
  case 0x00436920:case 0x00436924:case 0x0043692c:case 0x00436930:
  case 0x00436934:case 0x0043693c:
   if(delay!=(pc==0x0043693c))return AE3_EVT_UNSUPPORTED;
   access=(uint32_t)r(&n.cpu,29)+(pc==0x00436920?4u:pc==0x00436924?8u:
          pc==0x0043692c?12u:pc==0x00436930?16u:
          pc==0x00436934?32u:20u);
   areg=pc==0x00436920?2u:pc==0x00436924?3u:
        pc==0x0043692c?6u:pc==0x00436930?5u:
        pc==0x00436934?17u:0u;
   asize=4;write=1;break;
  case 0x00436928:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,4,r(&n.cpu,29));break;
  case 0x004358e0:
   if(delay)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,3,0x20u);break;
  case 0x00436954:
   if(delay)return AE3_EVT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0043698cu;break;
  case 0x00436958:
   if(!delay||next!=0x0043698cu)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,2,sx(0xffffffffu));break;
  case 0x0043698c:case 0x00436990:case 0x00436994:case 0x00436998:
   if(delay)return AE3_EVT_UNSUPPORTED;
   access=(uint32_t)r(&n.cpu,29)+(pc==0x0043698c?0x80u:
          pc==0x00436990?0x70u:pc==0x00436994?0x60u:0x50u);
   areg=pc==0x0043698c?31u:pc==0x00436990?18u:
        pc==0x00436994?17u:16u;asize=8;break;
  case 0x0043699c:
   if(delay||r(&n.cpu,31)!=AE3_EVENT_PARENT_NEXT)return AE3_EVT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x004369a0:
   if(!delay||next!=AE3_EVENT_PARENT_NEXT)return AE3_EVT_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)+0x90u));break;
  default:return AE3_EVT_UNSUPPORTED;
  }
  if(asize){
   uint8_t bytes[8]={0};
   if((access&(asize-1u))!=0)return AE3_EVT_MEMORY_FAULT;
   if(write){
    const uint64_t v=r(&n.cpu,areg);
    for(unsigned k=0;k<asize;k++)bytes[k]=(uint8_t)(v>>(8u*k));
    if(ae3_ee_bus_write(bus,access,bytes,asize)!=AE3_EE_BUS_OK)
       return AE3_EVT_MEMORY_FAULT;
   }else{
    if(ae3_ee_bus_read(bus,access,bytes,asize)!=AE3_EE_BUS_OK)
        return AE3_EVT_MEMORY_FAULT;
    uint64_t v=0;
    for(unsigned k=0;k<asize;k++)v|=(uint64_t)bytes[k]<<(8u*k);
    w(&n.cpu,areg,asize==4?sx((uint32_t)v):v);
    if(pc==0x004368c8){n.global_read=1;n.observed_global=(uint32_t)v;}
   }
  }
  n.cpu.pc=next;n.cpu.retired++;*s=n;
  if(event(s)!=AE3_EVT_YIELDED)return event(s);
 }
 return AE3_EVT_YIELDED;
}

AE3_EventInitResult ae3_boot_event_service_sema(AE3_BootEventInit *s,
 AE3_EEMemoryBus *bus,AE3_EESemaKernel *kernel){
 if(!s||!bus||!kernel||!s->entered||s->sema_serviced||
    s->cpu.in_delay_slot||s->cpu.pc!=AE3_EVENT_GUEST_SYSCALL||
    r(&s->cpu,3)!=0x40u)return AE3_EVT_INVALID;
 uint32_t opcode=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&opcode)!=AE3_EE_BUS_OK)
    return AE3_EVT_MEMORY_FAULT;
 if(opcode!=0x0000000cu)return AE3_EVT_UNSUPPORTED;
 const uint32_t descriptor=(uint32_t)r(&s->cpu,4);
 if((descriptor&3u)||descriptor>UINT32_MAX-20u)return AE3_EVT_MEMORY_FAULT;
 AE3_GuestSemaRequest req={0};
 req.pc=s->cpu.pc;req.guest_address=descriptor;req.syscall_number=0x40u;
 for(unsigned i=0;i<6;i++){
    if(ae3_ee_bus_read32(bus,descriptor+4u*i,&req.values[i])!=AE3_EE_BUS_OK)
        return AE3_EVT_MEMORY_FAULT;
 }
 /* Preserve existing compatibility-kernel object semantics; do not infer
  * Sony BIOS handler IDs, scheduling, or queue operation. */
 const int32_t id=ae3_ee_sema_kernel_create(kernel,&req);
 w(&s->cpu,2,sx((uint32_t)id));
 s->sema_result=id;s->sema_serviced=1;
 s->cpu.pc+=4u;s->cpu.retired++;
 return AE3_EVT_YIELDED;
}
