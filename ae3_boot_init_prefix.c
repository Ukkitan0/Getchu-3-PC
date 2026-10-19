#include "ae3_boot_init_prefix.h"
static uint32_t read32(const uint8_t *p) {return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t signext(uint32_t v) {return (uint64_t)(int64_t)(int32_t)v;}
AE3_InitResult ae3_boot_init_enter(AE3_BootInitPrefix *s,const AE3_BootKernelBridge *k) {
    if(!s||!k||!k->finished||!k->completed_heap||!k->completed_thread||
       k->cpu.pc!=AE3_INIT_ENTRY_JAL||k->cpu.in_delay_slot) return AE3_INIT_INVALID;
    *s=(AE3_BootInitPrefix){0};s->cpu=k->cpu;return AE3_INIT_YIELDED;
}
AE3_InitResult ae3_boot_init_slice(AE3_BootInitPrefix *s,uint8_t *ram,size_t bytes,uint32_t budget) {
    if(!s||!ram||s->done>1||s->cpu.in_delay_slot>1||
       (s->cpu.pc!=AE3_INIT_ENTRY_JAL&&s->cpu.pc!=AE3_INIT_ENTRY_JAL+4 &&
        s->cpu.pc!=AE3_KERNEL_NEXT_CALL_TARGET&&s->cpu.pc!=AE3_KERNEL_NEXT_CALL_TARGET+4&&
        s->cpu.pc!=AE3_INIT_FIRST_SUBCALL_PC))return AE3_INIT_INVALID;
    if(s->done)return s->cpu.pc==AE3_INIT_FIRST_SUBCALL_PC?AE3_INIT_SUBCALL:AE3_INIT_INVALID;
    if(!budget)return AE3_INIT_YIELDED;
    for(uint32_t i=0;i<budget;i++) {
        uint32_t pc=s->cpu.pc;
        if(pc==AE3_INIT_FIRST_SUBCALL_PC){s->done=1;return AE3_INIT_SUBCALL;}
        if((size_t)pc>bytes||bytes-(size_t)pc<4)return AE3_INIT_MEMORY_FAULT;
        uint32_t w=read32(ram+pc);
        AE3_BootPrefix tmp=s->cpu;
        uint32_t next=pc+4;
        int store=0;size_t addr=0;uint64_t data=0;
        if(pc==AE3_INIT_ENTRY_JAL && w==UINT32_C(0x0c10ff96) && !s->cpu.in_delay_slot) {
            tmp.gpr[31][0]=pc+8;
            tmp.in_delay_slot=1;
            tmp.branch_target=AE3_KERNEL_NEXT_CALL_TARGET;
        } else if(pc==AE3_INIT_ENTRY_JAL+4 && w==0 && s->cpu.in_delay_slot) {
            next=s->cpu.branch_target;tmp.in_delay_slot=0;tmp.branch_target=0;
        } else if(pc==AE3_KERNEL_NEXT_CALL_TARGET && w==UINT32_C(0x27bdfff0)) {
            tmp.gpr[29][0]=signext((uint32_t)s->cpu.gpr[29][0]-(uint32_t)16);
        } else if(pc==AE3_KERNEL_NEXT_CALL_TARGET+4 && w==UINT32_C(0xffbf0000)) {
            addr=(size_t)(uint32_t)s->cpu.gpr[29][0];
            if((addr&7)||addr>bytes||bytes-addr<8)return AE3_INIT_MEMORY_FAULT;
            data=s->cpu.gpr[31][0];store=1;
        } else return AE3_INIT_UNSUPPORTED;
        if(store)for(unsigned b=0;b<8;b++)ram[addr+b]=(uint8_t)(data>>(8*b));
        tmp.pc=next;tmp.retired++;
        s->cpu=tmp;
        if(next==AE3_INIT_FIRST_SUBCALL_PC){s->done=1;return AE3_INIT_SUBCALL;}
    }
    return AE3_INIT_YIELDED;
}