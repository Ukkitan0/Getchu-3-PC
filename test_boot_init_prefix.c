#include "ae3_elf_image.h"
#include "ae3_boot_init_prefix.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define RAM_BYTES (32u*1024u*1024u)
static uint8_t *read_file(const char *p,size_t *n) {
 FILE *f=fopen(p,"rb");assert(f);fseek(f,0,SEEK_END);long sz=ftell(f);assert(sz>0);
 rewind(f);uint8_t *b=malloc((size_t)sz);assert(b);assert(fread(b,1,(size_t)sz,f)==(size_t)sz);
 fclose(f);*n=(size_t)sz;return b;
}
static void prepare(uint8_t *ram,AE3_BootKernelBridge *k){
 AE3_BootPrefix p;ae3_boot_prefix_init(&p);
 assert(ae3_boot_prefix_slice(&p,ram,RAM_BYTES,400000)==AE3_BOOT_FIRST_CALL_REACHED);
 AE3_BootMemoryFill f;assert(ae3_boot_fill_enter(&f,&p)==AE3_FILL_YIELDED);
 assert(ae3_boot_fill_slice(&f,ram,RAM_BYTES,200000)==AE3_FILL_RETURNED);
 AE3_BootThreadSetup t;assert(ae3_boot_thread_enter(&t,&f)==AE3_THREAD_YIELDED);
 assert(ae3_boot_thread_slice(&t,ram,RAM_BYTES,30)==AE3_THREAD_SYSCALL);
 assert(ae3_boot_kernel_enter(k,&t)==AE3_KBOOT_YIELDED);
 assert(ae3_boot_kernel_slice(k,ram,RAM_BYTES,1)==AE3_KBOOT_SETUP_THREAD);
 /* Synthetic stack response to isolate userland instruction behavior. */
 assert(ae3_boot_kernel_return_thread(k,0x01fffff0)==AE3_KBOOT_YIELDED);
 assert(ae3_boot_kernel_slice(k,ram,RAM_BYTES,20)==AE3_KBOOT_SETUP_HEAP);
 assert(ae3_boot_kernel_ack_heap(k)==AE3_KBOOT_YIELDED);
 assert(ae3_boot_kernel_slice(k,ram,RAM_BYTES,20)==AE3_KBOOT_NEXT_CALL);
}
int main(int argc,char**argv){assert(argc==2);size_t n;
 uint8_t*elf=read_file(argv[1],&n);uint8_t*ram=calloc(RAM_BYTES,1);assert(ram);
 AE3_ElfImage image;assert(ae3_load_ee_elf32(elf,n,ram,RAM_BYTES,&image)==AE3_ELF_IMAGE_OK);
 AE3_BootKernelBridge k;prepare(ram,&k);
 AE3_BootInitPrefix a,b,prior;assert(ae3_boot_init_enter(&a,&k)==AE3_INIT_YIELDED);
 assert(ae3_boot_init_enter(&b,&k)==AE3_INIT_YIELDED);
 assert(ae3_boot_init_slice(&a,ram,RAM_BYTES,10)==AE3_INIT_SUBCALL);
 for(int j=0;j<4;j++) {
  AE3_InitResult r=ae3_boot_init_slice(&b,ram,RAM_BYTES,1);
  assert(r==(j==3?AE3_INIT_SUBCALL:AE3_INIT_YIELDED));
 }
 assert(!memcmp(&a,&b,sizeof a));
 assert(a.cpu.pc==AE3_INIT_FIRST_SUBCALL_PC);
 assert(a.cpu.gpr[31][0]==0x00100324);
 assert(a.cpu.gpr[29][0]==0x01ffffe0);
 uint64_t stored=0;for(int j=0;j<8;j++)stored|=(uint64_t)ram[0x01ffffe0+j]<<(8*j);
 assert(stored==0x00100324);
 assert(a.cpu.retired==k.cpu.retired+4);
 assert(ae3_boot_init_slice(&a,ram,RAM_BYTES,100)==AE3_INIT_SUBCALL);
 AE3_BootInitPrefix retry;assert(ae3_boot_init_enter(&retry,&k)==AE3_INIT_YIELDED);
 assert(ae3_boot_init_slice(&retry,ram,RAM_BYTES,2)==AE3_INIT_YIELDED);
 prior=retry;
 assert(ae3_boot_init_slice(&retry,ram,0x43fe58,10)==AE3_INIT_MEMORY_FAULT);
 assert(!memcmp(&retry,&prior,sizeof retry));
 assert(ae3_boot_init_slice(&retry,ram,RAM_BYTES,2)==AE3_INIT_SUBCALL);
 printf("PASS original first initialization JAL into 0x0043FE58, stack save and next call at 0x0043FE60 (test-only kernel response)\n");
 free(elf);free(ram);return 0;
}