#include "ae3_boot_thread_setup.h"
static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static uint64_t sext32(uint32_t word) {return (uint64_t)(int64_t)(int32_t)word;}
static uint64_t reg(const AE3_BootPrefix *s,unsigned r) {return r?s->gpr[r][0]:0;}
static void set(AE3_BootPrefix *s,unsigned r,uint64_t v) {if(r)s->gpr[r][0]=v;}
AE3_ThreadStatus ae3_boot_thread_enter(AE3_BootThreadSetup *thread,
                                        const AE3_BootMemoryFill *fill) {
    if (!thread||!fill||!fill->finished||fill->cpu.pc!=AE3_BOOT_RETURN||
        fill->cpu.in_delay_slot)return AE3_THREAD_BAD_STATE;
    *thread=(AE3_BootThreadSetup){0};
    thread->cpu=fill->cpu;
    return AE3_THREAD_YIELDED;
}
AE3_ThreadStatus ae3_boot_thread_slice(AE3_BootThreadSetup *thread,
                                        const uint8_t *ram, size_t ram_size,
                                        uint32_t budget) {
    if (!thread||!ram)return AE3_THREAD_BAD_STATE;
    AE3_BootPrefix *s=&thread->cpu;
    if (s->pc < AE3_BOOT_RETURN || s->pc>AE3_SETUP_THREAD_SYSCALL_PC ||
        s->pc&3 || s->in_delay_slot || thread->finished>1)
        return AE3_THREAD_BAD_STATE;
    if (thread->finished)return s->pc==AE3_SETUP_THREAD_SYSCALL_PC
        ? AE3_THREAD_SYSCALL : AE3_THREAD_BAD_STATE;
    if(!budget)return AE3_THREAD_YIELDED;
    for(uint32_t i=0;i<budget;i++) {
        if(s->pc==AE3_SETUP_THREAD_SYSCALL_PC){thread->finished=1;return AE3_THREAD_SYSCALL;}
        if(s->pc+4>ram_size)return AE3_THREAD_MEMORY_FAULT;
        uint32_t w=le32(ram+s->pc);
        unsigned op=w>>26,rs=(w>>21)&31,rt=(w>>16)&31,rd=(w>>11)&31,sh=(w>>6)&31,fn=w&63;
        uint32_t old=s->pc;
        AE3_BootPrefix c=*s;
        if (w==0) { }
        else if (op==15 && rs==0) set(&c,rt,sext32((w&0xffff)<<16));
        else if (op==9) set(&c,rt,sext32((uint32_t)reg(s,rs)+(uint32_t)(int16_t)(w&0xffff)));
        else if (op==0 && fn==0x2d && sh==0) set(&c,rd,reg(s,rs)+reg(s,rt));
        else return AE3_THREAD_UNSUPPORTED;
        c.pc=old+4;c.retired++;
        *s=c;
        if(s->pc==AE3_SETUP_THREAD_SYSCALL_PC){thread->finished=1;return AE3_THREAD_SYSCALL;}
    }
    return AE3_THREAD_YIELDED;
}