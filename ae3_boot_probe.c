/* Real retail boot probe; NEVER a playable game. No BIOS assumptions by default.
 * --test-kernel-fixture uses an explicitly synthetic stack pointer solely to
 * validate guest userland instructions beyond SetupThread/SetupHeap. */
#include "ae3_elf_image.h"
#include "ae3_boot_init_prefix.h"
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
 if(argc<2||argc>3||(argc==3&&strcmp(argv[2],"--test-kernel-fixture"))) {
  fprintf(stderr,"Usage: %s SCUS_975.01 [--test-kernel-fixture]\n",argv[0]);return 2;
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
 printf("TEST MODE: synthetic 0x01FFFFF0 kernel stack return -- NOT the verified BIOS value.\n");
 if(ae3_boot_kernel_return_thread(&k,0x01fffff0)!=AE3_KBOOT_YIELDED ||
    ae3_boot_kernel_slice(&k,ram,RAM_SIZE,20)!=AE3_KBOOT_SETUP_HEAP){fprintf(stderr,"Fixture SetupThread follow-up failed\n");free(elf);free(ram);return 8;}
 printf("Kernel service pending: SetupHeap at %08X start=%08X size=%08X\n",k.request.pc,k.request.arguments[0],k.request.arguments[1]);
 if(ae3_boot_kernel_ack_heap(&k)!=AE3_KBOOT_YIELDED||ae3_boot_kernel_slice(&k,ram,RAM_SIZE,20)!=AE3_KBOOT_NEXT_CALL){fprintf(stderr,"Fixture SetupHeap follow-up failed\n");free(elf);free(ram);return 9;}
 AE3_BootInitPrefix init;
 if(ae3_boot_init_enter(&init,&k)!=AE3_INIT_YIELDED||ae3_boot_init_slice(&init,ram,RAM_SIZE,10)!=AE3_INIT_SUBCALL){fprintf(stderr,"First init prologue failed\n");free(elf);free(ram);return 10;}
 printf("Original guest first initialization entered at %08X and next real nested call at %08X -> %08X\n",AE3_KERNEL_NEXT_CALL_TARGET,init.cpu.pc,AE3_INIT_FIRST_SUBCALL_TARGET);
 printf("No GS rendering, BIOS emulation, IOP or gameplay. This is a boot diagnostic only.\n");
 free(elf);free(ram);return 0;
}