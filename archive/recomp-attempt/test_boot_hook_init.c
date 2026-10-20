/* Original retail boot regression: first SetSyscall boundary after two
 * CreateSema calls serviced by the opt-in native EE compatibility kernel.
 * Initial SetupThread response remains an explicit test fixture. */
#include "ae3_elf_image.h"
#include "ae3_boot_hook_init.h"
#include "ae3_ee_sema_kernel.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_BYTES (32u*1024u*1024u)
static uint32_t rd32(const uint8_t*p){return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;}
static uint8_t *loadelf(const char *name,size_t *n) {
    FILE *f=fopen(name,"rb");assert(f);assert(fseek(f,0,SEEK_END)==0);
    long length=ftell(f);assert(length>0);assert(fseek(f,0,SEEK_SET)==0);
    uint8_t *buf=malloc((size_t)length);assert(buf);
    assert(fread(buf,1,(size_t)length,f)==(size_t)length);fclose(f);*n=(size_t)length;return buf;
}
static AE3_BootInitPrefix ready(uint8_t *ram) {
    AE3_BootPrefix p;ae3_boot_prefix_init(&p);
    assert(ae3_boot_prefix_slice(&p,ram,RAM_BYTES,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    AE3_BootMemoryFill fill;assert(ae3_boot_fill_enter(&fill,&p)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_slice(&fill,ram,RAM_BYTES,200000)==AE3_FILL_RETURNED);
    AE3_BootThreadSetup t;assert(ae3_boot_thread_enter(&t,&fill)==AE3_THREAD_YIELDED);
    assert(ae3_boot_thread_slice(&t,ram,RAM_BYTES,30)==AE3_THREAD_SYSCALL);
    AE3_BootKernelBridge k;assert(ae3_boot_kernel_enter(&k,&t)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_BYTES,1)==AE3_KBOOT_SETUP_THREAD);
    /* Synthetic kernel value used ONLY to test subsequent original guest code. */
    assert(ae3_boot_kernel_return_thread(&k,0x01fffff0)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_BYTES,30)==AE3_KBOOT_SETUP_HEAP);
    assert(ae3_boot_kernel_ack_heap(&k)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_BYTES,30)==AE3_KBOOT_NEXT_CALL);
    AE3_BootInitPrefix init;assert(ae3_boot_init_enter(&init,&k)==AE3_INIT_YIELDED);
    assert(ae3_boot_init_slice(&init,ram,RAM_BYTES,10)==AE3_INIT_SUBCALL);
    return init;
}
static void advance(AE3_BootHookInit *out, uint8_t *ram) {
 AE3_BootInitPrefix init=ready(ram);AE3_BootSemaphoreInit sema;
 assert(ae3_boot_sema_enter(&sema,&init)==AE3_SEMA_YIELDED);
 AE3_EESemaKernel kernel;ae3_ee_sema_kernel_init(&kernel);
 int count=0;
 for(int i=0;i<90;i++){
  AE3_SemaBootResult r=ae3_boot_sema_slice(&sema,ram,RAM_BYTES,100);
  if(r==AE3_SEMA_NEXT_CALL)break;
  assert(r==AE3_SEMA_CREATE_REQUEST && count<2);
  assert(ae3_ee_sema_boot_dispatch(&kernel,&sema)==AE3_SEMA_YIELDED);
  count++;
 }
 assert(count==2 && sema.done);
 assert(rd32(ram+0x006a1ea8)==1 && rd32(ram+0x006a1eac)==2);
 assert(ae3_boot_hook_enter(out,&sema)==AE3_HOOK_YIELDED);
}
int main(int argc,char **argv){
 assert(argc==2);size_t n;uint8_t *elf=loadelf(argv[1],&n);
 uint8_t *ram=calloc(RAM_BYTES,1);assert(ram);
 AE3_ElfImage img;assert(ae3_load_ee_elf32(elf,n,ram,RAM_BYTES,&img)==AE3_ELF_IMAGE_OK);
 AE3_BootHookInit hook,prior;advance(&hook,ram);
 assert(ae3_boot_hook_slice(&hook,ram,RAM_BYTES,100)==AE3_HOOK_SET_SYSCALL);
 assert(hook.cpu.pc==AE3_HOOK_SYSCALL_PC && hook.syscall_number==0x74);
 assert(hook.cpu.gpr[3][0]==0x74);
 assert(hook.guest_handler_number==rd32(ram+0x006a1e98));
 assert(hook.guest_handler_address==rd32(ram+0x006a1e9c));
 assert(ae3_boot_hook_slice(&hook,ram,RAM_BYTES,10)==AE3_HOOK_SET_SYSCALL);
 AE3_BootHookInit sliced;advance(&sliced,ram);
 AE3_HookResult rc=AE3_HOOK_YIELDED;
 for(int i=0;i<70;i++){
  rc=ae3_boot_hook_slice(&sliced,ram,RAM_BYTES,1);
  if(rc==AE3_HOOK_SET_SYSCALL)break;
  assert(rc==AE3_HOOK_YIELDED);
 }
 assert(rc==AE3_HOOK_SET_SYSCALL);
 assert(!memcmp(&sliced,&hook,sizeof hook));
 AE3_BootHookInit fault;advance(&fault,ram);
 for(int i=0;i<30 && fault.cpu.pc!=0x0043fd50;i++)
   assert(ae3_boot_hook_slice(&fault,ram,RAM_BYTES,1)==AE3_HOOK_YIELDED);
 assert(fault.cpu.pc==0x0043fd50);
 prior=fault;
 assert(ae3_boot_hook_slice(&fault,ram,0x01ffff00,1)==AE3_HOOK_MEMORY_FAULT);
 assert(!memcmp(&fault,&prior,sizeof prior));
 assert(ae3_boot_hook_slice(&fault,ram,RAM_BYTES,1)==AE3_HOOK_YIELDED);
 printf("PASS retail nested initialization 0x43FD48 until real SetSyscall 0x74 at 0x43FE4C; retryable store fault and sliced equivalence\n");
 free(ram);free(elf);return 0;
}