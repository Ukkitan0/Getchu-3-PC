#include "ae3_boot_thread_continue.h"
#include <stddef.h>
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static const struct {uint32_t pc,op;} code[]={
 {0x004358E4,0x0000000C},{0x004358E8,0x03E00008},
 {0x004358EC,0x00000000},{0x00436940,0x0040202D},
 {0x00436944,0x04810005},{0x00436948,0xAE041788},
 {0x0043694C,0x0C10D6BC},{0x00436950,0x8E449B90},
 {0x00435AF0,0x24030041},{0x00435AF4,0x0000000C},
 {0x00435AF8,0x03E00008},{0x00435AFC,0x00000000},
 {0x00436954,0x1000000D},{0x00436958,0x2402FFFF},
 {0x0043695C,0x3C020077},{0x00436960,0x24439B98},
 {0x00436964,0xAC409B98},{0x00436968,0x0060282D},
 {0x0043696C,0x0C10DB08},{0x00436970,0xAC600004},
 {0x0043698C,0xDFBF0080},{0x00436990,0xDFB20070},
 {0x00436994,0xDFB10060},{0x00436998,0xDFB00050},
 {0x0043699C,0x03E00008},{0x004369A0,0x27BD0090}
};
static AE3_ThreadBootResult pending(const AE3_BootThreadContinue *s){
 if(s->cpu.in_delay_slot)return AE3_THR_YIELDED;
 switch(s->cpu.pc){
 case AE3_THR_CREATE_PC:return s->create_serviced?AE3_THR_INVALID:AE3_THR_CREATE_PENDING;
 case AE3_THR_DELETE_PC:return s->delete_serviced?AE3_THR_INVALID:AE3_THR_DELETE_PENDING;
 case AE3_THR_NEXT_CALLEE:return AE3_THR_NEXT_CALL;
 case AE3_THR_PARENT_NEXT:return AE3_THR_RETURNED;
 default:return AE3_THR_YIELDED;
 }
}
AE3_ThreadBootResult ae3_boot_thread_continue_enter(AE3_BootThreadContinue *s,
 const AE3_BootEventInit *prior){
 if(!s||!prior||!prior->entered||!prior->sema_serviced||
    prior->sema_result<=0||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_EVENT_THREAD_SYSCALL||
    r(&prior->cpu,3)!=0x20u)return AE3_THR_INVALID;
 *s=(AE3_BootThreadContinue){0};s->cpu=prior->cpu;s->active=1;
 return AE3_THR_CREATE_PENDING;
}
AE3_ThreadBootResult ae3_boot_thread_create(AE3_BootThreadContinue *s,
 AE3_EEMemoryBus *bus,AE3_EEThreadKernel *kernel){
 if(!s||!bus||!kernel||!s->active||s->create_serviced||
    s->cpu.in_delay_slot||s->cpu.pc!=AE3_THR_CREATE_PC||
    r(&s->cpu,3)!=0x20u)return AE3_THR_INVALID;
 uint32_t op=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&op)!=AE3_EE_BUS_OK)
    return AE3_THR_MEMORY_FAULT;
 if(op!=0x0000000cu)return AE3_THR_UNSUPPORTED;
 const uint32_t ptr=(uint32_t)r(&s->cpu,4);
 if(!ptr||(ptr&3u)||ptr>UINT32_MAX-32u)return AE3_THR_MEMORY_FAULT;
 uint32_t words[AE3_EE_THREAD_DESC_WORDS];
 for(unsigned i=0;i<AE3_EE_THREAD_DESC_WORDS;i++)
  if(ae3_ee_bus_read32(bus,ptr+i*4u,&words[i])!=AE3_EE_BUS_OK)
      return AE3_THR_MEMORY_FAULT;
 /* The descriptor is read from actual guest RAM. Invalid contents are
  * ordinary CreateThread rejection (negative v0), not fabricated success. */
 int32_t id=ae3_ee_thread_kernel_create(kernel,bus,ptr,words);
 for(unsigned i=0;i<AE3_EE_THREAD_DESC_WORDS;i++)s->descriptor[i]=words[i];
 s->descriptor_address=ptr;s->thread_id=id;s->create_serviced=1;
 w(&s->cpu,2,sx((uint32_t)id));s->cpu.pc+=4u;s->cpu.retired++;
 return AE3_THR_YIELDED;
}
AE3_ThreadBootResult ae3_boot_thread_delete_sema(AE3_BootThreadContinue *s,
 const AE3_EEMemoryBus *bus,AE3_EESemaKernel *kernel){
 if(!s||!bus||!kernel||!s->active||!s->create_serviced||
    s->thread_id>=0||s->delete_serviced||s->cpu.in_delay_slot||
    s->cpu.pc!=AE3_THR_DELETE_PC||r(&s->cpu,3)!=0x41u)
    return AE3_THR_INVALID;
 uint32_t op=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&op)!=AE3_EE_BUS_OK)
    return AE3_THR_MEMORY_FAULT;
 if(op!=0x0000000cu)return AE3_THR_UNSUPPORTED;
 int result=ae3_ee_sema_kernel_delete(kernel,(int32_t)r(&s->cpu,4));
 w(&s->cpu,2,sx((uint32_t)result));s->delete_serviced=1;
 s->cpu.pc+=4u;s->cpu.retired++;
 return AE3_THR_YIELDED;
}
AE3_ThreadBootResult ae3_boot_thread_continue_slice(AE3_BootThreadContinue *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_THR_INVALID;
 AE3_ThreadBootResult state=pending(s);
 if(state!=AE3_THR_YIELDED)return state;
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,op=0;size_t index=0;
  for(;index<sizeof(code)/sizeof(code[0]);index++)
   if(code[index].pc==pc)break;
  if(index==sizeof(code)/sizeof(code[0]))return AE3_THR_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)return AE3_THR_MEMORY_FAULT;
  if(op!=code[index].op)return AE3_THR_UNSUPPORTED;
  AE3_BootThreadContinue n=*s;
  const int delay=n.cpu.in_delay_slot;
  uint32_t target=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t address=0;unsigned reg=0,size=0;int store=0;
  switch(pc){
  case 0x004358E8:case 0x00435AF8:case 0x0043699C:
   if(delay||(pc==0x004358E8&&!n.create_serviced)||
      (pc==0x00435AF8&&!n.delete_serviced))return AE3_THR_UNSUPPORTED;
   if(pc==0x0043699C && r(&n.cpu,31)!=AE3_THR_PARENT_NEXT)
      return AE3_THR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x004358EC:case 0x00435AFC:
   if(!delay||(pc==0x004358EC&&target!=0x00436940u)||
      (pc==0x00435AFC&&target!=0x00436954u))return AE3_THR_UNSUPPORTED;
   break;
  case 0x00436940:
   if(delay)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,4,r(&n.cpu,2));break;
  case 0x00436944:
   if(delay)return AE3_THR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=(int64_t)r(&n.cpu,4)>=0?0x0043695cu:0x0043694cu;
   break;
  case 0x00436948:
   if(!delay||(target!=0x0043695cu&&target!=0x0043694cu))
      return AE3_THR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,16)+0x1788u;reg=4;size=4;store=1;break;
  case 0x0043694C:
   if(delay||n.thread_id>=0)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=0x00435af0u;break;
  case 0x00436950:
   if(!delay||target!=0x00435af0u)return AE3_THR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,16)+0x9b90u;reg=4;size=4;break;
  case 0x00435AF0:
   if(delay)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,3,0x41u);break;
  case 0x00436954:
   if(delay)return AE3_THR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=0x0043698cu;break;
  case 0x00436958:
   if(!delay||target!=0x0043698cu)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,2,sx(0xffffffffu));break;
  case 0x0043695C:
   if(delay||n.thread_id<=0)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,2,sx(0x00770000u));break;
  case 0x00436960:
   if(delay)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,3,sx((uint32_t)r(&n.cpu,2)+0x9b98u));break;
  case 0x00436964:
   if(delay)return AE3_THR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,2)+0x9b98u;reg=0;size=4;store=1;break;
  case 0x00436968:
   if(delay)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,5,r(&n.cpu,3));break;
  case 0x0043696C:
   if(delay)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=AE3_THR_NEXT_CALLEE;break;
  case 0x00436970:
   if(!delay||target!=AE3_THR_NEXT_CALLEE)return AE3_THR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,3)+4u;reg=0;size=4;store=1;break;
  case 0x0043698C:case 0x00436990:case 0x00436994:case 0x00436998:
   if(delay)return AE3_THR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,29)+(pc==0x0043698c?0x80u:
       pc==0x00436990?0x70u:pc==0x00436994?0x60u:0x50u);
   reg=pc==0x0043698c?31u:pc==0x00436990?18u:
       pc==0x00436994?17u:16u;size=8;break;
  case 0x004369A0:
   if(!delay||target!=AE3_THR_PARENT_NEXT)return AE3_THR_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)+0x90u));break;
  default:return AE3_THR_UNSUPPORTED;
  }
  if(size){
   if(address&(size-1u))return AE3_THR_MEMORY_FAULT;
   uint8_t bytes[8]={0};
   if(store){
    uint64_t value=r(&n.cpu,reg);
    for(unsigned j=0;j<size;j++)bytes[j]=(uint8_t)(value>>(j*8u));
    if(ae3_ee_bus_write(bus,address,bytes,size)!=AE3_EE_BUS_OK)
        return AE3_THR_MEMORY_FAULT;
   }else{
    if(ae3_ee_bus_read(bus,address,bytes,size)!=AE3_EE_BUS_OK)
        return AE3_THR_MEMORY_FAULT;
    uint64_t value=0;
    for(unsigned j=0;j<size;j++)value|=(uint64_t)bytes[j]<<(j*8u);
    w(&n.cpu,reg,size==4?sx((uint32_t)value):value);
   }
  }
  n.cpu.pc=target;n.cpu.retired++;*s=n;
  state=pending(s);if(state!=AE3_THR_YIELDED)return state;
 }
 return AE3_THR_YIELDED;
}