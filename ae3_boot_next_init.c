#include "ae3_boot_next_init.h"
static uint64_t r(const AE3_BootPrefix*s,unsigned i){return i?s->gpr[i][0]:0;}
static void wr(AE3_BootPrefix*s,unsigned i,uint64_t v){if(i)s->gpr[i][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static int valid_pc(uint32_t pc){return pc==AE3_NEXT_INIT_JAL ||
    pc==AE3_NEXT_INIT_JAL+4 ||
    (pc>=AE3_NEXT_INIT_CALLEE && pc<=AE3_NEXT_INIT_MMIO_PC && !(pc&3));}
AE3_NextInitResult ae3_boot_next_init_enter(AE3_BootNextInit *out,
    const AE3_BootPostFind *post){
    if(!out||!post||!post->done||post->cpu.pc!=AE3_NEXT_INIT_JAL||
       post->cpu.in_delay_slot)return AE3_NEXT_INVALID;
    *out=(AE3_BootNextInit){0};out->cpu=post->cpu;
    return AE3_NEXT_YIELDED;
}
AE3_NextInitResult ae3_boot_next_init_slice(AE3_BootNextInit*s,
    AE3_EEMemoryBus *bus,uint32_t budget){
    if(!s||!bus||!bus->ram||bus->ram_size<AE3_EE_RAM_BYTES||
       !valid_pc(s->cpu.pc)||s->cpu.in_delay_slot>1)return AE3_NEXT_INVALID;
    if(s->cpu.pc==AE3_NEXT_INIT_MMIO_PC){
        uint32_t w=0;
        if(ae3_ee_bus_read32(bus,s->cpu.pc,&w)!=AE3_EE_BUS_OK ||
           w!=UINT32_C(0x8c430000) ||
           (uint32_t)r(&s->cpu,2)!=AE3_NEXT_INIT_MMIO_ADDRESS)
            return AE3_NEXT_UNSUPPORTED;
        s->waiting_mmio=1;return AE3_NEXT_MMIO_PENDING;
    }
    for(uint32_t i=0;i<budget;i++){
        uint32_t pc=s->cpu.pc,w=0;
        if(ae3_ee_bus_read32(bus,pc,&w)!=AE3_EE_BUS_OK)return AE3_NEXT_MEMORY_FAULT;
        unsigned op=w>>26,rs=(w>>21)&31u,rt=(w>>16)&31u;
        int32_t imm=(int16_t)(w&65535u);
        uint32_t advance=pc+4;
        AE3_BootPrefix t=s->cpu;
        if(s->cpu.in_delay_slot){
            advance=s->cpu.branch_target;
            t.branch_target=0;t.in_delay_slot=0;
        }
        uint8_t raw[8];uint32_t addr=0;int store=0;
        if(!w){}
        else if(op==3&&!s->cpu.in_delay_slot){
            uint32_t dest=((pc+4)&UINT32_C(0xf0000000))|((w&UINT32_C(0x03ffffff))<<2);
            if(pc!=AE3_NEXT_INIT_JAL ||dest!=AE3_NEXT_INIT_CALLEE)
                return AE3_NEXT_UNSUPPORTED;
            wr(&t,31,pc+8);t.in_delay_slot=1;t.branch_target=dest;
        }
        else if(op==9){wr(&t,rt,sx((uint32_t)r(&s->cpu,rs)+(uint32_t)imm));}
        else if(op==15&&!rs){wr(&t,rt,sx((w&65535u)<<16));}
        else if(op==13){wr(&t,rt,r(&s->cpu,rs)|(w&65535u));}
        else if(op==63){
            addr=(uint32_t)r(&s->cpu,rs)+(uint32_t)imm;
            if((addr&7)||ae3_ee_bus_read(bus,addr,raw,8)!=AE3_EE_BUS_OK)
                return AE3_NEXT_MEMORY_FAULT;
            uint64_t v=r(&s->cpu,rt);
            for(unsigned k=0;k<8;k++)raw[k]=(uint8_t)(v>>(8*k));
            store=1;
        } else return AE3_NEXT_UNSUPPORTED;
        if(!valid_pc(advance)||(t.in_delay_slot&&!valid_pc(t.branch_target)))
            return AE3_NEXT_UNSUPPORTED;
        if(store&&ae3_ee_bus_write(bus,addr,raw,8)!=AE3_EE_BUS_OK)
            return AE3_NEXT_MEMORY_FAULT;
        t.pc=advance;t.retired++;s->cpu=t;
        if(t.pc==AE3_NEXT_INIT_MMIO_PC){
            uint32_t x=0;
            if(ae3_ee_bus_read32(bus,t.pc,&x)!=AE3_EE_BUS_OK ||
               x!=UINT32_C(0x8c430000) ||
               (uint32_t)r(&t,2)!=AE3_NEXT_INIT_MMIO_ADDRESS)
                return AE3_NEXT_UNSUPPORTED;
            s->waiting_mmio=1;return AE3_NEXT_MMIO_PENDING;
        }
    }
    return AE3_NEXT_YIELDED;
}