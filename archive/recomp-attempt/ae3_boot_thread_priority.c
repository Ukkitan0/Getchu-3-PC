#include "ae3_boot_thread_priority.h"
#include <stddef.h>
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* Exact user-provided US retail MIPS words (crosschecked against EE RAM).
 * Never silently execute instructions outside this bounded table. */
static const struct {uint32_t pc,word;} code[]={
 {0x004359D4,0x0000000C},{0x004359D8,0x03E00008},
 {0x004359DC,0x00000000},{0x0043697C,0x0040202D},
 {0x00436980,0x0C10D65C},{0x00436984,0x24050001},
 {0x00435970,0x24030029},{0x00435974,0x0000000C},
 {0x00435978,0x03E00008},{0x0043597C,0x00000000},
 {0x00436988,0x8E021788},
 {0x0043698C,0xDFBF0080},{0x00436990,0xDFB20070},
 {0x00436994,0xDFB10060},{0x00436998,0xDFB00050},
 {0x0043699C,0x03E00008},{0x004369A0,0x27BD0090}
};
static AE3_ThreadPriorityResult boundary(const AE3_BootThreadPriority *s){
 if(s->cpu.in_delay_slot)return AE3_TPR_YIELDED;
 if(s->cpu.pc==AE3_TPR_GETID_SYSCALL)
  return s->getid_serviced?AE3_TPR_INVALID:AE3_TPR_GETID_PENDING;
 if(s->cpu.pc==AE3_TPR_PRIORITY_SYSCALL)
  return s->priority_serviced?AE3_TPR_INVALID:AE3_TPR_PRIORITY_PENDING;
 if(s->cpu.pc==AE3_TPR_PARENT_NEXT)return AE3_TPR_PARENT_RETURNED;
 return AE3_TPR_YIELDED;
}
static AE3_ThreadPriorityResult verify_syscall(
 const AE3_BootThreadPriority *s,const AE3_EEMemoryBus *bus,
 uint32_t expected_pc,uint32_t expected_number){
 uint32_t opcode=0;
 if(!s||!bus||!s->active||s->cpu.in_delay_slot||
    s->cpu.pc!=expected_pc||r(&s->cpu,3)!=expected_number)
  return AE3_TPR_INVALID;
 if(ae3_ee_bus_read32(bus,expected_pc,&opcode)!=AE3_EE_BUS_OK)
  return AE3_TPR_MEMORY_FAULT;
 return opcode==0x0000000cu?AE3_TPR_YIELDED:AE3_TPR_UNSUPPORTED;
}
AE3_ThreadPriorityResult ae3_boot_thread_priority_enter(
 AE3_BootThreadPriority *out,const AE3_BootThreadNext *prior){
 if(!out||!prior||!prior->active||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_TPR_GETID_SYSCALL||r(&prior->cpu,3)!=0x2fu)
  return AE3_TPR_INVALID;
 *out=(AE3_BootThreadPriority){0};out->cpu=prior->cpu;out->active=1;
 return AE3_TPR_GETID_PENDING;
}
AE3_ThreadPriorityResult ae3_boot_thread_priority_getid(
 AE3_BootThreadPriority *s,const AE3_EEMemoryBus *bus,
 const AE3_EEThreadKernel *kernel){
 AE3_ThreadPriorityResult v=verify_syscall(s,bus,AE3_TPR_GETID_SYSCALL,0x2fu);
 if(v!=AE3_TPR_YIELDED)return v;
 if(s->getid_serviced||!kernel)return AE3_TPR_INVALID;
 int32_t id=ae3_ee_thread_kernel_get_current(kernel);
 if(id<=0)return AE3_TPR_CONTEXT_MISSING;
 w(&s->cpu,2,sx((uint32_t)id));s->returned_id=id;
 s->getid_serviced=1;s->cpu.pc+=4;s->cpu.retired++;
 return AE3_TPR_YIELDED;
}
AE3_ThreadPriorityResult ae3_boot_thread_priority_change(
 AE3_BootThreadPriority *s,const AE3_EEMemoryBus *bus,
 AE3_EEThreadKernel *kernel){
 AE3_ThreadPriorityResult v=verify_syscall(s,bus,AE3_TPR_PRIORITY_SYSCALL,0x29u);
 if(v!=AE3_TPR_YIELDED)return v;
 if(!kernel||!s->getid_serviced||s->priority_serviced)return AE3_TPR_INVALID;
 /* Both the guest arguments and the current-thread policy are explicit.
  * Invalid IDs yield the negative guest result; host state never changes. */
 int32_t result=ae3_ee_thread_kernel_change_current_priority(kernel,
                (uint32_t)r(&s->cpu,4),(uint32_t)r(&s->cpu,5));
 w(&s->cpu,2,sx((uint32_t)result));s->priority_result=result;
 s->priority_serviced=1;s->cpu.pc+=4;s->cpu.retired++;
 return AE3_TPR_YIELDED;
}
AE3_ThreadPriorityResult ae3_boot_thread_priority_slice(
 AE3_BootThreadPriority *s,const AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_TPR_INVALID;
 AE3_ThreadPriorityResult state=boundary(s);
 if(state!=AE3_TPR_YIELDED)return state;
 for(uint32_t i=0;i<budget;i++){
  const uint32_t pc=s->cpu.pc;uint32_t op=0;size_t j=0;
  for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
  if(j==sizeof(code)/sizeof(code[0]))return AE3_TPR_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)
   return AE3_TPR_MEMORY_FAULT;
  if(op!=code[j].word)return AE3_TPR_UNSUPPORTED;
  AE3_BootThreadPriority n=*s;
  const int delay=n.cpu.in_delay_slot;
  uint32_t dest=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t address=0;unsigned reg=0,size=0;
  switch(pc){
  case 0x004359D8:case 0x00435978:
   if(delay||(pc==0x004359d8&&!n.getid_serviced)||
      (pc==0x00435978&&!n.priority_serviced))return AE3_TPR_UNSUPPORTED;
   if(r(&n.cpu,31)!=(pc==0x004359d8?0x0043697cu:0x00436988u))
    return AE3_TPR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x004359DC:case 0x0043597C:
   if(!delay||dest!=(pc==0x004359dc?0x0043697cu:0x00436988u))
    return AE3_TPR_UNSUPPORTED;
   break;
  case 0x0043697C:
   if(delay)return AE3_TPR_UNSUPPORTED;
   w(&n.cpu,4,r(&n.cpu,2));break;
  case 0x00436980:
   if(delay)return AE3_TPR_UNSUPPORTED;
   w(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=0x00435970u;break;
  case 0x00436984:
   if(!delay||dest!=0x00435970u)return AE3_TPR_UNSUPPORTED;
   w(&n.cpu,5,1);break;
  case 0x00435970:
   if(delay)return AE3_TPR_UNSUPPORTED;
   w(&n.cpu,3,0x29u);break;
  case 0x00436988:
   if(delay)return AE3_TPR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,16)+0x1788u;
   if(address<(uint32_t)r(&n.cpu,16))return AE3_TPR_MEMORY_FAULT;
   reg=2;size=4;break;
  case 0x0043698C:case 0x00436990:case 0x00436994:case 0x00436998:
   if(delay)return AE3_TPR_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,29)+(pc==0x0043698c?0x80u:
          pc==0x00436990?0x70u:pc==0x00436994?0x60u:0x50u);
   reg=pc==0x0043698c?31u:pc==0x00436990?18u:
          pc==0x00436994?17u:16u;size=8;break;
  case 0x0043699C:
   if(delay||r(&n.cpu,31)!=AE3_TPR_PARENT_NEXT)
    return AE3_TPR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x004369A0:
   if(!delay||dest!=AE3_TPR_PARENT_NEXT)return AE3_TPR_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)+0x90u));break;
  default:return AE3_TPR_UNSUPPORTED;
  }
  if(size){
   if(address&(size-1u))return AE3_TPR_MEMORY_FAULT;
   uint8_t raw[8]={0};
   if(ae3_ee_bus_read(bus,address,raw,size)!=AE3_EE_BUS_OK)
    return AE3_TPR_MEMORY_FAULT;
   uint64_t value=0;
   for(unsigned q=0;q<size;q++)value|=(uint64_t)raw[q]<<(q*8u);
   w(&n.cpu,reg,size==4?sx((uint32_t)value):value);
  }
  n.cpu.pc=dest;n.cpu.retired++;*s=n;
  state=boundary(s);if(state!=AE3_TPR_YIELDED)return state;
 }
 return AE3_TPR_YIELDED;
}