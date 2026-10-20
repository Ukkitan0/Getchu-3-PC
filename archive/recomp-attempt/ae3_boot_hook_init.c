#include "ae3_boot_hook_init.h"
static uint32_t r32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t r64(const AE3_BootPrefix *s,unsigned i){return i?s->gpr[i][0]:0;}
static void put(AE3_BootPrefix *s,unsigned i,uint64_t value){if(i)s->gpr[i][0]=value;}
static uint64_t sext32(uint32_t x){return (uint64_t)(int64_t)(int32_t)x;}
static int fits(size_t pos,size_t size,size_t capacity){return pos<=capacity&&size<=capacity-pos;}
static int allowed(uint32_t pc){return pc==AE3_HOOK_CALL_PC || pc==AE3_HOOK_CALL_PC+4 ||
   (pc>=AE3_HOOK_ROUTINE_PC&&pc<=UINT32_C(0x0043FD84)&&!(pc&3)) ||
   (pc>=UINT32_C(0x0043FD88) && pc<=UINT32_C(0x0043FDC4) && !(pc&3)) ||
   (pc>=UINT32_C(0x0043FD00) && pc<=UINT32_C(0x0043FD0C) && !(pc&3)) ||
   (pc>=AE3_HOOK_WRAPPER_PC && pc<=AE3_HOOK_WRAPPER_PC+12 && !(pc&3));}
AE3_HookResult ae3_boot_hook_enter(AE3_BootHookInit *out,
                                   const AE3_BootSemaphoreInit *sema){
 if(!out||!sema||!sema->done||sema->completed!=2||sema->pending||
    sema->cpu.pc!=AE3_HOOK_CALL_PC||sema->cpu.in_delay_slot)return AE3_HOOK_INVALID;
 *out=(AE3_BootHookInit){0};out->cpu=sema->cpu;return AE3_HOOK_YIELDED;
}
AE3_HookResult ae3_boot_hook_slice(AE3_BootHookInit *s,uint8_t *ram,
                                   size_t bytes,uint32_t budget){
 if(!s||!ram||!allowed(s->cpu.pc)||s->cpu.in_delay_slot>1||s->pending>1 || s->registered>2 || s->guest_pending>1 || s->guest_completed>2)
    return AE3_HOOK_INVALID;
 if(s->pending)return AE3_HOOK_SET_SYSCALL;
 if(s->guest_pending)return AE3_HOOK_GUEST_SYSCALL;
 if(s->guest_completed==2 && s->cpu.pc==UINT32_C(0x0043FDC4))
    return AE3_HOOK_FIND_PAIR_COMPLETE;
 if(!budget)return AE3_HOOK_YIELDED;
 for(uint32_t j=0;j<budget;j++){
    uint32_t pc=s->cpu.pc;
    if(!fits(pc,4,bytes))return AE3_HOOK_MEMORY_FAULT;
    uint32_t w=r32(ram+pc);
    unsigned op=w>>26,rs=(w>>21)&31,rt=(w>>16)&31,sh=(w>>6)&31,fn=w&63;
    int16_t imm=(int16_t)(w&0xffffu);
    if(pc==UINT32_C(0x0043FDC4) && s->guest_completed==2)
       return AE3_HOOK_FIND_PAIR_COMPLETE;
    if(pc==UINT32_C(0x0043FD04)) {
       if(w!=0x0000000c || s->cpu.in_delay_slot ||
          (uint32_t)r64(&s->cpu,3)!=0x83 || s->registered!=2)
         return AE3_HOOK_UNSUPPORTED;
       s->invoked_number=(uint32_t)r64(&s->cpu,3);
       /* Resolved against the guest kernel table during route(). */
       s->guest_pending=1;
       return AE3_HOOK_GUEST_SYSCALL;
    }
    if(pc==AE3_HOOK_SYSCALL_PC){
       if(s->registered>=2)return AE3_HOOK_UNSUPPORTED;
       if(w!=0x0000000c || s->cpu.in_delay_slot || (uint32_t)r64(&s->cpu,3)!=0x74)
         return AE3_HOOK_UNSUPPORTED;
       s->syscall_number=0x74;
       s->guest_handler_number=(uint32_t)r64(&s->cpu,4);
       s->guest_handler_address=(uint32_t)r64(&s->cpu,5);
       s->pending=1;return AE3_HOOK_SET_SYSCALL;
    }
    AE3_BootPrefix t=s->cpu;
    uint32_t next=pc+4;
    if(t.in_delay_slot){next=t.branch_target;t.in_delay_slot=0;t.branch_target=0;}
    size_t store_at=0;uint64_t store_value=0;int store=0;
    if(w==0){}
    else if(op==9){put(&t,rt,sext32((uint32_t)r64(&s->cpu,rs)+(uint32_t)(int32_t)imm));}
    else if(op==15&&rs==0){put(&t,rt,sext32((w&0xffffu)<<16));}
    else if(op==0&&fn==0x2d&&sh==0){put(&t,(w>>11)&31u,r64(&s->cpu,rs)+r64(&s->cpu,rt));}
    else if(op==35){
       size_t a=(size_t)((uint32_t)r64(&s->cpu,rs)+(uint32_t)(int32_t)imm);
       if((a&3u)||!fits(a,4,bytes))return AE3_HOOK_MEMORY_FAULT;
       put(&t,rt,sext32(r32(ram+a)));
    }
    else if(op==63){
       store_at=(size_t)((uint32_t)r64(&s->cpu,rs)+(uint32_t)(int32_t)imm);
       if((store_at&7u)||!fits(store_at,8,bytes))return AE3_HOOK_MEMORY_FAULT;
       store_value=r64(&s->cpu,rt);store=1;
    }
    else if(op==0&&fn==8&&!sh&&!rt&&!((w>>11)&31)&&!s->cpu.in_delay_slot){
       t.in_delay_slot=1;t.branch_target=(uint32_t)r64(&s->cpu,rs);
    }
    else if(op==3&&!s->cpu.in_delay_slot){
       put(&t,31,pc+8);t.in_delay_slot=1;
       t.branch_target=((pc+4)&0xf0000000u)|((w&0x03ffffffu)<<2);
    }
    else return AE3_HOOK_UNSUPPORTED;
    if(!allowed(next))return AE3_HOOK_UNSUPPORTED;
    if(store)for(unsigned k=0;k<8;k++)ram[store_at+k]=(uint8_t)(store_value>>(8*k));
    t.pc=next;t.retired++;s->cpu=t;
 }
 return AE3_HOOK_YIELDED;
}

AE3_HookResult ae3_boot_hook_register(AE3_BootHookInit *s,
    AE3_EESyscallKernel *kernel, const uint8_t *ram, size_t bytes) {
 if(!s||!kernel||!ram||!s->pending||s->guest_pending||s->registered>=2||
    s->cpu.pc!=AE3_HOOK_SYSCALL_PC ||
    s->syscall_number!=0x74 || s->cpu.in_delay_slot ||
    s->guest_handler_number>=AE3_EE_SYSCALL_SLOTS) return AE3_HOOK_INVALID;
 if(ae3_ee_syscall_register(kernel,s->guest_handler_number,
     s->guest_handler_address,ram,bytes))return AE3_HOOK_UNSUPPORTED;
 s->registered_numbers[s->registered]=s->guest_handler_number;
 s->registered_handlers[s->registered]=s->guest_handler_address;
 s->registered++;
 /* The original kernel syscall 0x74 is void; preserve the guest v0. */
 s->cpu.pc+=4;s->cpu.retired++;
 s->pending=0;
 return AE3_HOOK_YIELDED;
}
AE3_HookResult ae3_boot_hook_route(AE3_BootHookInit *s,
    const AE3_EESyscallKernel *kernel, uint32_t *pc) {
 if(!s || !kernel || !pc || !s->guest_pending ||
    s->cpu.pc!=UINT32_C(0x0043FD04) ||s->invoked_number!=0x83)
    return AE3_HOOK_INVALID;
 uint32_t addr=0;
 if(ae3_ee_syscall_lookup(kernel,s->invoked_number,&addr))return AE3_HOOK_UNSUPPORTED;
 s->invoked_handler_pc=addr;
 *pc=addr;
 /* Leave the syscall suspended; completing it requires executing the
    registered guest handler, which uses unmapped kernel address space. */
 return AE3_HOOK_GUEST_SYSCALL;
}