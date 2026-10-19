#include "ae3_elf_image.h"

static uint16_t le16(const uint8_t *p) {return (uint16_t)(p[0] | ((uint16_t)p[1]<<8));}
static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) |
           ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static int within(size_t start, size_t len, size_t size) {
    return start<=size && len<=size-start;
}
static int intersects(size_t a, size_t a_size, size_t b, size_t b_size) {
    return a < b + b_size && b < a + a_size;
}
int ae3_load_ee_elf32(const uint8_t *elf, size_t elf_size,
                     uint8_t *ram, size_t ram_size, AE3_ElfImage *report) {
    if (!elf || !ram || !report) return AE3_ELF_IMAGE_INVALID;
    *report = (AE3_ElfImage){0};
    if (elf_size<52 || elf[0]!=0x7f || elf[1]!='E' || elf[2]!='L' || elf[3]!='F')
        return AE3_ELF_IMAGE_INVALID;
    if (elf[4]!=1 || elf[5]!=1 || elf[6]!=1 ||
        le16(elf+16)!=2 || le16(elf+18)!=8 || le32(elf+20)!=1)
        return AE3_ELF_IMAGE_UNSUPPORTED;
    uint32_t entry = le32(elf+24);
    uint32_t phoff = le32(elf+28);
    uint16_t phent = le16(elf+42), phnum = le16(elf+44);
    if (!phnum || phent<32 || !within(phoff,(size_t)phnum*phent,elf_size))
        return AE3_ELF_IMAGE_INVALID;
    AE3_ElfImage state={entry,0,0,0};
    int entry_found=0;
    for (size_t i=0; i<phnum; i++) {
        const uint8_t *ph = elf+phoff+i*phent;
        if (le32(ph)!=1) continue;
        size_t offset=le32(ph+4), virtual_address=le32(ph+8);
        size_t physical_address=le32(ph+12), filesz=le32(ph+16), memsz=le32(ph+20);
        if (virtual_address!=physical_address) return AE3_ELF_IMAGE_UNSUPPORTED;
        if (filesz>memsz || !within(offset,filesz,elf_size))return AE3_ELF_IMAGE_INVALID;
        if (!within(physical_address,memsz,ram_size))return AE3_ELF_IMAGE_OUT_OF_BOUNDS;
        if (entry>=virtual_address && (size_t)entry-virtual_address<filesz)entry_found=1;
        /* No alias/overlap ambiguity, even when segments are out of order. */
        for (size_t j=0; j<i; j++) {
            const uint8_t *prev=elf+phoff+j*phent;
            if (le32(prev)!=1) continue;
            if(intersects(physical_address,memsz,le32(prev+12),le32(prev+20)))
                return AE3_ELF_IMAGE_UNSUPPORTED;
        }
        state.segment_count++;
        state.copied_bytes+=filesz;
        state.zeroed_bss_bytes+=memsz-filesz;
    }
    if (!state.segment_count || !entry_found)return AE3_ELF_IMAGE_UNSUPPORTED;
    /* Only after all segments pass preflight do we alter caller-owned RAM. */
    for (size_t i=0;i<phnum;i++) {
        const uint8_t *ph=elf+phoff+i*phent;
        if(le32(ph)!=1)continue;
        size_t offset=le32(ph+4),dst=le32(ph+12),len=le32(ph+16),mem=le32(ph+20);
        for (size_t k=0;k<len;k++)ram[dst+k]=elf[offset+k];
        for (size_t k=len;k<mem;k++)ram[dst+k]=0;
    }
    *report=state;
    return AE3_ELF_IMAGE_OK;
}