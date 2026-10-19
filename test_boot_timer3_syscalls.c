#include "ae3_boot_timer3_syscalls.h"
#include "ae3_ee_timer3.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t*load(const char*path,size_t*n){FILE*f=fopen(path,"rb");assert(f);
 assert(!fseek(f,0,SEEK_END));long len=ftell(f);assert(len>0);
 assert(!fseek(f,0,SEEK_SET));uint8_t*b=malloc((size_t)len);assert(b);
 assert(fread(b,1,(size_t)len,f)==(size_t)len);fclose(f);*n=len;return b;}
int main(int argc,char**argv){
 assert(argc==2);size_t n;uint8_t*elf=load(argv[1],&n);
 uint8_t*ram=calloc(AE3_EE_RAM_BYTES,1);assert(ram);AE3_ElfImage image;
 assert(ae3_load_ee_elf32(elf,n,ram,AE3_EE_RAM_BYTES,&image)==AE3_ELF_IMAGE_OK);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);
 AE3_EETimer3 timer;ae3_ee_timer3_reset(&timer);
 ae3_ee_bus_attach_mmio(&bus,ae3_ee_timer3_read,ae3_ee_timer3_write,&timer);
 AE3_BootPostFind post={0};post.done=1;post.cpu.pc=AE3_NEXT_INIT_JAL;
 post.cpu.gpr[29][0]=0x01fe0080;
 AE3_BootNextInit first;assert(ae3_boot_next_init_enter(&first,&post)==AE3_NEXT_YIELDED);
 assert(ae3_boot_next_init_slice(&first,&bus,40)==AE3_NEXT_MMIO_PENDING);
 AE3_BootTimer3Resume second;
 assert(ae3_boot_timer3_enter(&second,&first)==AE3_TIMER_BOOT_YIELDED);
 assert(ae3_boot_timer3_slice(&second,&bus,40)==AE3_TIMER_BOOT_NEXT_CALL);
 AE3_Timer3Syscalls stage;
 assert(ae3_timer3_syscalls_enter(&stage,&second)==AE3_TS_YIELDED);
 AE3_EESyscallKernel kernel;ae3_ee_syscall_kernel_init(&kernel);
 /* First guest JAL to wrapper; kernel registration may only be committed
    when the original function actually enters syscall 0x74. */
 assert(ae3_timer3_syscalls_slice(&stage,&bus,60)==AE3_TS_SET_SYSCALL);
 assert(stage.cpu.pc==0x4402d4&&stage.cpu.gpr[3][0]==0x74);
 assert(stage.cpu.gpr[4][0]==0x5a&&stage.cpu.gpr[5][0]==AE3_GUEST_COPY_ENTRY);
 uint32_t target=0;
 assert(ae3_ee_syscall_lookup(&kernel,0x5a,&target)!=0);
 assert(ae3_timer3_syscalls_dispatch(&stage,&bus,&kernel)==AE3_TS_YIELDED);
 assert(ae3_ee_syscall_lookup(&kernel,0x5a,&target)==0&&target==AE3_GUEST_COPY_ENTRY);
 assert(ae3_timer3_syscalls_slice(&stage,&bus,80)==AE3_TS_COPY_SYSCALL);
 assert(stage.cpu.pc==0x4402e4&&stage.cpu.gpr[3][0]==0x5a);
 assert((uint32_t)stage.cpu.gpr[4][0]==0x80076000u);
 assert(stage.cpu.gpr[5][0]==0x006a2678&&stage.cpu.gpr[6][0]==0x740);
 unsigned char reference[0x740];
 assert(ae3_ee_bus_read(&bus,0x6a2678,reference,sizeof reference)==AE3_EE_BUS_OK);
 assert(memcmp(ram+0x76000,reference,sizeof reference)!=0);
 assert(ae3_timer3_syscalls_dispatch(&stage,&bus,&kernel)==AE3_TS_YIELDED);
 assert(stage.copy_active);
 AE3_TimerSysResult r=AE3_TS_YIELDED;
 for(unsigned i=0;i<8000;i++){
   r=ae3_timer3_syscalls_slice(&stage,&bus,1);
   if(r==AE3_TS_RETURNED)break;
   assert(r==AE3_TS_YIELDED);
 }
 assert(r==AE3_TS_RETURNED);
 assert(stage.cpu.pc==0x440398&&stage.done&&!stage.copy_active);
 assert(stage.cpu.gpr[2][0]==0);
 assert(!memcmp(ram+0x76000,reference,sizeof reference));
 assert(stage.cpu.retired>3200);
 puts("PASS actual game Timer3 zero-mode -> SetSyscall 0x74 -> syscall 0x5A -> original 1,856-byte kernel RAM copy -> caller 0x440398");
 free(ram);free(elf);
}