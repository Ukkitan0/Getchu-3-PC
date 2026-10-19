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
 if(argc<2||argc>5||(argc>=3&&strcmp(argv[2],"--test-kernel-fixture")&&!model_mode)||
    (argc>=4&&strcmp(argv[3],"--test-sema-hle"))||
    (argc==5&&strcmp(argv[4],"--test-ee-alias"))) {
  fprintf(stderr,"Usage: %s SCUS_975.01 [--test-kernel-fixture|--test-ee-boot-model [--test-sema-hle [--test-ee-alias]]]\n",argv[0]);return 2;
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
          if(argc==5) {
             printf("TEST ONLY: running original 0x83 MIPS handler with KSEG0 aliased to current EE RAM.\n");
             printf("  BIOS/kernel was not initialized: no authentic startup kernel bytes are claimed.\n");
             AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,RAM_SIZE);
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