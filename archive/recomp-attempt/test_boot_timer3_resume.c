#include "ae3_boot_timer3_resume.h"
#include "ae3_ee_timer3.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t*load(const char*path,size_t*n){
 FILE*f=fopen(path,"rb");assert(f);assert(!fseek(f,0,SEEK_END));long len=ftell(f);
 assert(len>0&&!fseek(f,0,SEEK_SET));uint8_t*b=malloc((size_t)len);assert(b);
 assert(fread(b,1,(size_t)len,f)==(size_t)len);fclose(f);*n=(size_t)len;return b;
}
static uint64_t rd64(const uint8_t*r){uint64_t v=0;for(unsigned i=0;i<8;i++)v|=(uint64_t)r[i]<<(i*8);return v;}
static AE3_BootPostFind seed(void){
 AE3_BootPostFind p={0};p.done=1;p.cpu.pc=AE3_NEXT_INIT_JAL;
 p.cpu.gpr[29][0]=0x01fe0080;p.cpu.gpr[16][0]=0x123456789abcdef0ULL;
 p.cpu.gpr[17][0]=0x1223344556677889ULL;p.cpu.gpr[18][0]=0xaabbccddeeff0011ULL;
 return p;
}
static void run(int enabled,AE3_EEMemoryBus*bus,AE3_EETimer3*timer){
 assert(ae3_ee_bus_write32(bus,AE3_T3_MODE,enabled?AE3_T3_CMPE:0)==AE3_EE_BUS_OK);
 AE3_BootPostFind p=seed();AE3_BootNextInit pre;
 assert(ae3_boot_next_init_enter(&pre,&p)==AE3_NEXT_YIELDED);
 assert(ae3_boot_next_init_slice(&pre,bus,20)==AE3_NEXT_MMIO_PENDING);
 assert(pre.cpu.retired==9);
 AE3_BootTimer3Resume fast,slow;
 assert(ae3_boot_timer3_enter(&fast,&pre)==AE3_TIMER_BOOT_YIELDED);
 assert(ae3_boot_timer3_enter(&slow,&pre)==AE3_TIMER_BOOT_YIELDED);
 AE3_TimerBootResult goal=enabled?AE3_TIMER_BOOT_RETURNED:AE3_TIMER_BOOT_NEXT_CALL;
 /* A pending memory-mapped read is an atomic fault if no device is attached. */
 ae3_ee_bus_attach_mmio(bus,NULL,NULL,NULL);
 AE3_BootTimer3Resume previous=fast;
 assert(ae3_boot_timer3_slice(&fast,bus,30)==AE3_TIMER_BOOT_UNMAPPED);
 assert(!memcmp(&fast,&previous,sizeof fast));
 ae3_ee_bus_attach_mmio(bus,ae3_ee_timer3_read,ae3_ee_timer3_write,timer);
 assert(ae3_boot_timer3_slice(&fast,bus,30)==goal);
 for(unsigned i=0;i<20;i++){
   AE3_TimerBootResult z=ae3_boot_timer3_slice(&slow,bus,1);
   if(z==goal)break;
   assert(z==AE3_TIMER_BOOT_YIELDED);
 }
 assert(!memcmp(&fast,&slow,sizeof fast));
 assert(fast.cpu.retired==18); // nine original MIPS instructions after the MMIO boundary
 AE3_BootTimer3Resume before=fast;
 assert(ae3_boot_timer3_slice(&fast,bus,777)==goal);
 assert(!memcmp(&fast,&before,sizeof before));
 assert(fast.cpu.gpr[3][0]==(enabled?0x100u:0u));
 assert(fast.cpu.gpr[31][0]==AE3_TIMER3_RETURN_PC);
 if(enabled){
   assert(fast.cpu.pc==AE3_TIMER3_RETURN_PC);
   assert(fast.cpu.gpr[29][0]==0x01fe0080);
   assert(fast.cpu.gpr[16][0]==p.cpu.gpr[16][0]);
   assert(fast.cpu.gpr[17][0]==p.cpu.gpr[17][0]);
   assert(fast.cpu.gpr[18][0]==p.cpu.gpr[18][0]);
 }else{
   assert(fast.cpu.pc==AE3_TIMER3_NEXT_CALL);
   assert(fast.cpu.gpr[16][0]==0x006a2de0);
   assert(fast.cpu.gpr[18][0]==2);
   uint32_t a=0,b=0;
   assert(ae3_ee_bus_read32(bus,0x006a2de0,&a)==AE3_EE_BUS_OK);
   assert(ae3_ee_bus_read32(bus,0x006a2de4,&b)==AE3_EE_BUS_OK);
   assert(fast.cpu.gpr[4][0]==(uint64_t)(int64_t)(int32_t)a);
   assert(fast.cpu.gpr[5][0]==(uint64_t)(int64_t)(int32_t)b);
   assert(rd64(bus->ram+0x01fe0070)==0x43fe78);
 }
}
int main(int argc,char**argv){
 assert(argc==2);size_t n;uint8_t*elf=load(argv[1],&n);
 uint8_t *ram=calloc(AE3_EE_RAM_BYTES,1);assert(ram);AE3_ElfImage image;
 assert(ae3_load_ee_elf32(elf,n,ram,AE3_EE_RAM_BYTES,&image)==AE3_ELF_IMAGE_OK);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);
 AE3_EETimer3 timer;ae3_ee_timer3_reset(&timer);
 ae3_ee_bus_attach_mmio(&bus,ae3_ee_timer3_read,ae3_ee_timer3_write,&timer);
 run(0,&bus,&timer);run(1,&bus,&timer);
 puts("PASS original 0x440354 Timer3 mode branch both ways; 18 total retail instructions, exact return/next-JAL, atomic MMIO fault and 1-step parity");
 free(elf);free(ram);return 0;
}