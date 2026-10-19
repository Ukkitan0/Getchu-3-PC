#include "ae3_boot_semaphore_init.h"

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 |
           (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static uint64_t rd64(const uint8_t *p) {
    return (uint64_t)rd32(p)|((uint64_t)rd32(p+4)<<32);
}
static void wr32(uint8_t *p,uint32_t v) {
    for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8*i));
}
static void wr64(uint8_t *p,uint64_t v) {
    for(unsigned i=0;i<8;i++)p[i]=(uint8_t)(v>>(8*i));
}
static uint64_t ext32(uint32_t v) {return (uint64_t)(int64_t)(int32_t)v;}
static uint64_t reg(const AE3_BootPrefix *s,unsigned idx) {
    return idx?s->gpr[idx][0]:0;
}
static void set(AE3_BootPrefix *s,unsigned idx,uint64_t v) {
    if(idx)s->gpr[idx][0]=v;
}
static int range(size_t address, size_t size, size_t bytes) {
    return address<=bytes && size<=bytes-address;
}
static int allowed(uint32_t pc) {
    return pc==AE3_SEMA_INIT_CALL_PC || pc==AE3_SEMA_INIT_CALL_PC+4 ||
        (pc>=AE3_SEMA_ROUTINE_PC && pc<=UINT32_C(0x0043FC64) && !(pc&3)) ||
        (pc>=AE3_SEMA_SYSCALL_WRAPPER && pc<=AE3_SEMA_SYSCALL_WRAPPER+12 && !(pc&3)) ||
        pc==AE3_SEMA_NEXT_CALL_PC;
}
static int valid(const AE3_BootSemaphoreInit *s) {
    return s && allowed(s->cpu.pc) && s->cpu.in_delay_slot<=1 &&
        s->completed<=2 && s->pending<=1 && s->done<=1 &&
        (!s->done || (s->cpu.pc==AE3_SEMA_NEXT_CALL_PC && !s->pending));
}
AE3_SemaBootResult ae3_boot_sema_enter(AE3_BootSemaphoreInit *out,
                                       const AE3_BootInitPrefix *init) {
    if(!out||!init||!init->done||init->cpu.in_delay_slot||
       init->cpu.pc!=AE3_SEMA_INIT_CALL_PC) return AE3_SEMA_INVALID;
    *out=(AE3_BootSemaphoreInit){0};
    out->cpu=init->cpu;
    return AE3_SEMA_YIELDED;
}
AE3_SemaBootResult ae3_boot_sema_slice(AE3_BootSemaphoreInit *s,
                                       uint8_t *ram, size_t ram_bytes,
                                       uint32_t budget) {
    if(!valid(s)||!ram)return AE3_SEMA_INVALID;
    if(s->pending)return AE3_SEMA_CREATE_REQUEST;
    if(s->done)return AE3_SEMA_NEXT_CALL;
    if(!budget)return AE3_SEMA_YIELDED;
    for(uint32_t i=0;i<budget;i++) {
        const uint32_t pc=s->cpu.pc;
        if(pc==AE3_SEMA_NEXT_CALL_PC){s->done=1;return AE3_SEMA_NEXT_CALL;}
        if(!range(pc,4,ram_bytes))return AE3_SEMA_MEMORY_FAULT;
        const uint32_t w=rd32(ram+pc);
        const unsigned op=w>>26, rs=(w>>21)&31u,rt=(w>>16)&31u,
                       rd=(w>>11)&31u,sh=(w>>6)&31u,fn=w&63u;
        const int16_t imm=(int16_t)(w&0xffffu);
        const uint32_t oldpc=s->cpu.pc;
        AE3_BootPrefix tmp=s->cpu;
        uint32_t next=oldpc+4;
        size_t store_address=0,store_size=0;
        uint64_t store_value=0;
        if(w==UINT32_C(0x0000000c)) {
            if(pc!=AE3_SEMA_SYSCALL_PC || tmp.in_delay_slot ||
               (uint32_t)reg(&tmp,3)!=0x40 || s->completed>=2)
                return AE3_SEMA_UNSUPPORTED;
            const size_t pointer=(size_t)(uint32_t)reg(&tmp,4);
            if((pointer&3u)||!range(pointer,24,ram_bytes))return AE3_SEMA_MEMORY_FAULT;
            AE3_GuestSemaRequest req={0};
            req.syscall_number=0x40;req.pc=pc;req.guest_address=(uint32_t)pointer;
            for(unsigned j=0;j<6;j++)req.values[j]=rd32(ram+pointer+4*j);
            s->request=req;
            s->pending=1;
            return AE3_SEMA_CREATE_REQUEST;
        }
        if (tmp.in_delay_slot) {
            if(op==3 || op==2 || (op==0 && (fn==8||fn==9)))
                return AE3_SEMA_UNSUPPORTED;
            next=tmp.branch_target;
            tmp.in_delay_slot=0;
            tmp.branch_target=0;
        }
        if(w==0) { /* NOP */ }
        else if(op==15 && rs==0) { /* LUI */
            set(&tmp,rt,ext32((uint32_t)(w&0xffffu)<<16));
        } else if(op==9) { /* ADDIU sign-extends the 32-bit result */
            set(&tmp,rt,ext32((uint32_t)reg(&s->cpu,rs)+(uint32_t)(int32_t)imm));
        } else if(op==0 && fn==0x2d && sh==0) { /* DADDU */
            set(&tmp,rd,reg(&s->cpu,rs)+reg(&s->cpu,rt));
        } else if(op==3 && !s->cpu.in_delay_slot) { /* JAL */
            set(&tmp,31,pc+8);
            tmp.in_delay_slot=1;
            tmp.branch_target=((pc+4)&UINT32_C(0xf0000000))|((w&UINT32_C(0x03ffffff))<<2);
        } else if(op==0 && fn==8 && sh==0 && rt==0 && rd==0 && !s->cpu.in_delay_slot) { /* JR */
            tmp.in_delay_slot=1;
            tmp.branch_target=(uint32_t)reg(&s->cpu,rs);
        } else if((op==43||op==63) && !s->cpu.in_delay_slot) {
            /* SW/SD; code happens to use SW in the JAL delay slot too. */
            store_address=(size_t)((uint32_t)reg(&s->cpu,rs)+(uint32_t)(int32_t)imm);
            store_size=op==43?4:8;
            store_value=reg(&s->cpu,rt);
            if((store_address&(store_size-1))||!range(store_address,store_size,ram_bytes))
                return AE3_SEMA_MEMORY_FAULT;
        } else if((op==43||op==63) && s->cpu.in_delay_slot) {
            store_address=(size_t)((uint32_t)reg(&s->cpu,rs)+(uint32_t)(int32_t)imm);
            store_size=op==43?4:8;store_value=reg(&s->cpu,rt);
            if((store_address&(store_size-1))||!range(store_address,store_size,ram_bytes))return AE3_SEMA_MEMORY_FAULT;
        } else if(op==55) { /* LD */
            const size_t addr=(size_t)((uint32_t)reg(&s->cpu,rs)+(uint32_t)(int32_t)imm);
            if((addr&7)||!range(addr,8,ram_bytes))return AE3_SEMA_MEMORY_FAULT;
            set(&tmp,rt,rd64(ram+addr));
        } else if(op==35) { /* LW sign-extended, used by later init stages */
            const size_t addr=(size_t)((uint32_t)reg(&s->cpu,rs)+(uint32_t)(int32_t)imm);
            if((addr&3)||!range(addr,4,ram_bytes))return AE3_SEMA_MEMORY_FAULT;
            set(&tmp,rt,ext32(rd32(ram+addr)));
        } else return AE3_SEMA_UNSUPPORTED;
        if(!allowed(next))return AE3_SEMA_UNSUPPORTED;
        if(store_size==4)wr32(ram+store_address,(uint32_t)store_value);
        if(store_size==8)wr64(ram+store_address,store_value);
        tmp.pc=next;
        tmp.retired++;
        s->cpu=tmp;
        if(next==AE3_SEMA_NEXT_CALL_PC) {s->done=1;return AE3_SEMA_NEXT_CALL;}
    }
    return AE3_SEMA_YIELDED;
}
AE3_SemaBootResult ae3_boot_sema_kernel_return(AE3_BootSemaphoreInit *s,int32_t result) {
    if(!valid(s)||!s->pending||s->cpu.pc!=AE3_SEMA_SYSCALL_PC||
       s->request.pc!=AE3_SEMA_SYSCALL_PC || s->request.syscall_number!=0x40)
        return AE3_SEMA_INVALID;
    set(&s->cpu,2,ext32((uint32_t)result));
    s->cpu.pc+=4;
    s->cpu.retired++;
    s->pending=0;
    s->completed++;
    return AE3_SEMA_YIELDED;
}