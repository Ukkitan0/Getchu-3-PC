#include "ae3_boot_timer2_continue.h"
static uint64_t get(const AE3_BootPrefix *s,unsigned reg){return reg?s->gpr[reg][0]:0;}
static void set(AE3_BootPrefix *s,unsigned reg,uint64_t val){if(reg)s->gpr[reg][0]=val;}
static uint64_t sign32(uint32_t val){return (uint64_t)(int64_t)(int32_t)val;}
static const struct {uint32_t pc,word;} program[]={
 {0x00440590,0x8c700000},{0x00440594,0x3442fffc},
 {0x00440598,0x02021024},{0x0044059c,0x00548025},
 {0x004405a0,0x36100300},{0x004405a4,0x32020080},
 {0x004405a8,0x14400005},{0x004405ac,0x0000202d}
};
AE3_BootTimer2Result ae3_boot_timer2_enter(AE3_BootTimer2Continue *s,
 const AE3_BootIntcCritical *prior){
 if(!s||!prior||!prior->enabled||prior->cpu.pc!=AE3_T2_MODE_GUEST_LW||
    prior->cpu.in_delay_slot||((uint32_t)get(&prior->cpu,3)!=AE3_T2_MODE_REGISTER))
    return AE3_BT2_INVALID;
 *s=(AE3_BootTimer2Continue){0};s->cpu=prior->cpu;s->cop0_status=prior->cop0_status;s->entered=1;
 return AE3_BT2_YIELDED;
}
AE3_BootTimer2Result ae3_boot_timer2_slice(AE3_BootTimer2Continue *s,
 const AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->entered||s->cpu.in_delay_slot>1)return AE3_BT2_INVALID;
 for(uint32_t i=0;i<budget;i++){
    const uint32_t pc=s->cpu.pc;
    if(pc==AE3_T2_GUEST_COUNT_SETUP)return AE3_BT2_TIMER_SETUP_CALL;
    if(pc==AE3_T2_GUEST_MODE_SETUP)return AE3_BT2_MODE_SETUP_CALL;
    size_t j=0;
    for(;j<sizeof(program)/sizeof(program[0]);j++)if(program[j].pc==pc)break;
    if(j==sizeof(program)/sizeof(program[0])||
       (s->cpu.in_delay_slot&&pc!=0x004405ac))return AE3_BT2_UNSUPPORTED;
    uint32_t original=0;
    if(ae3_ee_bus_read32(bus,pc,&original)!=AE3_EE_BUS_OK)return AE3_BT2_MEMORY_FAULT;
    if(original!=program[j].word)return AE3_BT2_UNSUPPORTED;
    AE3_BootPrefix next=s->cpu;
    uint32_t target=next.in_delay_slot?next.branch_target:pc+4u;
    int delay=next.in_delay_slot;
    if(delay){next.in_delay_slot=0;next.branch_target=0;}
    switch(pc){
    case 0x00440590:{
        uint32_t value=0;
        if(ae3_ee_bus_read32(bus,(uint32_t)get(&next,3),&value)!=AE3_EE_BUS_OK)
            return AE3_BT2_MEMORY_FAULT;
        set(&next,16,sign32(value));break;
    }
    case 0x00440594:set(&next,2,sign32((uint32_t)get(&next,2)|0xfffcu));break;
    case 0x00440598:set(&next,2,get(&next,16)&get(&next,2));break;
    case 0x0044059c:set(&next,16,get(&next,2)|get(&next,20));break;
    case 0x004405a0:set(&next,16,sign32((uint32_t)get(&next,16)|0x300u));break;
    case 0x004405a4:set(&next,2,get(&next,16)&0x80u);break;
    case 0x004405a8:
        if(delay)return AE3_BT2_UNSUPPORTED;
        next.in_delay_slot=1;
        next.branch_target=get(&next,2)?AE3_T2_GUEST_MODE_SETUP:AE3_T2_GUEST_COUNT_SETUP;
        break;
    case 0x004405ac:
        if(!delay)return AE3_BT2_UNSUPPORTED;
        set(&next,4,0);break;
    default:return AE3_BT2_UNSUPPORTED;
    }
    next.pc=target;next.retired++;
    s->cpu=next;
    if(next.pc==AE3_T2_GUEST_COUNT_SETUP)return AE3_BT2_TIMER_SETUP_CALL;
    if(next.pc==AE3_T2_GUEST_MODE_SETUP)return AE3_BT2_MODE_SETUP_CALL;
 }
 return AE3_BT2_YIELDED;
}