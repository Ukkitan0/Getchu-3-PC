#include "ae3_boot_prefix.h"

static uint32_t phys(uint32_t address) { return address & UINT32_C(0x1fffffff); }
static int mapped(uint32_t addr, size_t n, size_t size) {
    size_t offset = phys(addr);
    return offset <= size && n <= size-offset;
}
static uint32_t load32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) |
        ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static uint64_t sx32(uint32_t x) { return (uint64_t)(int64_t)(int32_t)x; }
static uint32_t word128(const AE3_BootPrefix *s, unsigned r, unsigned w) {
    if (!r) return 0;
    return (uint32_t)(s->gpr[r][w/2] >> (32*(w&1)));
}
static void putword128(AE3_BootPrefix *s, unsigned r, unsigned w, uint32_t v) {
    if (!r) return;
    uint64_t mask = UINT64_C(0xffffffff) << (32*(w&1));
    s->gpr[r][w/2] = (s->gpr[r][w/2] & ~mask) | ((uint64_t)v << (32*(w&1)));
}
static void scalar(AE3_BootPrefix *s, unsigned r, uint64_t v) {
    if (r) s->gpr[r][0] = v;
}
static uint64_t reg(const AE3_BootPrefix *s, unsigned r) {
    return r ? s->gpr[r][0] : 0;
}

void ae3_boot_prefix_init(AE3_BootPrefix *s) {
    if (!s) return;
    *s = (AE3_BootPrefix){0};
    s->pc = AE3_BOOT_ENTRY;
}

AE3_BootResult ae3_boot_prefix_slice(AE3_BootPrefix *s,
                                     uint8_t *ram, size_t ram_size,
                                     uint32_t budget) {
    if (!s || !ram || !s->pc || (s->pc & 3) ||
        s->pc < AE3_BOOT_ENTRY || s->pc > AE3_BOOT_FIRST_CALL ||
        s->in_delay_slot > 1 || s->done > 1)
        return AE3_BOOT_INVALID_STATE;
    if (s->done) return s->pc == AE3_BOOT_FIRST_CALL && !s->in_delay_slot
        ? AE3_BOOT_FIRST_CALL_REACHED : AE3_BOOT_INVALID_STATE;
    if (s->pc == AE3_BOOT_FIRST_CALL && !s->in_delay_slot) {
        s->done = 1;
        return AE3_BOOT_FIRST_CALL_REACHED;
    }
    if (!budget) return AE3_BOOT_YIELDED;

    for (uint32_t i=0; i<budget; ++i) {
        const uint32_t pc = s->pc;
        if (pc == AE3_BOOT_FIRST_CALL && !s->in_delay_slot) {
            s->done = 1;
            return AE3_BOOT_FIRST_CALL_REACHED;
        }
        if (pc < AE3_BOOT_ENTRY || pc >= AE3_BOOT_FIRST_CALL || (pc & 3))
            return AE3_BOOT_INVALID_STATE;
        if (!mapped(pc, 4, ram_size)) return AE3_BOOT_MEMORY_FAULT;
        uint32_t w=load32(ram+phys(pc));
        unsigned op=w>>26, rs=(w>>21)&31, rt=(w>>16)&31;
        unsigned rd=(w>>11)&31, sh=(w>>6)&31, fn=w&63;
        int32_t simm=(int16_t)(w&0xffff);
        uint32_t next=pc+4, target=0;
        int new_branch=0, is_byte=0, is_quad=0;
        uint32_t store_addr=0;
        uint8_t store_data[16]={0};
        AE3_BootPrefix candidate=*s; /* transaction per instruction */

        if (w==0) { /* nop */ }
        else if (op==0x1c && fn==0x28 && sh==16) {
            /* PADDUW: four independent unsigned 32-bit lane additions. */
            uint32_t v[4];
            for (unsigned j=0;j<4;j++)v[j]=word128(s,rs,j)+word128(s,rt,j);
            for (unsigned j=0;j<4;j++)putword128(&candidate,rd,j,v[j]);
        }
        else if ((op==0 || op==0x1c) && rs==0 && rt==0 && rd==0 && sh==0 &&
                 (fn==0x11 || fn==0x13)) {
            /* MTHI[1] / MTLO[1], low 64-bit bank selected by MMI opcode. */
            candidate.hilo[op==0x1c][fn==0x11 ? 0 : 1]=0;
        }
        else if (op==1 && rs==0 && rt==0x19 && (w&0xffff)==0) {
            /* BGEZALL $zero,+0: taken, link, executes a delay slot. */
            candidate.gpr[31][0]=pc+8;
            target=pc+4;
            new_branch=1;
        }
        else if (op==0x11 && rs==4 && rt==0 && (w&0x7ff)==0) {
            /* MTC1 $zero,fs */
            candidate.fpr[rd]=0;
        }
        else if (w==UINT32_C(0x46010018)) {
            /* ADDA.S $f0,$f1: ACC <- 0+0 after the FPR reset. */
            if (s->fpr[0] || s->fpr[1])return AE3_BOOT_UNSUPPORTED_INSTRUCTION;
            candidate.fpu_acc=0;
        }
        else if (w==UINT32_C(0x0000040f)) { /* SYNC 0x10: host guest-RAM fence */ }
        else if (w==UINT32_C(0x44c0f800)) { candidate.fcr31=0; }
        else if (op==0xf && rs==0) { scalar(&candidate,rt,sx32((w&0xffff)<<16)); }
        else if (op==9) { scalar(&candidate,rt,sx32((uint32_t)reg(s,rs)+(uint32_t)simm)); }
        else if (op==0xc) { scalar(&candidate,rt,reg(s,rs)&(w&0xffff)); }
        else if (op==0xd) { scalar(&candidate,rt,reg(s,rs)|(w&0xffff)); }
        else if (op==0 && fn==0x24 && sh==0) { scalar(&candidate,rd,reg(s,rs)&reg(s,rt)); }
        else if (op==0x28) {
            store_addr=(uint32_t)reg(s,rs)+(uint32_t)simm;
            store_data[0]=(uint8_t)reg(s,rt);is_byte=1;
        }
        else if (op==0x1f) {
            store_addr=(uint32_t)reg(s,rs)+(uint32_t)simm;
            if ((store_addr&15)!=0)return AE3_BOOT_MEMORY_FAULT;
            for(unsigned j=0;j<16;j++)store_data[j]=(uint8_t)(s->gpr[rt][j/8]>>(8*(j%8)));
            is_quad=1;
        }
        else if ((op==4 || op==5) && !s->in_delay_slot) {
            int equals=reg(s,rs)==reg(s,rt);
            target=(uint32_t)(pc+4+(uint32_t)(simm*4));
            if ((op==4 && !equals)||(op==5 && equals))target=pc+8;
            new_branch=1;
        }
        else if (op==2 && !s->in_delay_slot) {
            target=((pc+4)&UINT32_C(0xf0000000)) | ((w&0x3ffffff)<<2);
            new_branch=1;
        }
        else return AE3_BOOT_UNSUPPORTED_INSTRUCTION;

        if (new_branch && s->in_delay_slot)return AE3_BOOT_UNSUPPORTED_INSTRUCTION;
        if ((is_byte || is_quad) && !mapped(store_addr,is_byte?1:16,ram_size))
            return AE3_BOOT_MEMORY_FAULT;
        if (s->in_delay_slot) {
            next=s->branch_target;
            candidate.in_delay_slot=0;
            candidate.branch_target=0;
        }
        if (new_branch) {
            candidate.in_delay_slot=1;
            candidate.branch_target=target;
        }
        candidate.pc=next;
        candidate.retired++;
        candidate.byte_stores+=(uint32_t)is_byte;
        candidate.quad_stores+=(uint32_t)is_quad;
        if (is_byte || is_quad)
            for (unsigned j=0;j<(is_byte?1u:16u);j++)
                ram[phys(store_addr)+j]=store_data[j];
        candidate.gpr[0][0]=candidate.gpr[0][1]=0;
        *s=candidate;
        if (s->pc == AE3_BOOT_FIRST_CALL && !s->in_delay_slot) {
            s->done=1;
            return AE3_BOOT_FIRST_CALL_REACHED;
        }
    }
    return AE3_BOOT_YIELDED;
}