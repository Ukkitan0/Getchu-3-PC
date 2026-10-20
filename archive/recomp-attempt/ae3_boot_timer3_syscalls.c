#include "ae3_boot_timer3_syscalls.h"
static uint64_t rr(const AE3_BootPrefix*s,unsigned i){return i?s->gpr[i][0]:0;}
static void wr(AE3_BootPrefix*s,unsigned i,uint64_t v){if(i)s->gpr[i][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static int validpc(uint32_t pc){
 return (pc>=0x440378 && pc<=0x440394 &&!(pc&3))||
        (pc>=0x4402d0 && pc<=0x4402ec &&!(pc&3));
}
AE3_TimerSysResult ae3_timer3_syscalls_enter(AE3_Timer3Syscalls*out,
 const AE3_BootTimer3Resume*previous){
 if(!out||!previous||!previous->complete||previous->cpu.pc!=AE3_TIMER3_NEXT_CALL||
    previous->cpu.in_delay_slot)return AE3_TS_INVALID;
 *out=(AE3_Timer3Syscalls){0};out->cpu=previous->cpu;return AE3_TS_YIELDED;
}
AE3_TimerSysResult ae3_timer3_syscalls_slice(AE3_Timer3Syscalls*s,
 AE3_EEMemoryBus*bus,uint32_t budget){
 if(!s||!bus||!bus->ram||s->cpu.in_delay_slot>1||(!validpc(s->cpu.pc)&&s->cpu.pc!=0x440398))
    return AE3_TS_INVALID;
 if(s->done)return AE3_TS_RETURNED;
 if(s->copy_active){
    AE3_CopyResult copied=ae3_guest_kernel_copy_slice(&s->copy,bus,budget);
    if(copied==AE3_COPY_MEMORY_FAULT)return AE3_TS_MEMORY_FAULT;
    if(copied<0)return AE3_TS_UNSUPPORTED;
    if(copied==AE3_COPY_YIELDED)return AE3_TS_YIELDED;
    const uint64_t retired=s->cpu.retired+s->copy.cpu.retired;
    AE3_BootPrefix newcpu=s->copy.cpu;
    /* The guest syscall wrapper must resume at its next instruction. The
     * handler's RA is not the live wrapper's RA; BIOS would supply its own
     * callback return convention. This is explicit compatibility bridging. */
    newcpu.gpr[31][0]=UINT32_C(0x440398);
    newcpu.pc=UINT32_C(0x4402e8);newcpu.in_delay_slot=0;newcpu.branch_target=0;
    newcpu.retired=retired;
    s->cpu=newcpu;s->copy_active=0;
    return AE3_TS_YIELDED;
 }
 if(s->cpu.pc==0x4402d4)return AE3_TS_SET_SYSCALL;
 if(s->cpu.pc==0x4402e4)return AE3_TS_COPY_SYSCALL;
 if(s->cpu.pc==0x440398){s->done=1;return AE3_TS_RETURNED;}
 for(uint32_t n=0;n<budget;n++){
    uint32_t pc=s->cpu.pc,w=0;
    if(ae3_ee_bus_read32(bus,pc,&w)!=AE3_EE_BUS_OK)return AE3_TS_MEMORY_FAULT;
    AE3_BootPrefix t=s->cpu;
    const int delay=t.in_delay_slot;
    uint32_t next=pc+4;
    if(delay){next=t.branch_target;t.in_delay_slot=0;t.branch_target=0;}
    switch(pc){
    case 0x440378:case 0x440390:
       if(delay||w!=(pc==0x440378?0x0c1100b4u:0x0c1100b8u))return AE3_TS_UNSUPPORTED;
       wr(&t,31,pc+8);t.in_delay_slot=1;
       t.branch_target=pc==0x440378?0x4402d0:0x4402e0;break;
    case 0x44037c:
       if(w!=0x26110010u||!delay)return AE3_TS_UNSUPPORTED;
       wr(&t,17,sx((uint32_t)rr(&t,16)+16u));break;
    case 0x4402d0:case 0x4402e0:
       if(w!=(pc==0x4402d0?0x24030074u:0x2403005au))return AE3_TS_UNSUPPORTED;
       wr(&t,3,pc==0x4402d0?0x74:0x5a);break;
    case 0x4402d8:case 0x4402e8:
       if(w!=0x03e00008u||delay||
          (uint32_t)rr(&t,31)!=(pc==0x4402d8?0x440380u:0x440398u))
          return AE3_TS_UNSUPPORTED;
       t.in_delay_slot=1;t.branch_target=(uint32_t)rr(&t,31);break;
    case 0x4402dc:case 0x4402ec:
       if(w||!delay)return AE3_TS_UNSUPPORTED;
       break;
    case 0x440380:case 0x440384:
       if(w!=(pc==0x440380?0x3c05006au:0x3c048007u))return AE3_TS_UNSUPPORTED;
       wr(&t,pc==0x440380?5:4,sx(pc==0x440380?0x006a0000u:0x80070000u));break;
    case 0x440388:case 0x44038c:
       if(w!=(pc==0x440388?0x24a52678u:0x34846000u))return AE3_TS_UNSUPPORTED;
       if(pc==0x440388)wr(&t,5,sx((uint32_t)rr(&t,5)+0x2678u));
       else wr(&t,4,rr(&t,4)|0x6000u);
       break;
    case 0x440394:
       if(w!=0x24060740u||!delay)return AE3_TS_UNSUPPORTED;
       wr(&t,6,0x740);break;
    default:return AE3_TS_UNSUPPORTED;
    }
    if(next!=0x440398&&!validpc(next))return AE3_TS_UNSUPPORTED;
    t.pc=next;t.retired++;s->cpu=t;
    if(next==0x4402d4)return AE3_TS_SET_SYSCALL;
    if(next==0x4402e4)return AE3_TS_COPY_SYSCALL;
    if(next==0x440398){s->done=1;return AE3_TS_RETURNED;}
 }
 return AE3_TS_YIELDED;
}
AE3_TimerSysResult ae3_timer3_syscalls_dispatch(AE3_Timer3Syscalls*s,
 AE3_EEMemoryBus*bus,AE3_EESyscallKernel*kernel){
 if(!s||!bus||!kernel||!bus->ram||s->copy_active)return AE3_TS_INVALID;
 if(s->cpu.pc==0x4402d4 && rr(&s->cpu,3)==0x74u){
    const uint32_t number=(uint32_t)rr(&s->cpu,4);
    const uint32_t handler=(uint32_t)rr(&s->cpu,5);
    if(number!=0x5au||handler!=AE3_GUEST_COPY_ENTRY)return AE3_TS_UNSUPPORTED;
    uint32_t old=0;
    (void)ae3_ee_syscall_lookup(kernel,number,&old);
    if(ae3_ee_syscall_register(kernel,number,handler,bus->ram,bus->ram_size))
       return AE3_TS_UNSUPPORTED;
    /* Test-only kernel return convention: previous handler or zero. */
    wr(&s->cpu,2,old);s->cpu.pc=0x4402d8;s->cpu.retired++;
    return AE3_TS_YIELDED;
 }
 if(s->cpu.pc==0x4402e4 && rr(&s->cpu,3)==0x5au){
    uint32_t handler=0;
    if(ae3_ee_syscall_lookup(kernel,0x5a,&handler)||handler!=AE3_GUEST_COPY_ENTRY)
       return AE3_TS_UNSUPPORTED;
    if(ae3_guest_kernel_copy_init(&s->copy,(uint32_t)rr(&s->cpu,4),
        (uint32_t)rr(&s->cpu,5),(uint32_t)rr(&s->cpu,6),0x4402e8)!=AE3_COPY_YIELDED)
        return AE3_TS_INVALID;
    AE3_BootPrefix guest=s->cpu;
    guest.pc=AE3_GUEST_COPY_ENTRY;guest.gpr[31][0]=0x4402e8;
    guest.in_delay_slot=0;guest.branch_target=0;guest.retired=0;
    s->copy.cpu=guest;
    s->copy_active=1;s->cpu.retired++;
    return AE3_TS_YIELDED;
 }
 return AE3_TS_INVALID;
}