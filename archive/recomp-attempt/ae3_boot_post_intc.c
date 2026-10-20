#include "ae3_boot_post_intc.h"
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sign32(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static const struct {uint32_t pc,op;} expected[]={
 {0x004357d4u,0x0000000cu}, {0x004357d8u,0x03e00008u},
 {0x004357dcu,0x00000000u}, {0x00440560u,0x0040182du},
 {0x00440564u,0x04630004u}, {0x00440568u,0x26422e20u},
 {0x0044056cu,0x3c028000u}, {0x00440570u,0x1000001cu},
 {0x00440574u,0x34429021u}
};
AE3_PostIntcResult ae3_boot_post_intc_enter(AE3_BootPostIntc *out,
    const AE3_BootAllocInit *a){
    if(!out||!a||a->cpu.pc!=AE3_ALLOC_INTC_SYSCALL_PC||
       a->cpu.in_delay_slot||r(&a->cpu,3)!=16||r(&a->cpu,4)!=11||
       r(&a->cpu,5)!=0x00440a50u||r(&a->cpu,6)!=0||
       r(&a->cpu,31)!=0x00440560u||!a->initialized_path||
       a->pointer_stores!=128||a->freelist_stores!=66)
        return AE3_POST_INTC_INVALID;
    *out=(AE3_BootPostIntc){0};out->cpu=a->cpu;
    return AE3_POST_INTC_YIELDED;
}
AE3_PostIntcResult ae3_boot_post_intc_service(AE3_BootPostIntc *s,
    AE3_EEIntcKernel *kernel,const AE3_EEMemoryBus *bus){
    if(!s||!kernel||!bus||s->serviced||
       s->cpu.pc!=AE3_ALLOC_INTC_SYSCALL_PC||s->cpu.in_delay_slot||
       r(&s->cpu,3)!=0x10)return AE3_POST_INTC_INVALID;
    uint32_t word=0;
    if(ae3_ee_bus_read32(bus,s->cpu.pc,&word)!=AE3_EE_BUS_OK)
        return AE3_POST_INTC_MEMORY_FAULT;
    if(word!=expected[0].op)return AE3_POST_INTC_UNSUPPORTED;
    /* The guest syscall retires only after service; failures are an ordinary
       negative guest return value, never fabricated success. */
    const int32_t id=ae3_ee_intc_add(kernel,bus,(uint32_t)r(&s->cpu,4),
                                   (uint32_t)r(&s->cpu,5),(uint32_t)r(&s->cpu,6));
    s->cpu.gpr[2][0]=sign32((uint32_t)id);
    s->cpu.pc=0x004357d8u;s->cpu.retired++;
    s->serviced=1;s->handler_id=id>0?(uint32_t)id:0;
    return AE3_POST_INTC_YIELDED;
}
AE3_PostIntcResult ae3_boot_post_intc_slice(AE3_BootPostIntc *s,
    const AE3_EEMemoryBus *bus,uint32_t budget){
    if(!s||!bus||!s->serviced||s->cpu.in_delay_slot>1)return AE3_POST_INTC_INVALID;
    if(s->cpu.pc==AE3_POST_INTC_NEXT_PC)return AE3_POST_INTC_NEXT_CALL;
    if(s->cpu.pc==AE3_POST_INTC_FAIL_PC)return AE3_POST_INTC_FAILED_PATH;
    for(uint32_t i=0;i<budget;i++){
        const uint32_t pc=s->cpu.pc;
        uint32_t ins=0;unsigned j=0;
        for(;j<sizeof(expected)/sizeof(expected[0]);j++)if(expected[j].pc==pc)break;
        if(j==sizeof(expected)/sizeof(expected[0]))return AE3_POST_INTC_UNSUPPORTED;
        if(ae3_ee_bus_read32(bus,pc,&ins)!=AE3_EE_BUS_OK)
            return AE3_POST_INTC_MEMORY_FAULT;
        if(ins!=expected[j].op)return AE3_POST_INTC_UNSUPPORTED;
        AE3_BootPrefix n=s->cpu;
        const int slot=n.in_delay_slot;
        uint32_t dest=slot?n.branch_target:pc+4;
        if(slot){n.in_delay_slot=0;n.branch_target=0;}
        switch(pc){
        case 0x004357d8u:
            if(slot)return AE3_POST_INTC_UNSUPPORTED;
            n.in_delay_slot=1;n.branch_target=(uint32_t)r(&n,31);break;
        case 0x004357dcu:
            if(!slot||dest!=0x00440560u)return AE3_POST_INTC_UNSUPPORTED;
            break;
        case 0x00440560u:
            if(slot)return AE3_POST_INTC_UNSUPPORTED;
            w(&n,3,r(&n,2));break;
        case 0x00440564u:
            if(slot)return AE3_POST_INTC_UNSUPPORTED;
            /* bgezl: untaken delay-slot is annulled entirely. */
            if((int64_t)r(&n,3)>=0){n.in_delay_slot=1;n.branch_target=0x00440578u;}
            else dest=0x0044056cu;
            break;
        case 0x00440568u:
            if(!slot||dest!=0x00440578u)return AE3_POST_INTC_UNSUPPORTED;
            w(&n,2,sign32((uint32_t)r(&n,18)+0x2e20u));break;
        case 0x0044056cu:
            if(slot)return AE3_POST_INTC_UNSUPPORTED;
            w(&n,2,sign32(0x80000000u));break;
        case 0x00440570u:
            if(slot)return AE3_POST_INTC_UNSUPPORTED;
            n.in_delay_slot=1;n.branch_target=0x004405e4u;break;
        case 0x00440574u:
            if(!slot||dest!=0x004405e4u)return AE3_POST_INTC_UNSUPPORTED;
            w(&n,2,r(&n,2)|0x9021u);break;
        default:return AE3_POST_INTC_UNSUPPORTED;
        }
        n.pc=dest;n.retired++;
        s->cpu=n;
        if(dest==AE3_POST_INTC_NEXT_PC)return AE3_POST_INTC_NEXT_CALL;
        if(dest==AE3_POST_INTC_FAIL_PC)return AE3_POST_INTC_FAILED_PATH;
    }
    return AE3_POST_INTC_YIELDED;
}