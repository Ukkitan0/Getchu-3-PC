#include "ae3_elf_image.h"
#include "ae3_boot_thread_setup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_SIZE (32u*1024u*1024u)
static uint8_t *read_game(const char *name,size_t *size){
    FILE *f=fopen(name,"rb");assert(f);fseek(f,0,SEEK_END);long sz=ftell(f);
    assert(sz>0);rewind(f);uint8_t *b=malloc((size_t)sz);assert(b);
    assert(fread(b,1,(size_t)sz,f)==(size_t)sz);fclose(f);*size=(size_t)sz;return b;
}
int main(int argc,char **argv){
    assert(argc==2);size_t bytes;
    uint8_t *elf=read_game(argv[1],&bytes);
    uint8_t *ram=calloc(RAM_SIZE,1);assert(ram);
    AE3_ElfImage img;assert(ae3_load_ee_elf32(elf,bytes,ram,RAM_SIZE,&img)==AE3_ELF_IMAGE_OK);
    AE3_BootPrefix prefix;ae3_boot_prefix_init(&prefix);
    assert(ae3_boot_prefix_slice(&prefix,ram,RAM_SIZE,350000)==AE3_BOOT_FIRST_CALL_REACHED);
    AE3_BootMemoryFill fill;assert(ae3_boot_fill_enter(&fill,&prefix)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_slice(&fill,ram,RAM_SIZE,200000)==AE3_FILL_RETURNED);
    AE3_BootThreadSetup one,sliced;assert(ae3_boot_thread_enter(&one,&fill)==AE3_THREAD_YIELDED);
    assert(ae3_boot_thread_enter(&sliced,&fill)==AE3_THREAD_YIELDED);
    assert(ae3_boot_thread_slice(&one,ram,RAM_SIZE,50)==AE3_THREAD_SYSCALL);
    unsigned n=0;
    while(!sliced.finished) {
        AE3_ThreadStatus r=ae3_boot_thread_slice(&sliced,ram,RAM_SIZE,1);
        assert(r==AE3_THREAD_YIELDED||r==AE3_THREAD_SYSCALL);
        assert(++n<30);
    }
    assert(!memcmp(&one,&sliced,sizeof one));
    assert(one.cpu.pc==AE3_SETUP_THREAD_SYSCALL_PC);
    assert(one.cpu.gpr[3][0]==AE3_SETUP_THREAD_SYSCALL_NUMBER);
    assert(one.cpu.gpr[4][0]==0x00734070);
    assert(one.cpu.gpr[5][0]==AE3_FILL_BASE);
    assert(one.cpu.gpr[6][0]==AE3_FILL_BYTES);
    assert(one.cpu.gpr[7][0]==0x0072D6C0);
    assert(one.cpu.gpr[8][0]==0x00100388);
    assert(one.cpu.gpr[28][0]==one.cpu.gpr[4][0]);
    assert(one.cpu.gpr[31][0]==AE3_BOOT_RETURN);
    unsigned len=one.cpu.retired-fill.cpu.retired;
    assert(len==12);
    AE3_BootThreadSetup rejected;assert(ae3_boot_thread_enter(&rejected,&fill)==AE3_THREAD_YIELDED);
    uint8_t saved[4];memcpy(saved,ram+AE3_BOOT_RETURN,4);
    memset(ram+AE3_BOOT_RETURN,0xff,4);
    AE3_BootThreadSetup prior=rejected;
    assert(ae3_boot_thread_slice(&rejected,ram,RAM_SIZE,1)==AE3_THREAD_UNSUPPORTED);
    assert(!memcmp(&prior,&rejected,sizeof prior));
    memcpy(ram+AE3_BOOT_RETURN,saved,4);
    assert(ae3_boot_thread_slice(&rejected,ram,RAM_SIZE,40)==AE3_THREAD_SYSCALL);
    assert(!memcmp(&one,&rejected,sizeof one));
    printf("PASS original startup through SetupThread syscall 0x3C at %08X; %u caller instructions after first callee, %u slices\n",
           one.cpu.pc,len,n);
    free(elf);free(ram);return 0;
}