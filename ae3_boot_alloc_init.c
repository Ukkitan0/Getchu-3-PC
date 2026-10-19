#include "ae3_boot_alloc_init.h"
#include <string.h>

static uint64_t get(const AE3_BootPrefix *s,unsigned r){return r?s->gpr[r][0]:0;}
static void set(AE3_BootPrefix *s,unsigned r,uint64_t v){if(r)s->gpr[r][0]=v;}
static uint64_t sx32(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static int allowed(uint32_t pc){
    return pc==AE3_ALLOC_PARENT_JAL||pc==AE3_ALLOC_PARENT_JAL+4||
           pc==AE3_ALLOC_MEMSET_ENTRY||pc==AE3_ALLOC_PARENT_RETURN||
           pc==AE3_ALLOC_NEXT_CALL||
           (pc>=0x004404B0u&&pc<=0x00440558u&&!(pc&3u))||
           (pc>=0x004405E4u&&pc<=0x00440600u&&!(pc&3u))||
           (pc>=0x00441780u&&pc<=0x004417C8u&&!(pc&3u));
}
AE3_AllocResult ae3_boot_alloc_enter(AE3_BootAllocInit *s,const AE3_BootPostPatch *prior){
    if(!s||!prior||!prior->done||prior->cpu.pc!=AE3_ALLOC_PARENT_JAL||
       prior->cpu.in_delay_slot)return AE3_ALLOC_INVALID;
    *s=(AE3_BootAllocInit){0};s->cpu=prior->cpu;
    return AE3_ALLOC_YIELDED;
}
static AE3_AllocResult event(const AE3_BootAllocInit *s){
    if(s->cpu.pc==AE3_ALLOC_MEMSET_ENTRY)return AE3_ALLOC_ZERO_FILL_REQUEST;
    if(s->cpu.pc==AE3_ALLOC_NEXT_CALL)return AE3_ALLOC_NEXT_CALL_PENDING;
    if(s->cpu.pc==AE3_ALLOC_PARENT_RETURN)return AE3_ALLOC_RETURNED;
    return AE3_ALLOC_YIELDED;
}
AE3_AllocResult ae3_boot_alloc_slice(AE3_BootAllocInit *s,AE3_EEMemoryBus *bus,uint32_t budget){
    if(!s||!bus||!bus->ram||!allowed(s->cpu.pc)||s->cpu.in_delay_slot>1u)
        return AE3_ALLOC_INVALID;
    if(s->done)return AE3_ALLOC_RETURNED;
    if(event(s)!=AE3_ALLOC_YIELDED)return event(s);
    for(uint32_t i=0;i<budget;i++){
        const uint32_t pc=s->cpu.pc;
        uint32_t w=0;
        if(ae3_ee_bus_read32(bus,pc,&w)!=AE3_EE_BUS_OK)return AE3_ALLOC_MEMORY_FAULT;
        const unsigned op=w>>26,rs=(w>>21)&31u,rt=(w>>16)&31u,rd=(w>>11)&31u,fn=w&63u;
        const int32_t imm=(int16_t)(w&0xffffu);
        const uint32_t branch=pc+4u+(uint32_t)(imm*4);
        AE3_BootPrefix n=s->cpu;
        const int slot=n.in_delay_slot;
        uint32_t dest=slot?n.branch_target:pc+4u;
        if(slot){n.in_delay_slot=0;n.branch_target=0;}
        uint32_t store_addr=0;uint8_t raw[8];size_t store_len=0;
        if(w==0){}
        else if(op==9u){set(&n,rt,sx32((uint32_t)get(&n,rs)+(uint32_t)imm));}
        else if(op==15u&&rs==0u){set(&n,rt,sx32((w&0xffffu)<<16));}
        else if(op==13u){set(&n,rt,get(&n,rs)|(w&0xffffu));}
        else if(op==35u){
            const uint32_t a=(uint32_t)get(&n,rs)+(uint32_t)imm;
            uint32_t value=0;
            if(ae3_ee_bus_read32(bus,a,&value)!=AE3_EE_BUS_OK)return AE3_ALLOC_MEMORY_FAULT;
            set(&n,rt,sx32(value));
        }
        else if(op==43u||op==63u){
            store_addr=(uint32_t)get(&n,rs)+(uint32_t)imm;
            store_len=op==43u?4u:8u;
            if(store_addr&(uint32_t)(store_len-1u))return AE3_ALLOC_MEMORY_FAULT;
            uint64_t val=get(&n,rt);
            for(size_t k=0;k<store_len;k++)raw[k]=(uint8_t)(val>>(8u*k));
        }
        else if(op==55u){
            const uint32_t a=(uint32_t)get(&n,rs)+(uint32_t)imm;
            uint8_t buf[8];if((a&7u)||ae3_ee_bus_read(bus,a,buf,8)!=AE3_EE_BUS_OK)
                return AE3_ALLOC_MEMORY_FAULT;
            uint64_t v=0;for(unsigned k=0;k<8;k++)v|=(uint64_t)buf[k]<<(k*8u);
            set(&n,rt,v);
        }
        else if(op==0u&&fn==45u){set(&n,rd,get(&n,rs)+get(&n,rt));}
        else if(op==0u&&fn==8u&&!slot){
            const uint32_t target=(uint32_t)get(&n,rs);
            if(rs!=31||!(target==0x00440548u||target==AE3_ALLOC_PARENT_RETURN))
                return AE3_ALLOC_UNSUPPORTED;
            n.in_delay_slot=1;n.branch_target=target;
        }
        else if(op==1u&&(rt==0u||rt==1u)&&!slot){
            const int negative=(int64_t)get(&n,rs)<0;
            if((rt==0u&&negative)||(rt==1u&&!negative)){
                n.in_delay_slot=1;n.branch_target=branch;
            }else {n.in_delay_slot=1;n.branch_target=pc+8u;}
        }
        else if(op==4u&&!slot){
            n.in_delay_slot=1;n.branch_target=get(&n,rs)==get(&n,rt)?branch:pc+8u;
        }
        else if(op==3u&&!slot){
            const uint32_t target=((pc+4u)&0xf0000000u)|((w&0x03ffffffu)<<2);
            if(!((pc==AE3_ALLOC_PARENT_JAL&&target==AE3_ALLOC_ENTRY)||
                 (pc==0x00440504u&&target==AE3_ALLOC_MEMSET_ENTRY)||
                 (pc==0x00440540u&&target==AE3_ALLOC_FREELIST_ENTRY)))
                return AE3_ALLOC_UNSUPPORTED;
            set(&n,31,pc+8u);n.in_delay_slot=1;n.branch_target=target;
        }
        else return AE3_ALLOC_UNSUPPORTED;
        if(!allowed(dest)||(n.in_delay_slot&&!allowed(n.branch_target)))
            return AE3_ALLOC_UNSUPPORTED;
        if(store_len){
            if(ae3_ee_bus_write(bus,store_addr,raw,store_len)!=AE3_EE_BUS_OK)
                return AE3_ALLOC_MEMORY_FAULT;
        }
        n.pc=dest;n.retired++;
        s->cpu=n;
        if(pc==0x004404F0u)s->initialized_path=1;
        if(pc==0x00440520u)s->pointer_stores++;
        if(pc==0x004417A0u||pc==0x00441790u||pc==0x004417C8u)s->freelist_stores++;
        if(dest==AE3_ALLOC_PARENT_RETURN)s->done=1;
        if(event(s)!=AE3_ALLOC_YIELDED)return event(s);
    }
    return AE3_ALLOC_YIELDED;
}
AE3_AllocResult ae3_boot_alloc_dispatch_zero(AE3_BootAllocInit *s,AE3_EEMemoryBus *bus){
    if(!s||!bus||!bus->ram||s->cpu.pc!=AE3_ALLOC_MEMSET_ENTRY||
       s->cpu.in_delay_slot||s->translated_zero_fills||
       (uint32_t)get(&s->cpu,4)!=AE3_ALLOC_AREA||get(&s->cpu,5)!=0||
       get(&s->cpu,6)!=AE3_ALLOC_CLEAR_BYTES||get(&s->cpu,31)!=0x0044050cu)
        return AE3_ALLOC_INVALID;
    /* Decompilation: the original optimized R5900 MMI memset is translated
       to this equivalent bounded byte operation for this exact invocation.
       We do not pretend that its internal MMI instructions were executed. */
    const uint8_t cleared[AE3_ALLOC_CLEAR_BYTES]={0};
    if(ae3_ee_bus_write(bus,AE3_ALLOC_AREA,cleared,sizeof(cleared))!=AE3_EE_BUS_OK)
        return AE3_ALLOC_MEMORY_FAULT;
    s->cpu.gpr[2][0]=AE3_ALLOC_AREA;
    s->cpu.pc=0x0044050cu;s->translated_zero_fills++;
    return AE3_ALLOC_YIELDED;
}

AE3_AllocResult ae3_boot_alloc_follow_intc(AE3_BootAllocInit *s,
    AE3_EEMemoryBus *bus){
    if(!s||!bus||!bus->ram)return AE3_ALLOC_INVALID;
    if(s->cpu.pc==AE3_ALLOC_INTC_SYSCALL_PC)return AE3_ALLOC_INTC_SYSCALL;
    if(s->cpu.pc!=AE3_ALLOC_NEXT_CALL||s->cpu.in_delay_slot||
       !s->initialized_path||s->pointer_stores!=128||
       s->freelist_stores!=66||s->translated_zero_fills!=1||
       (uint32_t)get(&s->cpu,4)!=11u||(uint32_t)get(&s->cpu,5)!=0x00440A50u||
       get(&s->cpu,6)!=0)
        return AE3_ALLOC_INVALID;
    /* Prevalidate every original guest instruction, including the syscall,
       BEFORE applying any register or PC changes (atomic retry on faults). */
    static const uint32_t pc[4]={0x00440558u,0x0044055cu,
                                  0x004357d0u,0x004357d4u};
    static const uint32_t op[4]={0x0c10d5f4u,0x0000382du,
                                  0x24030010u,0x0000000cu};
    for(unsigned i=0;i<4;i++){
        uint32_t word=0;
        if(ae3_ee_bus_read32(bus,pc[i],&word)!=AE3_EE_BUS_OK)
            return AE3_ALLOC_MEMORY_FAULT;
        if(word!=op[i])return AE3_ALLOC_UNSUPPORTED;
    }
    AE3_BootPrefix n=s->cpu;
    n.gpr[31][0]=0x00440560u;/* retail JAL link */
    n.gpr[7][0]=0;/* original JAL delay slot: MOVE a3,zero */
    n.gpr[3][0]=0x10u;/* original ADDIU v1,zero,16 */
    n.pc=AE3_ALLOC_INTC_SYSCALL_PC;
    n.retired+=3u;/* JAL + delay + ADDIU; syscall not executed */
    s->cpu=n;
    return AE3_ALLOC_INTC_SYSCALL;
}