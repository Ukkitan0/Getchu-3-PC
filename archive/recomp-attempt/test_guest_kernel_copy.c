#include "ae3_guest_kernel_copy.h"
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
int main(int argc,char**argv){
 assert(argc==2);size_t n;uint8_t*elf=load(argv[1],&n);
 uint8_t *ram=calloc(AE3_EE_RAM_BYTES,1);assert(ram);AE3_ElfImage image;
 assert(ae3_load_ee_elf32(elf,n,ram,AE3_EE_RAM_BYTES,&image)==AE3_ELF_IMAGE_OK);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);
 const uint32_t dst=0x80076000,src=0x006a2678,len=0x740,ra=0x440398;
 /* Retail data at 0x6A2DE0 determines the handler and subsequent copy setup. */
 uint32_t nsys=0,handler=0;
 assert(ae3_ee_bus_read32(&bus,0x006a2de0,&nsys)==AE3_EE_BUS_OK&&nsys==0x5a);
 assert(ae3_ee_bus_read32(&bus,0x006a2de4,&handler)==AE3_EE_BUS_OK&&handler==AE3_GUEST_COPY_ENTRY);
 uint8_t *expected=malloc(len);assert(expected);
 assert(ae3_ee_bus_read(&bus,src,expected,len)==AE3_EE_BUS_OK);
 assert(memcmp(expected,ram+(dst&0x01ffffff),len)!=0);
 AE3_GuestKernelCopy fast,slow;
 assert(ae3_guest_kernel_copy_init(&fast,dst,src,len,ra)==AE3_COPY_YIELDED);
 assert(ae3_guest_kernel_copy_init(&slow,dst,src,len,ra)==AE3_COPY_YIELDED);
 /* Test atomic unmapped-store recovery after running through first load. */
 AE3_GuestKernelCopy fault;
 assert(ae3_guest_kernel_copy_init(&fault,0x9f000000u,src,len,ra)==AE3_COPY_YIELDED);
 for(unsigned i=0;i<8;i++)assert(ae3_guest_kernel_copy_slice(&fault,&bus,1)==AE3_COPY_YIELDED);
 assert(fault.cpu.pc==0x440310);
 AE3_GuestKernelCopy before=fault;
 assert(ae3_guest_kernel_copy_slice(&fault,&bus,1)==AE3_COPY_MEMORY_FAULT);
 assert(!memcmp(&fault,&before,sizeof before));
 fault.cpu.gpr[4][0]=(uint64_t)(int64_t)(int32_t)dst;
 assert(ae3_guest_kernel_copy_slice(&fault,&bus,1)==AE3_COPY_YIELDED);
 assert(ae3_guest_kernel_copy_slice(&fast,&bus,5000)==AE3_COPY_RETURNED);
 assert(fast.cpu.pc==ra&&fast.done&&fast.cpu.gpr[2][0]==0);
 assert(fast.cpu.gpr[4][0]==(uint64_t)(int64_t)(int32_t)(dst+len));
 assert(fast.cpu.gpr[5][0]==src+len);
 assert(fast.cpu.gpr[7][0]==len/4u);
 assert(!memcmp(ram+(dst&0x01ffffff),expected,len));
 /* Independently exercise each original instruction and saved state. */
 size_t saved=0;
 assert(ae3_ee_bus_write(&bus,dst,"\0\0\0\0",4)==AE3_EE_BUS_OK);
 for(unsigned i=0;i<6000;i++){
     AE3_CopyResult r=ae3_guest_kernel_copy_slice(&slow,&bus,1);
     if(r==AE3_COPY_RETURNED){saved=i+1;break;}
     assert(r==AE3_COPY_YIELDED);
 }
 assert(saved>3000u&&saved<5000u);
 assert(!memcmp(&slow,&fast,sizeof fast));
 assert(!memcmp(ram+(dst&0x01ffffff),expected,len));
 AE3_GuestKernelCopy done=fast;
 assert(ae3_guest_kernel_copy_slice(&fast,&bus,50)==AE3_COPY_RETURNED);
 assert(!memcmp(&done,&fast,sizeof done));
 /* Zero-byte copy exercises beq/delay-slot path, should not write. */
 AE3_GuestKernelCopy zero;
 assert(ae3_guest_kernel_copy_init(&zero,dst,src,0,ra)==AE3_COPY_YIELDED);
 assert(ae3_guest_kernel_copy_slice(&zero,&bus,20)==AE3_COPY_RETURNED);
 assert(zero.cpu.gpr[7][0]==0&&zero.cpu.gpr[2][0]==0);
 assert(!memcmp(ram+(dst&0x01ffffff),expected,len));
 puts("PASS original syscall 0x5A guest MIPS kernel-copy handler; 1,856-byte real payload, KSEG0 shared RAM, delay slots, fault retry and single-step parity");
 free(expected);free(ram);free(elf);
}