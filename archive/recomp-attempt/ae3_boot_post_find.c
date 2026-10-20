#include "ae3_boot_post_find.h"

static uint64_t r(const AE3_BootPrefix *s,unsigned i){return i?s->gpr[i][0]:0;}
static void wr(AE3_BootPrefix *s,unsigned i,uint64_t value){if(i)s->gpr[i][0]=value;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static int permitted(uint32_t pc){
    if(pc==AE3_POST_FIND_NEXT_CALL)return 1;
    if(pc>=AE3_POST_FIND_ENTRY && pc<=UINT32_C(0x0043FE40) && !(pc&3))return 1;
    if(pc>=UINT32_C(0x0043FD00) && pc<=UINT32_C(0x0043FD0C) && !(pc&3))return 1;
    return 0;
}
static int return_site(uint32_t pc){
    return pc==UINT32_C(0x0043FDE4) || pc==UINT32_C(0x0043FE00);
}
AE3_PostFindResult ae3_boot_post_find_enter(AE3_BootPostFind *out,
    const AE3_BootHookInit *hook){
    if(!out||!hook||hook->guest_completed!=2 || hook->guest_pending ||
       hook->cpu.pc!=AE3_POST_FIND_ENTRY || hook->cpu.in_delay_slot)
       return AE3_POST_INVALID;
    *out=(AE3_BootPostFind){0};out->cpu=hook->cpu;
    return AE3_POST_YIELDED;
}
AE3_PostFindResult ae3_boot_post_find_slice(AE3_BootPostFind *s,
    AE3_EEMemoryBus *bus,uint32_t budget){
    if(!s||!bus||!bus->ram||bus->ram_size<AE3_EE_RAM_BYTES||
       !permitted(s->cpu.pc)||s->cpu.in_delay_slot>1||s->done>1)
       return AE3_POST_INVALID;
    if(s->cpu.pc==AE3_POST_FIND_NEXT_CALL){s->done=1;return AE3_POST_NEXT_INIT;}
    if(s->cpu.pc==AE3_POST_FIND_SYSCALL){
        if(s->cpu.in_delay_slot||r(&s->cpu,3)!=UINT64_C(0x83)||
           !return_site((uint32_t)r(&s->cpu,31)))return AE3_POST_UNSUPPORTED;
        return AE3_POST_FIND_REQUEST;
    }
    for(uint32_t n=0;n<budget;n++){
        uint32_t pc=s->cpu.pc,w=0;
        if(ae3_ee_bus_read32(bus,pc,&w)!=AE3_EE_BUS_OK)return AE3_POST_MEMORY_FAULT;
        AE3_BootPrefix next=s->cpu;
        uint32_t advance=pc+4;
        if(s->cpu.in_delay_slot){
            advance=s->cpu.branch_target;
            next.in_delay_slot=0;next.branch_target=0;
        }
        unsigned op=w>>26,rs=(w>>21)&31,rt=(w>>16)&31,rd=(w>>11)&31;
        unsigned sh=(w>>6)&31,fn=w&63;
        int32_t imm=(int16_t)(w&0xffffu);
        uint32_t addr=0,lo=0;uint8_t stored[4];int do_store=0;
        if(w==0){}
        else if(op==9)wr(&next,rt,sx((uint32_t)r(&s->cpu,rs)+(uint32_t)imm));
        else if(op==15&&rs==0)wr(&next,rt,sx((w&65535u)<<16));
        else if(op==0&&fn==0x2d&&!sh)
            wr(&next,rd,r(&s->cpu,rs)+r(&s->cpu,rt));
        else if(op==0&&fn==0x2b&&!sh)
            wr(&next,rd,r(&s->cpu,rs)<r(&s->cpu,rt));
        else if((op==4||op==5)&&!s->cpu.in_delay_slot){
            int taken=op==4?r(&s->cpu,rs)==r(&s->cpu,rt):
                            r(&s->cpu,rs)!=r(&s->cpu,rt);
            next.in_delay_slot=1;
            next.branch_target=taken?pc+4+(uint32_t)(imm*4):pc+8;
        }
        else if(op==3&&!s->cpu.in_delay_slot){
            uint32_t target=((pc+4)&UINT32_C(0xf0000000))|((w&UINT32_C(0x03ffffff))<<2);
            if(target!=UINT32_C(0x0043FD00))return AE3_POST_UNSUPPORTED;
            wr(&next,31,pc+8);next.in_delay_slot=1;next.branch_target=target;
        }
        else if(op==0&&fn==8&&!sh&&!rt&&!rd&&!s->cpu.in_delay_slot){
            next.in_delay_slot=1;next.branch_target=(uint32_t)r(&s->cpu,rs);
        }
        else if(op==35||op==55){
            uint32_t a=(uint32_t)r(&s->cpu,rs)+(uint32_t)imm;
            uint8_t bytes[8]={0};size_t count=op==35?4:8;
            if((a&(count-1))||ae3_ee_bus_read(bus,a,bytes,count)!=AE3_EE_BUS_OK)
                return AE3_POST_MEMORY_FAULT;
            uint64_t v=0;for(size_t i=0;i<count;i++)v|=(uint64_t)bytes[i]<<(8*i);
            wr(&next,rt,op==35?sx((uint32_t)v):v);
        }
        else if(op==43){
            addr=(uint32_t)r(&s->cpu,rs)+(uint32_t)imm;
            if((addr&3)||ae3_ee_bus_read32(bus,addr,&lo)!=AE3_EE_BUS_OK)
                return AE3_POST_MEMORY_FAULT;
            uint32_t v=(uint32_t)r(&s->cpu,rt);
            for(unsigned i=0;i<4;i++)stored[i]=(uint8_t)(v>>(i*8));
            do_store=1;
        }
        else return AE3_POST_UNSUPPORTED;
        /* Reject malformed call/branch targets before committing any mutation. */
        if(!permitted(advance)||
           (next.in_delay_slot&&!permitted(next.branch_target))){
            return AE3_POST_UNSUPPORTED;
        }
        if(do_store&&ae3_ee_bus_write(bus,addr,stored,4)!=AE3_EE_BUS_OK)
            return AE3_POST_MEMORY_FAULT;
        next.pc=advance;next.retired++;
        s->cpu=next;
        if(next.pc==AE3_POST_FIND_NEXT_CALL){s->done=1;return AE3_POST_NEXT_INIT;}
        if(next.pc==AE3_POST_FIND_SYSCALL){
            if(next.in_delay_slot||r(&next,3)!=0x83 ||
               !return_site((uint32_t)r(&next,31)))return AE3_POST_UNSUPPORTED;
            s->search_requests++;
            return AE3_POST_FIND_REQUEST;
        }
    }
    return AE3_POST_YIELDED;
}
AE3_PostFindResult ae3_boot_post_find_begin_guest(
    const AE3_BootPostFind *s,AE3_GuestFindAddress *guest){
    if(!s||!guest||s->cpu.pc!=AE3_POST_FIND_SYSCALL||s->cpu.in_delay_slot||
       !return_site((uint32_t)r(&s->cpu,31)))return AE3_POST_INVALID;
    AE3_BootHookInit hook={0};
    hook.cpu=s->cpu;hook.guest_pending=1;hook.registered=2;
    hook.invoked_handler_pc=AE3_FIND_ENTRY;
    return ae3_guest_find_begin(guest,&hook)==AE3_FIND_YIELDED?
        AE3_POST_YIELDED:AE3_POST_INVALID;
}
AE3_PostFindResult ae3_boot_post_find_finish_guest(
    AE3_BootPostFind *s,const AE3_GuestFindAddress *guest){
    if(!s||!guest||!guest->done||s->cpu.pc!=AE3_POST_FIND_SYSCALL||
       !return_site((uint32_t)r(&s->cpu,31))||
       guest->return_pc!=(uint32_t)r(&s->cpu,31)||
       guest->matched_guest_address!=(uint32_t)r(&guest->cpu,2))return AE3_POST_INVALID;
    AE3_BootPrefix next=guest->cpu;
    next.pc=AE3_POST_FIND_SYSCALL+4;
    next.retired++;next.in_delay_slot=0;next.branch_target=0;next.done=0;
    s->cpu=next;return AE3_POST_YIELDED;
}