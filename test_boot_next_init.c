/* Guest MIPS after address search: next real first I/O boundary. */
#include "ae3_boot_next_init.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N (32u*1024u*1024u)
static uint32_t rd(const uint8_t*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint8_t* load(const char*path,size_t*n){
 FILE*f=fopen(path,"rb");assert(f);assert(!fseek(f,0,SEEK_END));long count=ftell(f);
 assert(count>0&&!fseek(f,0,SEEK_SET));uint8_t*b=malloc((size_t)count);assert(b);
 assert(fread(b,1,(size_t)count,f)==(size_t)count);fclose(f);*n=(size_t)count;return b;
}
static AE3_BootPostFind seed(void){
 AE3_BootPostFind p={0};p.done=1;p.cpu.pc=AE3_NEXT_INIT_JAL;
 p.cpu.gpr[29][0]=0x01fe0080;
 p.cpu.gpr[16][0]=0x123456789abcdef0ULL;
 p.cpu.gpr[17][0]=0x1223344556677889ULL;
 p.cpu.gpr[18][0]=0xaabbccddeeff0011ULL;
 return p;
}
int main(int argc,char **argv){
 assert(argc==2);size_t n;uint8_t *elf=load(argv[1],&n);
 uint8_t *ram=calloc(N,1);assert(ram);AE3_ElfImage img;
 assert(ae3_load_ee_elf32(elf,n,ram,N,&img)==AE3_ELF_IMAGE_OK);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,N);
 assert(rd(ram+AE3_NEXT_INIT_JAL)==0x0c1100ceu);
 assert(rd(ram+AE3_NEXT_INIT_MMIO_PC)==0x8c430000u);
 AE3_BootPostFind post=seed();AE3_BootNextInit batch,single;
 assert(ae3_boot_next_init_enter(&batch,&post)==AE3_NEXT_YIELDED);
 assert(ae3_boot_next_init_enter(&single,&post)==AE3_NEXT_YIELDED);
 assert(ae3_boot_next_init_slice(&batch,&bus,100)==AE3_NEXT_MMIO_PENDING);
 assert(batch.cpu.pc==AE3_NEXT_INIT_MMIO_PC);
 assert(batch.cpu.gpr[2][0]==AE3_NEXT_INIT_MMIO_ADDRESS);
 assert(batch.cpu.gpr[31][0]==0x43fe78u);
 assert(batch.cpu.gpr[29][0]==0x01fe0040u);
 assert(batch.cpu.retired==9);
 assert(batch.waiting_mmio);
 uint64_t saved_ra=0;
 for(unsigned k=0;k<8;k++)saved_ra|=(uint64_t)ram[0x01fe0070+k]<<(8*k);
 assert(saved_ra==0x43fe78u);
 for(unsigned k=0;k<30;k++){
    AE3_NextInitResult r=ae3_boot_next_init_slice(&single,&bus,1);
    if(r==AE3_NEXT_MMIO_PENDING)break;
    assert(r==AE3_NEXT_YIELDED);
 }
 assert(!memcmp(&single,&batch,sizeof batch));
 AE3_BootNextInit prior=batch;
 assert(ae3_boot_next_init_slice(&batch,&bus,999)==AE3_NEXT_MMIO_PENDING);
 assert(!memcmp(&prior,&batch,sizeof prior));
 /* The source actually accesses hardware outside mapped 32MiB RAM. */
 uint32_t fake;
 assert(ae3_ee_bus_read32(&bus,AE3_NEXT_INIT_MMIO_ADDRESS,&fake)==AE3_EE_BUS_UNMAPPED);
 /* The first stack store must not commit when its alignment is invalid. */
 AE3_BootPostFind fault_post=seed();fault_post.cpu.gpr[29][0]++;
 AE3_BootNextInit fault;assert(ae3_boot_next_init_enter(&fault,&fault_post)==AE3_NEXT_YIELDED);
 for(unsigned k=0;k<4;k++)assert(ae3_boot_next_init_slice(&fault,&bus,1)==AE3_NEXT_YIELDED);
 assert(fault.cpu.pc==0x440340u);
 AE3_BootNextInit before=fault;
 assert(ae3_boot_next_init_slice(&fault,&bus,1)==AE3_NEXT_MEMORY_FAULT);
 assert(!memcmp(&before,&fault,sizeof before));
 fault.cpu.gpr[29][0]--;
 assert(ae3_boot_next_init_slice(&fault,&bus,1)==AE3_NEXT_YIELDED);
 puts("PASS original 0x43FE70 -> 0x440338 guest prologue, exact MMIO 0x10001810 pause, atomic stack faults and 1-step parity");
 free(ram);free(elf);return 0;
}