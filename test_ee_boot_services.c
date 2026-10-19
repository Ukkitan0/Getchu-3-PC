#include "ae3_elf_image.h"
#include "ae3_boot_prefix.h"
#include "ae3_boot_memory_fill.h"
#include "ae3_boot_thread_setup.h"
#include "ae3_ee_boot_services.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_LEN (32u*1024u*1024u)
static uint8_t *load_file(const char *name,size_t *count){
    FILE *f=fopen(name,"rb");assert(f);
    assert(fseek(f,0,SEEK_END)==0);long n=ftell(f);assert(n>0);
    assert(fseek(f,0,SEEK_SET)==0);uint8_t *p=malloc((size_t)n);assert(p);
    assert(fread(p,1,(size_t)n,f)==(size_t)n);fclose(f);*count=(size_t)n;return p;
}
int main(int argc,char **argv){
    assert(argc==2);size_t len;uint8_t *elf=load_file(argv[1],&len);
    uint8_t *ram=calloc(1,RAM_LEN);assert(ram);
    AE3_ElfImage img;assert(ae3_load_ee_elf32(elf,len,ram,RAM_LEN,&img)==AE3_ELF_IMAGE_OK);
    AE3_BootPrefix p;ae3_boot_prefix_init(&p);
    assert(ae3_boot_prefix_slice(&p,ram,RAM_LEN,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    AE3_BootMemoryFill fill;assert(ae3_boot_fill_enter(&fill,&p)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_slice(&fill,ram,RAM_LEN,200000)==AE3_FILL_RETURNED);
    AE3_BootThreadSetup thread;assert(ae3_boot_thread_enter(&thread,&fill)==AE3_THREAD_YIELDED);
    assert(ae3_boot_thread_slice(&thread,ram,RAM_LEN,100)==AE3_THREAD_SYSCALL);
    AE3_BootKernelBridge k;assert(ae3_boot_kernel_enter(&k,&thread)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_LEN,1)==AE3_KBOOT_SETUP_THREAD);
    AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,RAM_LEN);
    AE3_EEBootServices model;ae3_ee_boot_services_init(&model,16);
    AE3_EEBootServices before=model;AE3_BootKernelBridge saved=k;
    model.reserved_top_bytes=0;
    assert(ae3_ee_boot_services_dispatch(&model,&k,&bus)==AE3_BS_BAD_ARGUMENT);
    assert(!memcmp(&k,&saved,sizeof(k)));
    model=before;
    AE3_EEMemoryBus unmapped=bus;unmapped.ram_size=0x00800000;
    assert(ae3_ee_boot_services_dispatch(&model,&k,&unmapped)==AE3_BS_UNMAPPED);
    assert(!memcmp(&model,&before,sizeof model));assert(!memcmp(&k,&saved,sizeof k));
    assert(ae3_ee_boot_services_dispatch(&model,&k,&bus)==AE3_BS_APPLIED);
    assert(model.thread_created==1&&model.stack_base==0x01fa0000&&model.stack_size==0x60000);
    assert(model.stack_pointer==0x01fffff0&&model.gp==0x00734070&&model.exit_function==0x00100388);
    assert(k.cpu.gpr[2][0]==0x01fffff0 && k.cpu.pc==0x00100300);
    assert(ae3_ee_boot_services_dispatch(&model,&k,&bus)==AE3_BS_WRONG_SYSCALL);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_LEN,30)==AE3_KBOOT_SETUP_HEAP);
    before=model;saved=k;
    model.stack_base=0x00770000;
    assert(ae3_ee_boot_services_dispatch(&model,&k,&bus)==AE3_BS_OVERLAP);
    assert(!memcmp(&k,&saved,sizeof k));
    model=before;
    assert(ae3_ee_boot_services_dispatch(&model,&k,&bus)==AE3_BS_APPLIED);
    assert(model.heap_created&&model.heap_base==0x00772910&&model.heap_size==0x10000);
    assert(model.generation==2 && k.cpu.pc==0x0010031c);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_LEN,30)==AE3_KBOOT_NEXT_CALL);
    printf("PASS EE startup compatibility kernel derives GP=%08x SP=%08x heap=%08x+%x from real guest syscall arguments\n",
           model.gp,model.stack_pointer,model.heap_base,model.heap_size);
    printf("PASS invalid, unmapped and overlapping guest ranges leave guest/kernel state unchanged\n");
    printf("NOTE This is an experimental guest-kernel policy, NOT a verified Sony BIOS implementation.\n");
    free(ram);free(elf);return 0;
}