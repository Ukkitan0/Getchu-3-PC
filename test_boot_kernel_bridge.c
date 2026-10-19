#include "ae3_boot_kernel_bridge.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_SIZE (32u*1024u*1024u)

static uint8_t *load_file(const char *file, size_t *n) {
    FILE *f=fopen(file,"rb");assert(f);
    assert(fseek(f,0,SEEK_END)==0);long k=ftell(f);assert(k>0);
    assert(fseek(f,0,SEEK_SET)==0);
    uint8_t *p=malloc((size_t)k);assert(p);
    assert(fread(p,1,(size_t)k,f)==(size_t)k);fclose(f);
    *n=(size_t)k;return p;
}
static void advance_to_kernel(uint8_t *ram,AE3_BootThreadSetup *thread) {
    AE3_BootPrefix prefix;ae3_boot_prefix_init(&prefix);
    assert(ae3_boot_prefix_slice(&prefix,ram,RAM_SIZE,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    AE3_BootMemoryFill fill;
    assert(ae3_boot_fill_enter(&fill,&prefix)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_slice(&fill,ram,RAM_SIZE,200000)==AE3_FILL_RETURNED);
    assert(ae3_boot_thread_enter(thread,&fill)==AE3_THREAD_YIELDED);
    assert(ae3_boot_thread_slice(thread,ram,RAM_SIZE,100)==AE3_THREAD_SYSCALL);
}
/* A fixed test-only value, not a claim about retail BIOS stack placement. */
#define FIXTURE_STACK_POINTER UINT32_C(0x01fffff0)
static unsigned until(AE3_BootKernelBridge *s, uint8_t *ram, AE3_KernelBootResult expected, int sliced) {
    unsigned iters=0;
    for(;;){
       AE3_KernelBootResult rc=ae3_boot_kernel_slice(s,ram,RAM_SIZE,sliced?1:100);
       iters++;
       if(rc==expected)return iters;
       assert(rc==AE3_KBOOT_YIELDED);
       assert(iters<100);
    }
}
static void complete(AE3_BootKernelBridge *s,uint8_t *ram,int sliced){
   assert(until(s,ram,AE3_KBOOT_SETUP_THREAD,sliced)==1);
   assert(s->request.pc==0x001002fc && s->request.number==0x3c);
   uint32_t args[]={0x00734070,0x01fa0000,0x00060000,0x0072d6c0,0x00100388};
   assert(!memcmp(s->request.arguments,args,sizeof args));
   assert(ae3_boot_kernel_return_thread(s,FIXTURE_STACK_POINTER)==AE3_KBOOT_YIELDED);
   assert(s->cpu.gpr[2][0]==FIXTURE_STACK_POINTER && s->cpu.pc==0x00100300);
   assert(until(s,ram,AE3_KBOOT_SETUP_HEAP,sliced)>0);
   assert(s->request.pc==0x00100318 && s->request.number==0x3d);
   assert(s->request.arguments[0]==0x00772910 && s->request.arguments[1]==0x00010000);
   assert(s->cpu.gpr[29][0]==FIXTURE_STACK_POINTER);
   assert(ae3_boot_kernel_ack_heap(s)==AE3_KBOOT_YIELDED);
   assert(s->heap_start==0x00772910 && s->heap_size==0x00010000);
   assert(until(s,ram,AE3_KBOOT_NEXT_CALL,sliced)==1);
   assert(s->cpu.pc==0x0010031c && !s->cpu.in_delay_slot);
   assert(s->completed_thread && s->completed_heap && s->finished);
   assert(s->cpu.retired==315897+8); /* 2 syscalls and 7 intervening guest instructions */
}
int main(int argc,char**argv){
   assert(argc==2);
   size_t elf_size;uint8_t *elf=load_file(argv[1],&elf_size);
   uint8_t *ram=calloc(RAM_SIZE,1);assert(ram);
   AE3_ElfImage report;
   assert(ae3_load_ee_elf32(elf,elf_size,ram,RAM_SIZE,&report)==AE3_ELF_IMAGE_OK);
   AE3_BootThreadSetup ready;advance_to_kernel(ram,&ready);
   AE3_BootKernelBridge a,b,prior;
   assert(ae3_boot_kernel_enter(&a,&ready)==AE3_KBOOT_YIELDED);
   assert(ae3_boot_kernel_enter(&b,&ready)==AE3_KBOOT_YIELDED);
   assert(ae3_boot_kernel_slice(&a,ram,RAM_SIZE,0)==AE3_KBOOT_YIELDED);
   assert(ae3_boot_kernel_slice(&a,ram,RAM_SIZE,1)==AE3_KBOOT_SETUP_THREAD);
   prior=a;
   assert(ae3_boot_kernel_return_thread(&a,0x01fafff1)==AE3_KBOOT_REJECTED);
   assert(memcmp(&prior,&a,sizeof a)==0);
   assert(ae3_boot_kernel_return_thread(&a,0x02000010)==AE3_KBOOT_REJECTED);
   assert(memcmp(&prior,&a,sizeof a)==0);
   complete(&a,ram,0);
   complete(&b,ram,1);
   assert(!memcmp(&a,&b,sizeof a));
   assert(ae3_boot_kernel_slice(&a,ram,RAM_SIZE,30)==AE3_KBOOT_NEXT_CALL);
   assert(ae3_boot_kernel_ack_heap(&a)==AE3_KBOOT_INVALID);
   assert(ae3_boot_kernel_return_thread(&a,FIXTURE_STACK_POINTER)==AE3_KBOOT_INVALID);
   /* Unsupported instruction: guest registers + PC must not change. */
   AE3_BootKernelBridge corrupt;assert(ae3_boot_kernel_enter(&corrupt,&ready)==AE3_KBOOT_YIELDED);
   uint8_t stored[4];memcpy(stored,ram+0x1002fc,4);
   memset(ram+0x1002fc,0xff,4);
   prior=corrupt;
   assert(ae3_boot_kernel_slice(&corrupt,ram,RAM_SIZE,100)==AE3_KBOOT_UNSUPPORTED);
   assert(!memcmp(&corrupt,&prior,sizeof corrupt));
   memcpy(ram+0x1002fc,stored,4);
   assert(ae3_boot_kernel_slice(&corrupt,ram,RAM_SIZE,100)==AE3_KBOOT_SETUP_THREAD);
   /* Missing guest RAM at a later instruction leaves the execution state untouched. */
   assert(ae3_boot_kernel_return_thread(&corrupt,FIXTURE_STACK_POINTER)==AE3_KBOOT_YIELDED);
   prior=corrupt;
   assert(ae3_boot_kernel_slice(&corrupt,ram,0x100300,10)==AE3_KBOOT_MEMORY_FAULT);
   assert(!memcmp(&corrupt,&prior,sizeof corrupt));
   assert(ae3_boot_kernel_slice(&corrupt,ram,RAM_SIZE,20)==AE3_KBOOT_SETUP_HEAP);
   printf("PASS retail boot SetupThread and SetupHeap service boundaries; next JAL PC 0x%08X target 0x%08X (stack response is a test fixture)\n",a.cpu.pc,AE3_KERNEL_NEXT_CALL_TARGET);
   free(ram);free(elf);return 0;
}