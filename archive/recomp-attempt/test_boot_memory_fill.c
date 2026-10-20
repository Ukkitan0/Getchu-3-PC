#include "ae3_elf_image.h"
#include "ae3_boot_prefix.h"
#include "ae3_boot_memory_fill.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_SIZE (32u*1024u*1024u)
static uint8_t *read_elf(const char *path,size_t *n) {
    FILE *f=fopen(path,"rb");assert(f);
    assert(!fseek(f,0,SEEK_END));long sz=ftell(f);assert(sz>0);
    rewind(f);uint8_t *p=malloc((size_t)sz);assert(p);
    assert(fread(p,1,(size_t)sz,f)==(size_t)sz);fclose(f);*n=(size_t)sz;return p;
}
static void run_prefix(AE3_BootPrefix *s,uint8_t *ram) {
    ae3_boot_prefix_init(s);
    AE3_BootResult r=ae3_boot_prefix_slice(s,ram,RAM_SIZE,350000);
    assert(r==AE3_BOOT_FIRST_CALL_REACHED);
}
int main(int argc,char **argv) {
    assert(argc==2);
    size_t n;uint8_t *elf=read_elf(argv[1],&n);
    uint8_t *ram=calloc(RAM_SIZE,1);assert(ram);
    AE3_ElfImage img;assert(ae3_load_ee_elf32(elf,n,ram,RAM_SIZE,&img)==AE3_ELF_IMAGE_OK);
    AE3_BootPrefix prefix;run_prefix(&prefix,ram);
    uint8_t *ram2=malloc(RAM_SIZE),*ram3=malloc(RAM_SIZE);assert(ram2 && ram3);
    memcpy(ram2,ram,RAM_SIZE);memcpy(ram3,ram,RAM_SIZE);
    AE3_BootMemoryFill one,sliced,retry;
    assert(ae3_boot_fill_enter(&one,&prefix)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_enter(&sliced,&prefix)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_enter(&retry,&prefix)==AE3_FILL_YIELDED);
    assert(one.cpu.pc==AE3_BOOT_CALLEE && one.cpu.gpr[31][0]==AE3_BOOT_RETURN);
    assert(ae3_boot_fill_slice(&one,ram,RAM_SIZE,200000)==AE3_FILL_RETURNED);
    assert(one.finished && one.cpu.pc==AE3_BOOT_RETURN && !one.cpu.in_delay_slot);
    assert(one.fill_stores==AE3_FILL_BYTES/16);
    uint8_t original[16];memcpy(original,ram+0x6b8120,16);
    for(size_t i=0;i<AE3_FILL_BYTES;i++)assert(ram[AE3_FILL_BASE+i]==original[i&15]);
    unsigned calls=0,pending=0;
    for(;;) {
        AE3_FillResult r=ae3_boot_fill_slice(&sliced,ram2,RAM_SIZE,(calls*7)%23+1);
        pending|=sliced.cpu.in_delay_slot;
        calls++;assert(calls<200000);
        if(r==AE3_FILL_RETURNED)break;
        assert(r==AE3_FILL_YIELDED);
    }
    assert(pending);
    assert(!memcmp(&one,&sliced,sizeof(one)) && !memcmp(ram,ram2,RAM_SIZE));
    while(retry.cpu.pc!=UINT32_C(0x0010370C))
        assert(ae3_boot_fill_slice(&retry,ram3,RAM_SIZE,1)==AE3_FILL_YIELDED);
    AE3_BootMemoryFill before=retry;
    uint8_t preserved=ram3[AE3_FILL_BASE];
    assert(ae3_boot_fill_slice(&retry,ram3,AE3_FILL_BASE,1)==AE3_FILL_MEMORY_FAULT);
    assert(!memcmp(&retry,&before,sizeof retry) && ram3[AE3_FILL_BASE]==preserved);
    assert(ae3_boot_fill_slice(&retry,ram3,RAM_SIZE,200000)==AE3_FILL_RETURNED);
    assert(!memcmp(&retry,&one,sizeof one) && !memcmp(ram,ram3,RAM_SIZE));
    printf("PASS boot RAM fill: %u 128-bit stores (%u bytes), %llu total retired instructions, %u slices; return %08X\n",
         one.fill_stores,AE3_FILL_BYTES,(unsigned long long)one.cpu.retired,calls,one.cpu.pc);
    puts("PASS exact retail data pattern, delayed branches, atomic memory faults, resumable state");
    free(elf);free(ram);free(ram2);free(ram3);return 0;
}