#include "ae3_boot_memory_fill.h"

static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint32_t phys(uint32_t address) {
    return address & UINT32_C(0x1fffffff);
}
static int in_ram(uint32_t address, size_t len, size_t n) {
    size_t off = (size_t)phys(address);
    return off <= n && len <= n-off;
}
static int in_fill(uint32_t address, size_t len, size_t n) {
    if (address < AE3_FILL_BASE) return 0;
    size_t off = (size_t)(address - AE3_FILL_BASE);
    return off <= AE3_FILL_BYTES && len <= AE3_FILL_BYTES-off &&
           in_ram(address,len,n);
}
static uint64_t sext32(uint32_t word) {
    return (uint64_t)(int64_t)(int32_t)word;
}
static uint64_t scalar(const AE3_BootPrefix *s, unsigned r) {
    return r ? s->gpr[r][0] : 0;
}
static void set_scalar(AE3_BootPrefix *s, unsigned r, uint64_t value) {
    if (r) s->gpr[r][0] = value;
}

AE3_FillResult ae3_boot_fill_enter(AE3_BootMemoryFill *stage,
                                       const AE3_BootPrefix *prefix) {
    if (!stage || !prefix || prefix->pc != AE3_BOOT_FIRST_CALL ||
        !prefix->done || prefix->in_delay_slot) return AE3_FILL_BAD_STATE;
    *stage = (AE3_BootMemoryFill){0};
    stage->cpu = *prefix;
    stage->cpu.done = 0;
    stage->cpu.pc = AE3_BOOT_CALLEE;
    stage->cpu.gpr[31][0] = AE3_BOOT_RETURN;
    stage->cpu.gpr[31][1] = 0;
    stage->cpu.retired += 2;  /* original JAL and NOP delay instruction */
    return AE3_FILL_YIELDED;
}
AE3_FillResult ae3_boot_fill_slice(AE3_BootMemoryFill *stage,
       uint8_t *ram, size_t ram_size, uint32_t budget) {
    if (!stage || !ram) return AE3_FILL_BAD_STATE;
    AE3_BootPrefix *s = &stage->cpu;
    if (!s->pc || (s->pc & 3) || s->in_delay_slot > 1 ||
        stage->finished > 1 || s->done) return AE3_FILL_BAD_STATE;
    if (stage->finished) return s->pc == AE3_BOOT_RETURN && !s->in_delay_slot
        ? AE3_FILL_RETURNED : AE3_FILL_BAD_STATE;
    if (!budget) return AE3_FILL_YIELDED;
    for (uint32_t t = 0; t < budget; t++) {
        if (s->pc == AE3_BOOT_RETURN && !s->in_delay_slot) {
            stage->finished = 1;
            return AE3_FILL_RETURNED;
        }
        if (s->pc < AE3_BOOT_CALLEE || s->pc > UINT32_C(0x0010372C) ||
            !in_ram(s->pc,4,ram_size)) return AE3_FILL_MEMORY_FAULT;
        uint32_t pc=s->pc, w=le32(ram+phys(pc));
        unsigned op=w>>26, rs=(w>>21)&31, rt=(w>>16)&31;
        unsigned rd=(w>>11)&31, sh=(w>>6)&31, fn=w&63;
        int32_t simm=(int16_t)(w&0xffff);
        uint32_t next=pc+4, target=0, write_addr=0;
        int is_branch=0, is_store=0;
        uint8_t data[16]={0};
        AE3_BootPrefix candidate=*s;
        if (w==0) { /* nop */ }
        else if (op==15 && rs==0) {
            set_scalar(&candidate,rt,sext32((uint32_t)(w&0xffff)<<16));
        } else if (op==9) {
            set_scalar(&candidate,rt,sext32((uint32_t)scalar(s,rs)+(uint32_t)simm));
        } else if (op==10) {
            set_scalar(&candidate,rt,(int64_t)scalar(s,rs)<(int64_t)simm);
        } else if (op==0 && fn==10 && sh==0) { /* MOVZ */
            if (scalar(s,rt)==0) set_scalar(&candidate,rd,scalar(s,rs));
        } else if (op==0 && fn==3 && rs==0) { /* SRA */
            set_scalar(&candidate,rd,sext32((uint32_t)((int32_t)(uint32_t)scalar(s,rt)>>sh)));
        } else if (op==0 && fn==0x2d && sh==0) { /* DADDU */
            set_scalar(&candidate,rd,scalar(s,rs)+scalar(s,rt));
        } else if (op==0x1e) { /* LQ, full 128 bits */
            uint32_t a=(uint32_t)scalar(s,rs)+(uint32_t)simm;
            if ((a&15) || !in_ram(a,16,ram_size))return AE3_FILL_MEMORY_FAULT;
            if (rt) {
                uint64_t lo=0,hi=0;
                for(unsigned b=0;b<8;b++)lo|=(uint64_t)ram[phys(a)+b]<<(b*8);
                for(unsigned b=0;b<8;b++)hi|=(uint64_t)ram[phys(a)+8+b]<<(b*8);
                candidate.gpr[rt][0]=lo;candidate.gpr[rt][1]=hi;
            }
        } else if (op==0x1f) { /* SQ, external bus capture */
            write_addr=(uint32_t)scalar(s,rs)+(uint32_t)simm;
            if ((write_addr&15) || !in_fill(write_addr,16,ram_size))
                return AE3_FILL_MEMORY_FAULT;
            for(unsigned b=0;b<16;b++)data[b]=(uint8_t)(s->gpr[rt][b/8]>>(8*(b%8)));
            is_store=1;
        } else if (op==6 && rt==0 && !s->in_delay_slot) { /* BLEZ */
            target=((int64_t)scalar(s,rs)<=0)
                ? pc+4+(uint32_t)(simm*4) : pc+8;
            is_branch=1;
        } else if (op==5 && !s->in_delay_slot) { /* BNE */
            target=(scalar(s,rs)!=scalar(s,rt))
                ? pc+4+(uint32_t)(simm*4) : pc+8;
            is_branch=1;
        } else if (op==0 && fn==8 && !s->in_delay_slot && !rt && !rd && !sh) {
            target=(uint32_t)scalar(s,rs);is_branch=1;
        } else return AE3_FILL_UNSUPPORTED;
        if (s->in_delay_slot) {
            next=s->branch_target;
            candidate.in_delay_slot=0;
            candidate.branch_target=0;
        }
        if (is_branch) {
            candidate.in_delay_slot=1;
            candidate.branch_target=target;
        }
        candidate.pc=next;
        candidate.retired++;
        candidate.gpr[0][0]=candidate.gpr[0][1]=0;
        if (is_store) {
            for(unsigned b=0;b<16;b++)ram[phys(write_addr)+b]=data[b];
            stage->fill_stores++;
        }
        *s=candidate;
        if (s->pc == AE3_BOOT_RETURN && !s->in_delay_slot) {
            stage->finished=1;
            return AE3_FILL_RETURNED;
        }
    }
    return AE3_FILL_YIELDED;
}