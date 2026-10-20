#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAM_BYTES (32u*1024u*1024u)
#define FILE_BYTES 6488148u
static uint8_t ram[RAM_BYTES];
static uint8_t pristine[RAM_BYTES];
static void write32(uint8_t *p, uint32_t value) {
    for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(value>>(8*i));
}
int main(int argc, char **argv) {
    if(argc!=2){fprintf(stderr,"usage: %s /path/to/SCUS_975.01\n",argv[0]);return 2;}
    FILE *f=fopen(argv[1],"rb");assert(f);
    assert(fseek(f,0,SEEK_END)==0);long len=ftell(f); assert(len==(long)FILE_BYTES);
    assert(fseek(f,0,SEEK_SET)==0);
    uint8_t *elf=malloc((size_t)len);assert(elf);
    assert(fread(elf,1,(size_t)len,f)==(size_t)len);fclose(f);
    memset(ram,0xa5,sizeof(ram));
    AE3_ElfImage info={0};
    assert(ae3_load_ee_elf32(elf,(size_t)len,ram,sizeof(ram),&info)==AE3_ELF_IMAGE_OK);
    assert(info.entry==0x00100134 && info.segment_count==1);
    assert(info.copied_bytes==0x62c5d0u && info.zeroed_bss_bytes==0x672910u-0x62c5d0u);
    assert(memcmp(ram+0x100000,elf+0x1000,0x62c5d0u)==0);
    for(size_t i=0x100000+0x62c5d0u;i<0x100000+0x672910u;i++)assert(ram[i]==0);
    assert(ram[0xfffff]==0xa5 && ram[0x100000+0x672910u]==0xa5);
    memset(pristine,0x2f,sizeof(pristine));
    for(int mutation=0;mutation<6;mutation++) {
        memcpy(ram,pristine,sizeof(ram));
        unsigned char backup[4];
        size_t off= mutation==0?0:mutation==1?18:mutation==2?28:
                    mutation==3?0x34+16:mutation==4?0x34+20:24;
        size_t n=mutation==0||mutation==1?1:4;
        memcpy(backup,elf+off,n);
        if(mutation==0)elf[0]='X';
        if(mutation==1)elf[18]=0;
        if(mutation==2)write32(elf+off,0xFFFFFFF0u); /* phoff */
        if(mutation==3)write32(elf+off,0xFFFFFFFFu); /* filesz */
        if(mutation==4)write32(elf+off,0x04000000u); /* memsz, outside RAM */
        if(mutation==5)write32(elf+off,0x00800000u); /* entry outside executable */
        assert(ae3_load_ee_elf32(elf,(size_t)len,ram,sizeof(ram),&info)!=0);
        assert(info.entry==0 && info.segment_count==0);
        assert(memcmp(ram,pristine,sizeof(ram))==0);
        memcpy(elf+off,backup,n);
    }
    printf("PASS ELF guest RAM mapping: entry 0x00100134, one PT_LOAD, exact bytes/BSS, six rejection cases (no guest execution)\n");
    free(elf);
    return 0;
}