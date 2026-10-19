#include "ae3_guest_find_address.h"
static uint32_t u32(const uint8_t *p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static uint64_t reg(const AE3_BootPrefix *s, unsigned i) {
    return i?s->gpr[i][0]:0;
}
static void set(AE3_BootPrefix *s,unsigned i,uint64_t x) {
    if(i)s->gpr[i][0]=x;
}
static uint64_t sext(uint32_t x) {return (uint64_t)(int64_t)(int32_t)x;}
static int mapped(uint32_t address,size_t length,uint32_t base,size_t count) {
    /* Avoid host arithmetic overflow and never alias an unmapped EE kernel page. */
    if(address<base)return 0;
    uint64_t offset=(uint64_t)address-base;
    return offset<=count && length<=count-(size_t)offset;
}
AE3_FindResult ae3_guest_find_begin(AE3_GuestFindAddress *out,
    const AE3_BootHookInit *hook) {
    if(!out||!hook||!hook->guest_pending ||
       hook->cpu.pc!=UINT32_C(0x0043FD04) ||
       hook->invoked_handler_pc!=AE3_FIND_ENTRY ||
       hook->cpu.in_delay_slot || hook->registered!=2 ||
       ((uint32_t)reg(&hook->cpu,31)!=UINT32_C(0x0043FDA4) &&
        (uint32_t)reg(&hook->cpu,31)!=UINT32_C(0x0043FDB8)))
       return AE3_FIND_INVALID;
    *out=(AE3_GuestFindAddress){0};
    out->cpu=hook->cpu;
    out->cpu.pc=AE3_FIND_ENTRY;
    out->return_pc=(uint32_t)reg(&hook->cpu,31);
    return AE3_FIND_YIELDED;
}
AE3_FindResult ae3_guest_find_slice(AE3_GuestFindAddress *s,
    const uint8_t *ee,size_t ee_size,const uint8_t *kernel,
    uint32_t base,size_t count,uint32_t budget) {
    if(!s||!ee || s->cpu.in_delay_slot>1 || s->done>1 ||
       (s->return_pc&3)||
       (s->return_pc!=UINT32_C(0x0043FDA4) &&
        s->return_pc!=UINT32_C(0x0043FDB8)) ||
       (!s->done && (s->cpu.pc<AE3_FIND_ENTRY || s->cpu.pc>=AE3_FIND_END || (s->cpu.pc&3))))
       return AE3_FIND_INVALID;
    if(s->done)return AE3_FIND_RETURNED;
    for(uint32_t n=0;n<budget;n++) {
        const uint32_t pc=s->cpu.pc;
        if((size_t)pc>ee_size || ee_size-(size_t)pc<4)return AE3_FIND_UNSUPPORTED;
        const uint32_t w=u32(ee+pc);
        const unsigned op=w>>26,rs=(w>>21)&31u,rt=(w>>16)&31u,
             rd=(w>>11)&31u,sh=(w>>6)&31u,fn=w&63u;
        const int32_t imm=(int16_t)(w&65535u);
        AE3_BootPrefix next=s->cpu;
        uint32_t advance=pc+4u;
        uint32_t load_count=0;
        if(next.in_delay_slot){
            advance=next.branch_target;
            next.in_delay_slot=0;
            next.branch_target=0;
            /* A branch in a delay slot is explicitly unsupported. */
            if(op==4||op==5||op==0x14||op==0x15||
               (op==0&&(fn==8||fn==9)))return AE3_FIND_UNSUPPORTED;
        }
        if(w==0){}
        else if(op==35){
            uint32_t addr=(uint32_t)reg(&s->cpu,rs)+(uint32_t)imm;
            if((addr&3u)||!kernel||!mapped(addr,4,base,count))
                return AE3_FIND_KERNEL_UNMAPPED;
            set(&next,rt,sext(u32(kernel+(size_t)(addr-base))));
            load_count=1;
        }
        else if(op==9){
            uint32_t v=(uint32_t)reg(&s->cpu,rs)+(uint32_t)imm;
            set(&next,rt,sext(v));
        }
        else if(op==0&&fn==0x2b&&sh==0){
            set(&next,rd,reg(&s->cpu,rs)<reg(&s->cpu,rt));
        }
        else if(op==0&&fn==0x0a&&sh==0){
            if(reg(&s->cpu,rt)==0)set(&next,rd,reg(&s->cpu,rs));
        }
        else if(op==0&&fn==0x2d&&sh==0){
            set(&next,rd,reg(&s->cpu,rs)+reg(&s->cpu,rt));
        }
        else if((op==4||op==5||op==0x14||op==0x15) && !s->cpu.in_delay_slot){
            int taken=(op==4||op==0x14) ?
                reg(&s->cpu,rs)==reg(&s->cpu,rt):
                reg(&s->cpu,rs)!=reg(&s->cpu,rt);
            if((op==0x14||op==0x15)&&!taken)advance=pc+8;
            else {
                next.in_delay_slot=1;
                next.branch_target=taken?pc+4+(uint32_t)(imm*4):pc+8;
            }
        }
        else if(op==0&&fn==8&&rd==0&&rt==0&&sh==0&&!s->cpu.in_delay_slot){
            next.in_delay_slot=1;
            next.branch_target=(uint32_t)reg(&s->cpu,rs);
        }
        else return AE3_FIND_UNSUPPORTED;
        if(advance!=s->return_pc &&
           (advance<AE3_FIND_ENTRY || advance>=AE3_FIND_END || (advance&3)))
            return AE3_FIND_UNSUPPORTED;
        next.pc=advance;
        next.retired++;
        s->cpu=next;
        s->loads+=load_count;
        if(advance==s->return_pc){
            s->done=1;s->matched_guest_address=(uint32_t)reg(&s->cpu,2);
            return AE3_FIND_RETURNED;
        }
    }
    return AE3_FIND_YIELDED;
}
AE3_FindResult ae3_guest_find_commit(AE3_BootHookInit *hook,
    const AE3_GuestFindAddress *find) {
    if(!hook||!find||!find->done||!hook->guest_pending||
       hook->cpu.pc!=UINT32_C(0x0043FD04) ||
       find->matched_guest_address!=(uint32_t)reg(&find->cpu,2) ||
       find->return_pc!=(uint32_t)reg(&hook->cpu,31))return AE3_FIND_INVALID;
    /* Restore the architectural state after the SYSCALL, not after a JAL.
       The guest handler's own return is kernel-internal; its register effects
       remain visible to the userland code at 0x43FD08. */
    AE3_BootPrefix t=find->cpu;
    t.pc=hook->cpu.pc+4;
    t.done=0;t.in_delay_slot=0;t.branch_target=0;
    t.retired++;
    hook->cpu=t;
    hook->guest_pending=0;
    hook->guest_completed++;
    return AE3_FIND_RETURNED;
}