/* Real retail boot probe; NEVER a playable game. No BIOS assumptions by default.
 * --test-kernel-fixture uses an explicitly synthetic stack pointer solely to
 * validate guest userland instructions beyond SetupThread/SetupHeap. */
#include "ae3_elf_image.h"
#include "ae3_boot_init_prefix.h"
#include "ae3_ee_sema_kernel.h"
#include "ae3_boot_hook_init.h"
#include "ae3_ee_syscall_kernel.h"
#include "ae3_guest_find_address.h"
#include "ae3_ee_memory_bus.h"
#include "ae3_ee_boot_services.h"
#include "ae3_boot_post_find.h"
#include "ae3_boot_next_init.h"
#include "ae3_boot_timer3_resume.h"
#include "ae3_ee_timer3.h"
#include "ae3_boot_timer3_syscalls.h"
#include "ae3_boot_post_patch.h"
#include "ae3_boot_alloc_init.h"
#include "ae3_boot_post_intc.h"
#include "ae3_boot_intc_critical.h"
#include "ae3_boot_timer2_setup.h"
#include "ae3_boot_timer2_recheck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_SIZE (32u*1024u*1024u)
static uint8_t *read_file(const char *path,size_t *n) {
 FILE *f=fopen(path,"rb");if(!f)return NULL;
 if(fseek(f,0,SEEK_END)){fclose(f);return NULL;}
 long size=ftell(f);if(size<=0||fseek(f,0,SEEK_SET)){fclose(f);return NULL;}
 uint8_t *p=(uint8_t*)malloc((size_t)size);if(!p){fclose(f);return NULL;}
 if(fread(p,1,(size_t)size,f)!=(size_t)size){fclose(f);free(p);return NULL;}
 fclose(f);*n=(size_t)size;return p;
}
int main(int argc,char**argv) {
 int model_mode=argc>=3 && strcmp(argv[2],"--test-ee-boot-model")==0;
 if(argc<2||argc>6||(argc>=3&&strcmp(argv[2],"--test-kernel-fixture")&&!model_mode)||
    (argc>=4&&strcmp(argv[3],"--test-sema-hle"))||
    (argc>=5&&strcmp(argv[4],"--test-ee-alias"))||
    (argc==6&&strcmp(argv[5],"--test-post-find")&&strcmp(argv[5],"--test-next-mmio")&&strcmp(argv[5],"--test-timer3")&&strcmp(argv[5],"--test-timer3-enabled")&&strcmp(argv[5],"--test-timer3-copy")&&(strcmp(argv[5],"--test-timer3-copy-alloc")&&strcmp(argv[5],"--test-timer2-mode")))) {
  fprintf(stderr,"Usage: %s SCUS_975.01 [--test-kernel-fixture|--test-ee-boot-model [--test-sema-hle [--test-ee-alias [--test-post-find|--test-next-mmio|--test-timer3|--test-timer3-enabled|--test-timer3-copy|--test-timer3-copy-alloc|--test-timer2-mode]]]\n",argv[0]);return 2;
 }
 size_t bytes=0;uint8_t *elf=read_file(argv[1],&bytes);
 if(!elf){fprintf(stderr,"Cannot read game executable\n");return 2;}
 uint8_t *ram=(uint8_t*)calloc(RAM_SIZE,1);if(!ram){free(elf);return 2;}
 AE3_ElfImage img;
 int status=ae3_load_ee_elf32(elf,bytes,ram,RAM_SIZE,&img);
 if(status!=AE3_ELF_IMAGE_OK||img.entry!=AE3_BOOT_ENTRY){fprintf(stderr,"ELF mapping or entry rejected (%d)\n",status);free(elf);free(ram);return 3;}
 AE3_BootPrefix prefix;ae3_boot_prefix_init(&prefix);
 if(ae3_boot_prefix_slice(&prefix,ram,RAM_SIZE,400000)!=AE3_BOOT_FIRST_CALL_REACHED){fprintf(stderr,"Boot prefix stopped at %08X\n",prefix.pc);free(elf);free(ram);return 4;}
 AE3_BootMemoryFill fill;
 if(ae3_boot_fill_enter(&fill,&prefix)!=AE3_FILL_YIELDED||ae3_boot_fill_slice(&fill,ram,RAM_SIZE,200000)!=AE3_FILL_RETURNED){fprintf(stderr,"Stack fill did not return\n");free(elf);free(ram);return 5;}
 AE3_BootThreadSetup thread;
 if(ae3_boot_thread_enter(&thread,&fill)!=AE3_THREAD_YIELDED||ae3_boot_thread_slice(&thread,ram,RAM_SIZE,100)!=AE3_THREAD_SYSCALL){fprintf(stderr,"SetupThread prologue failed\n");free(elf);free(ram);return 6;}
 AE3_BootKernelBridge k;
 if(ae3_boot_kernel_enter(&k,&thread)!=AE3_KBOOT_YIELDED||ae3_boot_kernel_slice(&k,ram,RAM_SIZE,1)!=AE3_KBOOT_SETUP_THREAD){fprintf(stderr,"SetupThread syscall decode failed\n");free(elf);free(ram);return 7;}
 printf("Verified real retail EE startup: %llu retired guest instructions\n",(unsigned long long)k.cpu.retired);
 printf("Kernel service pending: SetupThread ($v1=%02X) at %08X\n",k.request.number,k.request.pc);
 printf("  gp=%08X stack=%08X stack_size=%08X args=%08X root=%08X\n",k.request.arguments[0],k.request.arguments[1],k.request.arguments[2],k.request.arguments[3],k.request.arguments[4]);
 if(argc==2){printf("Stopped correctly: no verified BIOS response supplied (not playable).\n");free(elf);free(ram);return 0;}
 AE3_EEMemoryBus bootbus;ae3_ee_bus_init(&bootbus,ram,RAM_SIZE);
 AE3_EEBootServices bootmodel;ae3_ee_boot_services_init(&bootmodel,16);
 if(model_mode) {
    printf("EXPERIMENTAL startup kernel compatibility policy: reserve 16 bytes below guest stack top; NOT Sony BIOS execution.\n");
    if(ae3_ee_boot_services_dispatch(&bootmodel,&k,&bootbus)!=AE3_BS_APPLIED){
        fprintf(stderr,"Compatibility SetupThread rejected guest arguments\n");free(elf);free(ram);return 8;
    }
    printf("Guest stack derived from real syscall arguments: %08X + %08X - %08X = %08X\n",
        bootmodel.stack_base,bootmodel.stack_size,bootmodel.reserved_top_bytes,bootmodel.stack_pointer);
 } else {
    printf("TEST MODE: synthetic 0x01FFFFF0 kernel stack return -- NOT the verified BIOS value.\n");
    if(ae3_boot_kernel_return_thread(&k,0x01fffff0)!=AE3_KBOOT_YIELDED){
        fprintf(stderr,"Fixture SetupThread failed\n");free(elf);free(ram);return 8;
    }
 }
 if(ae3_boot_kernel_slice(&k,ram,RAM_SIZE,20)!=AE3_KBOOT_SETUP_HEAP){fprintf(stderr,"Fixture SetupThread follow-up failed\n");free(elf);free(ram);return 8;}
 printf("Kernel service pending: SetupHeap at %08X start=%08X size=%08X\n",k.request.pc,k.request.arguments[0],k.request.arguments[1]);
 if((model_mode ? (ae3_ee_boot_services_dispatch(&bootmodel,&k,&bootbus)==AE3_BS_APPLIED) :
      (ae3_boot_kernel_ack_heap(&k)==AE3_KBOOT_YIELDED))==0 ||
    ae3_boot_kernel_slice(&k,ram,RAM_SIZE,20)!=AE3_KBOOT_NEXT_CALL){fprintf(stderr,"Fixture SetupHeap follow-up failed\n");free(elf);free(ram);return 9;}
 if(model_mode) printf("EXPERIMENTAL kernel heap recorded: %08X+%08X (not an authentic BIOS side effect).\n",bootmodel.heap_base,bootmodel.heap_size);
 AE3_BootInitPrefix init;
 if(ae3_boot_init_enter(&init,&k)!=AE3_INIT_YIELDED||ae3_boot_init_slice(&init,ram,RAM_SIZE,10)!=AE3_INIT_SUBCALL){fprintf(stderr,"First init prologue failed\n");free(elf);free(ram);return 10;}
 printf("Original guest first initialization entered at %08X and next real nested call at %08X -> %08X\n",AE3_KERNEL_NEXT_CALL_TARGET,init.cpu.pc,AE3_INIT_FIRST_SUBCALL_TARGET);
 if(argc>=4) {
    AE3_EESemaKernel semakernel;ae3_ee_sema_kernel_init(&semakernel);
    AE3_BootSemaphoreInit sema;
    if(ae3_boot_sema_enter(&sema,&init)!=AE3_SEMA_YIELDED){fprintf(stderr,"Semaphore entry failed\n");free(elf);free(ram);return 11;}
    unsigned served=0;
    for(unsigned i=0;i<10;i++) {
      AE3_SemaBootResult sr=ae3_boot_sema_slice(&sema,ram,RAM_SIZE,100);
      if(sr==AE3_SEMA_NEXT_CALL)break;
      if(sr!=AE3_SEMA_CREATE_REQUEST||served>=2){fprintf(stderr,"Semaphore wrapper stopped: %d at %08X\n",(int)sr,sema.cpu.pc);free(elf);free(ram);return 12;}
      printf("EE CreateSema request %u at %08X guest descriptor=%08X max=%u initial=%u\n",served+1,sema.request.pc,sema.request.guest_address,sema.request.values[1],sema.request.values[2]);
      if(ae3_ee_sema_boot_dispatch(&semakernel,&sema)!=AE3_SEMA_YIELDED){fprintf(stderr,"Semaphore HLE failed\n");free(elf);free(ram);return 13;}
      served++;
    }
    if(!sema.done||served!=2){fprintf(stderr,"Semaphore initialization incomplete\n");free(elf);free(ram);return 14;}
    printf("EXPERIMENTAL HLE: two semaphore IDs stored at 0x006A1EA8/0x006A1EAC (not verified BIOS IDs)\n");
    AE3_BootHookInit hook;
    if(ae3_boot_hook_enter(&hook,&sema)!=AE3_HOOK_YIELDED ||
      ae3_boot_hook_slice(&hook,ram,RAM_SIZE,100)!=AE3_HOOK_SET_SYSCALL){fprintf(stderr,"SetSyscall boundary was not reached\n");free(elf);free(ram);return 15;}
    printf("Original SetSyscall 0x74 pending at %08X: syscall_no=%08X handler=%08X\n",hook.cpu.pc,hook.guest_handler_number,hook.guest_handler_address);
    AE3_EESyscallKernel syscall_kernel;ae3_ee_syscall_kernel_init(&syscall_kernel);
    int routed=0;
    for(unsigned step=0;step<8;step++) {
       if(ae3_boot_hook_register(&hook,&syscall_kernel,ram,RAM_SIZE)!=AE3_HOOK_YIELDED){
          fprintf(stderr,"SetSyscall registration failed\n");free(elf);free(ram);return 16;
       }
       printf("EXPERIMENTAL kernel registered syscall %02X -> original guest handler %08X\n",
          hook.registered_numbers[hook.registered-1],hook.registered_handlers[hook.registered-1]);
       AE3_HookResult state=ae3_boot_hook_slice(&hook,ram,RAM_SIZE,100);
       if(state==AE3_HOOK_GUEST_SYSCALL){
          uint32_t handler=0;
          if(ae3_boot_hook_route(&hook,&syscall_kernel,&handler)!=AE3_HOOK_GUEST_SYSCALL){
             fprintf(stderr,"Could not route guest syscall\n");free(elf);free(ram);return 17;
          }
          printf("Original syscall %02X reached at %08X: registered guest handler %08X\n",
             hook.invoked_number,hook.cpu.pc,handler);
          if(argc>=5) {
             printf("TEST ONLY: running original 0x83 MIPS handler with KSEG0 aliased to current EE RAM.\n");
             printf("  BIOS/kernel was not initialized: no authentic startup kernel bytes are claimed.\n");
             AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,RAM_SIZE);
             if(argc==6&&(!strcmp(argv[5],"--test-next-mmio")||!strcmp(argv[5],"--test-timer3")||!strcmp(argv[5],"--test-timer3-enabled")||!strcmp(argv[5],"--test-timer3-copy")||!(strcmp(argv[5],"--test-timer3-copy-alloc")&&strcmp(argv[5],"--test-timer2-mode")))) {
                /* Explicitly artificial values, with the exact offset
                   relationship required by the retail reconciliation code. */
                if(ae3_ee_bus_write32(&bus,UINT32_C(0x80000300),AE3_FIND_ENTRY)!=AE3_EE_BUS_OK||
                   ae3_ee_bus_write32(&bus,UINT32_C(0x8000025C),UINT32_C(0x0043FC88))!=AE3_EE_BUS_OK){
                  fprintf(stderr,"Could not stage test-only kernel lookup words\n");free(elf);free(ram);return 27;
                }
                printf("TEST FIXTURE: staged non-BIOS syscall lookup words at 80000300 and 8000025C.\n");
             }
             for(unsigned which=0;which<2;which++){
                AE3_GuestFindAddress find;
                if(ae3_guest_find_begin(&find,&hook)!=AE3_FIND_YIELDED){
                  fprintf(stderr,"Guest handler setup failed\n");free(elf);free(ram);return 20;
                }
                AE3_FindResult fr=ae3_guest_find_slice_bus(&find,&bus,1000000);
                if(fr!=AE3_FIND_RETURNED){
                  fprintf(stderr,"Original guest handler failed: %d PC=%08X\n",fr,find.cpu.pc);
                  free(elf);free(ram);return 21;
                }
                printf("  Guest FindAddress #%u returned %08X after %u physical RAM reads\n",
                   which+1,find.matched_guest_address,find.loads);
                if(ae3_guest_find_commit(&hook,&find)!=AE3_FIND_RETURNED){
                  fprintf(stderr,"Handler return commit failed\n");free(elf);free(ram);return 22;
                }
                AE3_HookResult follow=ae3_boot_hook_slice(&hook,ram,RAM_SIZE,100);
                if(which==0) {
                  if(follow!=AE3_HOOK_GUEST_SYSCALL||
                     ae3_boot_hook_route(&hook,&syscall_kernel,&handler)!=AE3_HOOK_GUEST_SYSCALL){
                     fprintf(stderr,"Second original syscall not reached\n");free(elf);free(ram);return 23;
                  }
                }else if(follow!=AE3_HOOK_FIND_PAIR_COMPLETE){
                  fprintf(stderr,"FindAddress pair did not complete: %d\n",follow);
                  free(elf);free(ram);return 24;
                }
             }
             printf("Original code reached %08X after both guest handlers (still no BIOS/kernel services).\n",hook.cpu.pc);
             if(argc==6) {
                AE3_BootPostFind post;
                if(ae3_boot_post_find_enter(&post,&hook)!=AE3_POST_YIELDED) {
                    fprintf(stderr,"Cannot resume original post-FindAddress routine\n");
                    free(elf);free(ram);return 25;
                }
                AE3_PostFindResult stage=ae3_boot_post_find_slice(&post,&bus,400);
                if(stage==AE3_POST_FIND_REQUEST) {
                    printf("Original reconciliation loop requests another FindAddress at %08X from %08X (retired %llu).\n",
                       post.cpu.pc,(uint32_t)post.cpu.gpr[4][0],
                       (unsigned long long)post.cpu.retired);
                    printf("Kernel data absent: stopped before third syscall, no synthetic address inserted.\n");
                }else if(stage==AE3_POST_NEXT_INIT) {
                    printf("Original reconciliation reached next real init JAL at %08X.\n",post.cpu.pc);
                    if(!strcmp(argv[5],"--test-next-mmio")||!strcmp(argv[5],"--test-timer3")||!strcmp(argv[5],"--test-timer3-enabled")||!strcmp(argv[5],"--test-timer3-copy")||!(strcmp(argv[5],"--test-timer3-copy-alloc")&&strcmp(argv[5],"--test-timer2-mode"))) {
                        AE3_BootNextInit next;
                        if(ae3_boot_next_init_enter(&next,&post)!=AE3_NEXT_YIELDED ||
                           ae3_boot_next_init_slice(&next,&bus,40)!=AE3_NEXT_MMIO_PENDING){
                           fprintf(stderr,"Next original guest prologue did not reach MMIO boundary\n");
                           free(elf);free(ram);return 28;
                        }
                        printf("Next retail callee entered at %08X; hardware read pending at %08X address %08X (retired %llu).\n",
                          AE3_NEXT_INIT_CALLEE,next.cpu.pc,AE3_NEXT_INIT_MMIO_ADDRESS,
                          (unsigned long long)next.cpu.retired);
                        if(!strcmp(argv[5],"--test-next-mmio")){
                            printf("No hardware register read was simulated. Not a playable game.\n");
                        }else{
                            /* Explicit device-model continuation. MODE defaults
                             * to zero; the enabled case is a deliberate test
                             * fixture, NOT an inference about Sony BIOS. */
                            AE3_EETimer3 timer;ae3_ee_timer3_reset(&timer);
                            ae3_ee_bus_attach_mmio(&bus,ae3_ee_timer3_read,ae3_ee_timer3_write,&timer);
                            if(!strcmp(argv[5],"--test-timer3-enabled") &&
                               ae3_ee_bus_write32(&bus,AE3_T3_MODE,AE3_T3_CMPE)!=AE3_EE_BUS_OK){
                               fprintf(stderr,"Could not stage test-only Timer3 bit 8\n");free(elf);free(ram);return 29;
                            }
                            AE3_BootTimer3Resume resume;
                            if(ae3_boot_timer3_enter(&resume,&next)!=AE3_TIMER_BOOT_YIELDED){
                               fprintf(stderr,"Timer3 guest code entry failed\n");free(elf);free(ram);return 30;
                            }
                            AE3_TimerBootResult result=ae3_boot_timer3_slice(&resume,&bus,40);
                            if(result!=AE3_TIMER_BOOT_NEXT_CALL&&result!=AE3_TIMER_BOOT_RETURNED){
                               fprintf(stderr,"Timer3 guest continuation failed at %08X (result %d)\n",resume.cpu.pc,result);
                               free(elf);free(ram);return 31;
                            }
                            printf("EXPERIMENTAL EE Timer3 model: MODE=%08X, original guest result %s, PC=%08X, retired=%llu\n",
                               timer.mode,result==AE3_TIMER_BOOT_RETURNED?"returned to first-init caller":"next SetSyscall JAL pending",
                               resume.cpu.pc,(unsigned long long)resume.cpu.retired);
                            if(!strcmp(argv[5],"--test-timer3-copy")||!(strcmp(argv[5],"--test-timer3-copy-alloc")&&strcmp(argv[5],"--test-timer2-mode"))){
                               AE3_Timer3Syscalls t3sys;
                               if(result!=AE3_TIMER_BOOT_NEXT_CALL||
                                  ae3_timer3_syscalls_enter(&t3sys,&resume)!=AE3_TS_YIELDED){
                                  fprintf(stderr,"Timer3 syscall wrappers not entered\n");free(elf);free(ram);return 32;
                               }
                               if(ae3_timer3_syscalls_slice(&t3sys,&bus,40)!=AE3_TS_SET_SYSCALL||
                                  ae3_timer3_syscalls_dispatch(&t3sys,&bus,&syscall_kernel)!=AE3_TS_YIELDED||
                                  ae3_timer3_syscalls_slice(&t3sys,&bus,40)!=AE3_TS_COPY_SYSCALL||
                                  ae3_timer3_syscalls_dispatch(&t3sys,&bus,&syscall_kernel)!=AE3_TS_YIELDED){
                                  fprintf(stderr,"Original nested kernel syscall wrappers failed\n");
                                  free(elf);free(ram);return 33;
                               }
                               AE3_TimerSysResult copied=AE3_TS_YIELDED;
                               for(unsigned tries=0;tries<8;tries++){
                                  copied=ae3_timer3_syscalls_slice(&t3sys,&bus,4000);
                                  if(copied==AE3_TS_RETURNED)break;
                                  if(copied!=AE3_TS_YIELDED)break;
                               }
                               if(copied!=AE3_TS_RETURNED||
                                  memcmp(ram+0x76000,ram+0x6a2678,0x740)!=0){
                                  fprintf(stderr,"Guest 0x5A kernel patch byte parity failed\n");
                                  free(elf);free(ram);return 34;
                               }
                               printf("EXPERIMENTAL guest syscall dispatch: original 0x5A MIPS handler copied 1856 source bytes 006A2678 -> KSEG0 80076000, then reached %08X (retired %llu).\n",
                                  t3sys.cpu.pc,(unsigned long long)t3sys.cpu.retired);
                               printf("This is a guest payload copy with compatibility-kernel dispatcher, NOT a verified BIOS exception path.\n");
                               AE3_BootPostPatch later;
                               if(ae3_boot_post_patch_enter(&later,&t3sys)!=AE3_PP_YIELDED){
                                  fprintf(stderr,"Original post-kernel-copy guest entry rejected\n");
                                  free(elf);free(ram);return 35;
                               }
                               unsigned copied40=0,flushes=0,registered=0,lookups=0;
                               AE3_PostPatchResult post_result=AE3_PP_YIELDED;
                               for(unsigned cycle=0;cycle<30000;cycle++){
                                  post_result=ae3_boot_post_patch_slice(&later,&bus,1000);
                                  if(post_result==AE3_PP_RETURNED)break;
                                  switch(post_result){
                                    case AE3_PP_YIELDED:break;
                                    case AE3_PP_COPY_REQUEST:copied40++;goto dispatch_post;
                                    case AE3_PP_FLUSH_REQUEST:flushes++;goto dispatch_post;
                                    case AE3_PP_REGISTER_REQUEST:registered++;goto dispatch_post;
                                    case AE3_PP_GET_ENTRY_REQUEST:lookups++;goto dispatch_post;
                                    default:fprintf(stderr,"Guest post-patch halted: %d at %08X\n",
                                       (int)post_result,later.cpu.pc);
                                       free(elf);free(ram);return 36;
                                  }
                                  continue;
                                  dispatch_post:
                                  if(ae3_boot_post_patch_dispatch(&later,&bus,&syscall_kernel)!=AE3_PP_YIELDED){
                                     fprintf(stderr,"Guest post-patch service unavailable at %08X\n",later.cpu.pc);
                                     free(elf);free(ram);return 37;
                                  }
                               }
                               if(post_result!=AE3_PP_RETURNED||copied40!=1||flushes!=2||
                                  registered!=7||lookups!=6||later.cpu.pc!=0x0043FE78||
                                  memcmp(ram+0x82000,ram+0x6a2db8,40)!=0){
                                  fprintf(stderr,"Original post-patch registration/copy loop mismatch\n");
                                  free(elf);free(ram);return 38;
                               }
                               printf("EXPERIMENTAL: original 40-byte Copy -> 00082000, two FlushCache requests, six original guest 0x5B lookups and seven SetSyscall registrations. Returned to %08X (retired %llu).\n",
                                     later.cpu.pc,(unsigned long long)later.cpu.retired);
                               printf("FlushCache behavior is recorded in compatibility mode, NOT authentic PS2 cache coherency.\n");
                               if(!(strcmp(argv[5],"--test-timer3-copy-alloc")&&strcmp(argv[5],"--test-timer2-mode"))){
                                  AE3_BootAllocInit alloc;
                                  if(ae3_boot_alloc_enter(&alloc,&later)!=AE3_ALLOC_YIELDED){
                                     fprintf(stderr,"Original 0x4404B0 allocator entry rejected\n");
                                     free(elf);free(ram);return 39;
                                  }
                                  unsigned fills=0;
                                  AE3_AllocResult ar=AE3_ALLOC_YIELDED;
                                  for(unsigned step=0;step<20000;step++){
                                     ar=ae3_boot_alloc_slice(&alloc,&bus,1000);
                                     if(ar==AE3_ALLOC_NEXT_CALL_PENDING||ar==AE3_ALLOC_RETURNED)break;
                                     if(ar==AE3_ALLOC_ZERO_FILL_REQUEST){
                                        if(ae3_boot_alloc_dispatch_zero(&alloc,&bus)!=AE3_ALLOC_YIELDED){
                                           fprintf(stderr,"Original native translated memset could not run\n");
                                           free(elf);free(ram);return 40;
                                        }
                                        fills++;
                                     }else if(ar!=AE3_ALLOC_YIELDED){
                                        fprintf(stderr,"Retail allocator stopped at %08X status=%d\n",alloc.cpu.pc,ar);
                                        free(elf);free(ram);return 41;
                                     }
                                  }
                                  if(ar!=AE3_ALLOC_NEXT_CALL_PENDING||fills!=1||alloc.pointer_stores!=128||
                                     alloc.freelist_stores!=66||alloc.cpu.pc!=AE3_ALLOC_NEXT_CALL){
                                     fprintf(stderr,"Original pointer chain/free list initialization mismatch\n");
                                     free(elf);free(ram);return 42;
                                  }
                                  printf("EXPERIMENTAL: original 0x4404B0 allocator cleared 8192 bytes via native-C translated MMI memset; original MIPS built 128 pointers and a 64-node free list; now at next guest JAL %08X. Total retired=%llu (translated memset not counted).\n",
                                     alloc.cpu.pc,(unsigned long long)alloc.cpu.retired);
                                  if(ae3_boot_alloc_follow_intc(&alloc,&bus)!=AE3_ALLOC_INTC_SYSCALL){
                                     fprintf(stderr,"Original interrupt setup wrapper did not reach kernel boundary\n");
                                     free(elf);free(ram);return 43;
                                  }
                                  printf("ORIGINAL AddIntcHandler syscall 0x10 pending at %08X (cause=%u handler=%08X next=%u); no BIOS response supplied. Total retired=%llu.\n",
                                     alloc.cpu.pc,(unsigned)alloc.cpu.gpr[4][0],(unsigned)alloc.cpu.gpr[5][0],
                                     (unsigned)alloc.cpu.gpr[6][0],(unsigned long long)alloc.cpu.retired);
                                  /* Opt-in compatibility testing ONLY: create a stateful guest
                                     INTC registration and run the real JR/branch-likely sequence. */
                                  AE3_EEIntcKernel intc_kernel;
                                  ae3_ee_intc_init(&intc_kernel);
                                  AE3_BootPostIntc intc_after;
                                  if(ae3_boot_post_intc_enter(&intc_after,&alloc)!=AE3_POST_INTC_YIELDED||
                                     ae3_boot_post_intc_service(&intc_after,&intc_kernel,&bus)!=AE3_POST_INTC_YIELDED||
                                     ae3_boot_post_intc_slice(&intc_after,&bus,64)!=AE3_POST_INTC_NEXT_CALL){
                                     fprintf(stderr,"Original EE AddIntcHandler guest continuation failed\n");
                                     free(elf);free(ram);return 44;
                                  }
                                  AE3_EEIntcHandler intc_entry;
                                  if(ae3_ee_intc_lookup(&intc_kernel,intc_after.handler_id,&intc_entry)!=0||
                                     intc_entry.cause!=11||intc_entry.entry!=0x00440a50u){
                                     fprintf(stderr,"EE INTC compatibility handler registry mismatch\n");
                                     free(elf);free(ram);return 45;
                                  }
                                  printf("EXPERIMENTAL AddIntcHandler model allocated ID %u: original JALR return and BGEZL now at %08X; returned ID is NOT validated against Sony BIOS.\n",
                                     intc_after.handler_id,intc_after.cpu.pc);
                                  AE3_BootIntcCritical intc_critical;
                                  if(ae3_boot_intc_critical_enter(&intc_critical,&intc_after,0)!=AE3_INTC_CRITICAL_YIELDED||
                                     ae3_boot_intc_critical_slice(&intc_critical,&bus,64)!=AE3_INTC_CRITICAL_T2_MODE_PENDING){
                                     fprintf(stderr,"Original EE COP0-disabled critical section could not reach Timer2 MODE\n");
                                     free(elf);free(ram);return 46;
                                  }
                                  uint32_t committed_id=0;
                                  if(ae3_ee_bus_read32(&bus,0x006a2e28u,&committed_id)!=AE3_EE_BUS_OK||
                                     committed_id!=intc_after.handler_id||intc_critical.cpu.pc!=0x00440590u){
                                     fprintf(stderr,"Original guest INTC registration/store mismatch\n");
                                     free(elf);free(ram);return 47;
                                  }
                                  printf("EXPERIMENTAL explicit COP0 Status=0: original EE guest wrote handler ID to 006A2E28; stopped BEFORE real Timer2 MODE read at PC %08X, MMIO %08X. Retired=%llu.\n",
                                     intc_critical.cpu.pc,AE3_T2_MODE_REGISTER,
                                     (unsigned long long)intc_critical.cpu.retired);
                                  if(!strcmp(argv[5],"--test-timer2-mode")){
                                     /* Diagnostic fixture ONLY: reset-mode Timer 2 is
                                        explicitly supplied, not inferred from BIOS. */
                                     AE3_EETimer2 timer2;
                                     ae3_ee_timer2_reset(&timer2);
                                     ae3_ee_bus_attach_mmio(&bus,ae3_ee_timer2_read,
                                         ae3_ee_timer2_write,&timer2);
                                     AE3_BootTimer2Continue t2;
                                     if(ae3_boot_timer2_enter(&t2,&intc_critical)!=AE3_BT2_YIELDED||
                                        ae3_boot_timer2_slice(&t2,&bus,32)!=AE3_BT2_TIMER_SETUP_CALL||
                                        t2.cpu.pc!=AE3_T2_GUEST_COUNT_SETUP||
                                        t2.cpu.retired!=intc_critical.cpu.retired+8){
                                        fprintf(stderr,"Original Timer 2 MODE boot continuation mismatch\n");
                                        free(elf);free(ram);return 48;
                                     }
                                     printf("EXPERIMENTAL supplied Timer2 MODE=0000: original guest LW/branch-delay path reached first SetT2_COUNT JAL %08X; retired=%llu.\n",
                                        t2.cpu.pc,(unsigned long long)t2.cpu.retired);
                                     AE3_BootTimer2Setup t2_setup;
                                     if(ae3_boot_timer2_setup_enter(&t2_setup,&t2)!=AE3_BT2S_YIELDED||
                                        ae3_boot_timer2_setup_slice(&t2_setup,&bus,512)!=AE3_BT2S_ENABLE_INTC_PENDING||
                                        t2_setup.io_stores!=3||
                                        t2_setup.cpu.pc!=AE3_T2_GUEST_NEXT_ENABLE_INTC||
                                        timer2.count!=0||timer2.compare!=0xffff||
                                        !(timer2.mode&0x80u)||t2_setup.cop0_status!=0){
                                         fprintf(stderr,"Original Timer2 hardware setup/ERET sequence mismatch\n");
                                         free(elf);free(ram);return 49;
                                     }
                                     printf("EXPERIMENTAL: original Timer2 COUNT/COMP/MODE wrappers completed, EnableIntc JAL at %08X; retired=%llu.\n",
                                        t2_setup.cpu.pc,(unsigned long long)t2_setup.cpu.retired);
                                     AE3_BootEnableIntc enable_stage;
                                     if(ae3_boot_enable_intc_enter(&enable_stage,&t2_setup)!=AE3_EINTC_YIELDED||
                                        ae3_boot_enable_intc_slice(&enable_stage,&bus,100)!=AE3_EINTC_SYSCALL_PENDING||
                                        enable_stage.cpu.pc!=AE3_ENABLE_INTC_SYSCALL||
                                        enable_stage.cpu.gpr[3][0]!=0x14u||
                                        enable_stage.cpu.gpr[4][0]!=11u){
                                         fprintf(stderr,"Original EnableIntc wrapper failed before syscall 0x14\n");
                                         free(elf);free(ram);return 50;
                                     }
                                     if(ae3_boot_enable_intc_service(&enable_stage,&bus,&intc_kernel)!=AE3_EINTC_YIELDED||
                                        enable_stage.last_return!=0||
                                        !ae3_ee_intc_is_enabled(&intc_kernel,11)||
                                        ae3_boot_enable_intc_slice(&enable_stage,&bus,100)!=AE3_EINTC_PARENT_RETURNED||
                                        enable_stage.cpu.pc!=AE3_ENABLE_INTC_PARENT_RETURN){
                                         fprintf(stderr,"Experimental EnableIntc kernel service or retail stack return failed\n");
                                         free(elf);free(ram);return 51;
                                     }
                                     printf("EXPERIMENTAL: syscall 0x14 cause 11 enabled in compatibility registry; game epilogue returned to %08X, retired=%llu. No Sony BIOS INTC_MASK modeled.\n",
                                        enable_stage.cpu.pc,(unsigned long long)enable_stage.cpu.retired);
                                     AE3_BootTimer2Recheck recheck;
                                     if(ae3_boot_timer2_recheck_enter(&recheck,&enable_stage)!=AE3_T2RC_YIELDED||
                                        ae3_boot_timer2_recheck_slice(&recheck,&bus,100)!=AE3_T2RC_MMIO_PENDING||
                                        recheck.cpu.pc!=AE3_T2_RECHECK_MMIO_LW||
                                        (uint32_t)recheck.cpu.gpr[2][0]!=AE3_T2_MODE){
                                         fprintf(stderr,"Next original initializer failed to reach second Timer2 MODE read\n");
                                         free(elf);free(ram);return 52;
                                     }
                                     printf("EXPERIMENTAL: parent original JAL 0043FE80 -> 004406F0, original CP0-disabled path reached real Timer2 MODE LW at %08X; retired=%llu. Guest MMIO read pending.\n",
                                        recheck.cpu.pc,(unsigned long long)recheck.cpu.retired);
                                  }


                               }
                            }
                            printf("Original MIPS mode branch executed with explicit hardware state, not authentic boot-time BIOS settings.\n");
                        }
                    }
                }else {
                    fprintf(stderr,"Reconciliation stopped unexpectedly: result %d PC %08X\n",stage,post.cpu.pc);
                    free(elf);free(ram);return 26;
                }
             }
             printf("  Runtime registers: s0=%016llX s1=%016llX s2=%016llX s3=%016llX s4=%016llX s5=%016llX s6=%016llX sp=%016llX\n",
               (unsigned long long)hook.cpu.gpr[16][0],(unsigned long long)hook.cpu.gpr[17][0],
               (unsigned long long)hook.cpu.gpr[18][0],(unsigned long long)hook.cpu.gpr[19][0],
               (unsigned long long)hook.cpu.gpr[20][0],(unsigned long long)hook.cpu.gpr[21][0],
               (unsigned long long)hook.cpu.gpr[22][0],(unsigned long long)hook.cpu.gpr[29][0]);
          }else{
             printf("Guest handler requires kernel memory mapping. Run --test-ee-alias after test flags to use the current EE RAM, without BIOS initialization.\n");
          }
          routed=1;break;
       }
       if(state!=AE3_HOOK_SET_SYSCALL) {
          fprintf(stderr,"Unexpected hook continuation: %d at %08X\n",state,hook.cpu.pc);
          free(elf);free(ram);return 18;
       }
    }
    if(!routed || hook.registered!=2){free(elf);free(ram);return 19;}
    printf("Retired original EE instruction count: %llu (startup kernel is compatibility/test-only).\n",(unsigned long long)hook.cpu.retired);
 }
 printf("No GS rendering, BIOS emulation, IOP or gameplay. This is a boot diagnostic only.\n");
 free(elf);free(ram);return 0;
}
