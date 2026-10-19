/* Original retail boot regression: first SetSyscall boundary after two
 * CreateSema calls serviced by the opt-in native EE compatibility kernel.
 * Initial SetupThread response remains an explicit test fixture. */
#include "ae3_elf_image.h"
#include "ae3_boot_hook_init.h"
#include "ae3_ee_sema_kernel.h"
#include "ae3_guest_find_address.h"
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
 assert(argc==2 || argc==3);size_t n;uint8_t *elf=loadelf(argv[1],&n);
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
 /* A real guest SetSyscall registration must preserve v0 and be routed by
    an explicit kernel table, not by fabricated return values. */
 AE3_EESyscallKernel sys;ae3_ee_syscall_kernel_init(&sys);
 AE3_BootHookInit after=hook;
 const uint64_t v0_before=after.cpu.gpr[2][0];
 uint32_t handler=0;
 assert(ae3_boot_hook_register(&after,&sys,ram,RAM_BYTES)==AE3_HOOK_YIELDED);
 assert(after.cpu.gpr[2][0]==v0_before);
 assert(after.registered==1 && after.registered_numbers[0]==0x83);
 assert(ae3_ee_syscall_lookup(&sys,0x83,&handler)==0 && handler==0x43fcc0);
 assert(ae3_ee_syscall_lookup(&sys,0x5a,&handler)<0);
 AE3_HookResult next=ae3_boot_hook_slice(&after,ram,RAM_BYTES,100);
 assert(next==AE3_HOOK_SET_SYSCALL && after.registered==1);
 assert(after.guest_handler_number==0x5a && after.guest_handler_address==0x43fc88);
 assert(ae3_boot_hook_register(&after,&sys,ram,RAM_BYTES)==AE3_HOOK_YIELDED);
 assert(after.registered==2);
 assert(ae3_ee_syscall_lookup(&sys,0x5a,&handler)==0 && handler==0x43fc88);
 assert(sys.generation==2);
 next=ae3_boot_hook_slice(&after,ram,RAM_BYTES,80);
 assert(next==AE3_HOOK_GUEST_SYSCALL);
 assert(after.cpu.pc==0x43fd04 && after.invoked_number==0x83);
 AE3_BootHookInit waiting=after;
 assert(ae3_boot_hook_route(&after,&sys,&handler)==AE3_HOOK_GUEST_SYSCALL);
 assert(handler==0x43fcc0 && after.invoked_handler_pc==0x43fcc0);
 assert(after.cpu.pc==waiting.cpu.pc && after.cpu.retired==waiting.cpu.retired);
 assert(ae3_boot_hook_slice(&after,ram,RAM_BYTES,5)==AE3_HOOK_GUEST_SYSCALL);
 AE3_EESyscallKernel saved=sys;
 assert(ae3_ee_syscall_register(&sys,256,0x43fcc0,ram,RAM_BYTES)<0);
 assert(ae3_ee_syscall_register(&sys,0x83,3,ram,RAM_BYTES)<0);
 assert(ae3_ee_syscall_register(&sys,0x83,0x3000000,ram,RAM_BYTES)<0);
 assert(!memcmp(&sys,&saved,sizeof sys));
 /* Pre/post effects must be identical when the host schedules one guest
    instruction at a time, even across JR/JAL delay slots and both syscalls. */
 AE3_BootHookInit small=hook;
 AE3_EESyscallKernel smallsys;ae3_ee_syscall_kernel_init(&smallsys);
 AE3_HookResult sr=AE3_HOOK_SET_SYSCALL;
 unsigned calls=0;
 for(unsigned step=0;step<150;step++) {
    if(sr==AE3_HOOK_SET_SYSCALL){
        assert(ae3_boot_hook_register(&small,&smallsys,ram,RAM_BYTES)==AE3_HOOK_YIELDED);
        calls++;
    }
    sr=ae3_boot_hook_slice(&small,ram,RAM_BYTES,1);
    assert(sr==AE3_HOOK_YIELDED || sr==AE3_HOOK_SET_SYSCALL || sr==AE3_HOOK_GUEST_SYSCALL);
    if(sr==AE3_HOOK_GUEST_SYSCALL)break;
 }
 assert(sr==AE3_HOOK_GUEST_SYSCALL && calls==2);
 assert(ae3_boot_hook_route(&small,&smallsys,&handler)==AE3_HOOK_GUEST_SYSCALL);
 assert(!memcmp(&small,&after,sizeof small));
 assert(!memcmp(&smallsys,&sys,sizeof sys));
 /* A registration error is atomic: no kernel mutation, no guest CPU advance. */
 AE3_BootHookInit invalid=hook, previous=invalid;
 AE3_EESyscallKernel untouched;ae3_ee_syscall_kernel_init(&untouched);
 AE3_EESyscallKernel initial=untouched;
 invalid.guest_handler_address=3;
 assert(ae3_boot_hook_register(&invalid,&untouched,ram,RAM_BYTES)==AE3_HOOK_UNSUPPORTED);
 assert(invalid.cpu.pc==previous.cpu.pc && invalid.cpu.retired==previous.cpu.retired);
 assert(invalid.pending==previous.pending);
 assert(!memcmp(&untouched,&initial,sizeof untouched));
 /* Execute the ORIGINAL installed guest syscall 0x83 with an explicitly
    synthetic kernel-memory fixture. Never use this to claim BIOS accuracy. */
 uint8_t *kseg=calloc(0x80000,1);assert(kseg);
 kseg[12]=0xc0;kseg[13]=0xfc;kseg[14]=0x43; /* 0x0043FCC0 */
 kseg[0x140]=0x88;kseg[0x141]=0xfc;kseg[0x142]=0x43; /* 0x0043FC88 */
 uint32_t targets[2]={0x8000000cu,0x80000140u};
 for(unsigned which=0;which<2;which++) {
    assert(ae3_boot_hook_route(&after,&sys,&handler)==AE3_HOOK_GUEST_SYSCALL);
    assert(handler==0x43fcc0);
    AE3_GuestFindAddress find,prior_find,single;
    assert(ae3_guest_find_begin(&find,&after)==AE3_FIND_YIELDED);
    prior_find=find;
    assert(ae3_guest_find_slice(&find,ram,RAM_BYTES,NULL,0x80000000,0,100)==AE3_FIND_KERNEL_UNMAPPED);
    assert(!memcmp(&find,&prior_find,sizeof find));
    assert(ae3_guest_find_slice(&find,ram,RAM_BYTES,kseg,0x80000000,8,100)==AE3_FIND_KERNEL_UNMAPPED);
    AE3_GuestFindAddress fault_state=find;
    assert(ae3_guest_find_slice(&find,ram,RAM_BYTES,kseg,0x80000000,8,1)==AE3_FIND_KERNEL_UNMAPPED);
    assert(!memcmp(&find,&fault_state,sizeof find));
    /* Earlier valid instructions were retired before the short window fault. */
    find=prior_find;
    AE3_FindResult fr=ae3_guest_find_slice(&find,ram,RAM_BYTES,kseg,0x80000000,0x80000,4000);
    assert(fr==AE3_FIND_RETURNED);
    assert(find.matched_guest_address==targets[which]);
    assert((uint32_t)find.cpu.gpr[2][0]==targets[which]);
    assert(find.return_pc==(which==0?0x43fda4u:0x43fdb8u));
    /* Optional REAL captured EE RAM from the user's PCSX2 save state.
       This evidence may reflect patches made after boot and must not be
       mistaken for the original unmodified BIOS boot-time contents. */
    if(argc==3) {
       FILE *kf=fopen(argv[2],"rb");assert(kf);
       uint8_t *recorded=malloc(0x80004u);assert(recorded);
       assert(fread(recorded,1,0x80004u,kf)>=0x80000u);
       fclose(kf);
       AE3_GuestFindAddress observed=prior_find;
       AE3_FindResult real=ae3_guest_find_slice(&observed,ram,RAM_BYTES,
          recorded,0x80000000u,0x80004u,2000000);
       assert(real==AE3_FIND_RETURNED);
       printf("REFERENCE PCSX2 memory (post-boot): FindAddress for guest value %08X returned %08X after %u kernel reads. Not proof of boot-time kernel state.\n",
          (unsigned)(which==0?0x43fcc0u:0x43fc88u),
          observed.matched_guest_address,observed.loads);
       free(recorded);
    }
    single=prior_find;
    for(unsigned step=0;step<4000;step++) {
       fr=ae3_guest_find_slice(&single,ram,RAM_BYTES,kseg,0x80000000,0x80000,1);
       if(fr==AE3_FIND_RETURNED)break;
       assert(fr==AE3_FIND_YIELDED);
    }
    assert(fr==AE3_FIND_RETURNED);
    assert(!memcmp(&single,&find,sizeof find));
    assert(ae3_guest_find_commit(&after,&find)==AE3_FIND_RETURNED);
    assert(after.cpu.pc==0x43fd08 && after.guest_completed==which+1);
    AE3_HookResult h=ae3_boot_hook_slice(&after,ram,RAM_BYTES,90);
    assert(h==(which==0?AE3_HOOK_GUEST_SYSCALL:AE3_HOOK_FIND_PAIR_COMPLETE));
 }
 assert(after.cpu.pc==0x43fdc4);
 assert(after.cpu.gpr[19][0]==0xffffffff8000000cULL);
 assert(after.cpu.gpr[18][0]==0xffffffff80000140ULL);
 free(kseg);
 printf("PASS original guest FindAddress handler executed twice against clearly synthetic mapped kernel window, with 1-step parity and atomic unmapped faults\n");
 printf("PASS registered SetSyscall 0x83 and 0x5A, executed original wrappers, routed 0x83 to guest 0x43FCC0 without synthetic return\n");
 printf("PASS retail nested initialization 0x43FD48 until real SetSyscall 0x74 at 0x43FE4C; retryable store fault and sliced equivalence\n");
 free(ram);free(elf);return 0;
}