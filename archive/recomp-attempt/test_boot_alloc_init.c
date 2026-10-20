/* Retail AE3 allocator initialization path: original CPU words, memory and
 * delay slots. The one identified optimized MMI memset is native C-translated,
 * not instruction-interpreted; its exact 8192-byte result is checked. */
#include "ae3_boot_alloc_init.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t *load(const char *p,size_t *n){
 FILE *f=fopen(p,"rb");assert(f);assert(!fseek(f,0,SEEK_END));
 long size=ftell(f);assert(size>0);assert(!fseek(f,0,SEEK_SET));
 uint8_t *b=malloc((size_t)size);assert(b);assert(fread(b,1,(size_t)size,f)==(size_t)size);
 fclose(f);*n=(size_t)size;return b;
}
static void setup(AE3_EEMemoryBus *bus,AE3_BootPostPatch *prior,
    uint8_t *ram,const uint8_t *elf,size_t n,int cold){
 AE3_ElfImage img;
 assert(ae3_load_ee_elf32(elf,n,ram,AE3_EE_RAM_BYTES,&img)==AE3_ELF_IMAGE_OK);
 ae3_ee_bus_init(bus,ram,AE3_EE_RAM_BYTES);
 if(cold)assert(ae3_ee_bus_write32(bus,0x006a2e28,0)==AE3_EE_BUS_OK);
 memset(ram+AE3_ALLOC_AREA,0xA5,AE3_ALLOC_CLEAR_BYTES);
 *prior=(AE3_BootPostPatch){0};prior->done=1;prior->cpu.pc=AE3_ALLOC_PARENT_JAL;
 prior->cpu.gpr[29][0]=0x01fe0080;
 prior->cpu.gpr[16][0]=0x11223344;
 prior->cpu.gpr[17][0]=0x55667788;
 prior->cpu.gpr[18][0]=0x99aa;
 prior->cpu.gpr[19][0]=0xbbcc;
 prior->cpu.gpr[20][0]=0xddee;
 prior->cpu.gpr[16][1]=0x123456789abcdef0ULL;
}
static void verify_positive(AE3_BootAllocInit *s, AE3_EEMemoryBus *bus){
 assert(s->cpu.pc==AE3_ALLOC_NEXT_CALL && s->cpu.in_delay_slot==0);
 assert(s->pointer_stores==128 && s->freelist_stores==66);
 assert(s->translated_zero_fills==1 && s->initialized_path);
 assert(s->cpu.gpr[31][0]==0x00440548u);
 assert(s->cpu.gpr[16][1]==0x123456789abcdef0ULL);
 uint8_t expected[AE3_ALLOC_CLEAR_BYTES]={0};
 for(uint32_t j=0;j<128u;j++){
   const uint32_t next=(j==127u)?0u:AE3_ALLOC_AREA+(j+1u)*64u;
   for(unsigned k=0;k<4;k++)expected[j*64u+k]=(uint8_t)(next>>(k*8u));
 }
 assert(!memcmp(bus->ram+AE3_ALLOC_AREA,expected,sizeof expected));
 uint32_t v=0;
 assert(ae3_ee_bus_read32(bus,0x006a2e30,&v)==AE3_EE_BUS_OK && v==0);
 assert(ae3_ee_bus_read32(bus,0x006a2e34,&v)==AE3_EE_BUS_OK && v==AE3_ALLOC_AREA);
 assert(ae3_ee_bus_read32(bus,0x0076ee80,&v)==AE3_EE_BUS_OK && v==0x0076eec0);
 assert(ae3_ee_bus_read32(bus,0x00770e40,&v)==AE3_EE_BUS_OK && v==0);
 assert(ae3_ee_bus_read32(bus,0x00770e80,&v)==AE3_EE_BUS_OK && v==0x00770e90);
 assert(ae3_ee_bus_read32(bus,0x00771270,&v)==AE3_EE_BUS_OK && v==0);
 assert(ae3_ee_bus_read32(bus,0x00771280,&v)==AE3_EE_BUS_OK && v==0x00770e80);
 assert(ae3_ee_bus_read32(bus,0x00771270,&v)==AE3_EE_BUS_OK && v==0);
}
static void run(AE3_BootAllocInit *s, AE3_EEMemoryBus *bus,unsigned budget,int cold){
 AE3_AllocResult result=AE3_ALLOC_YIELDED;
 for(unsigned k=0;k<15000;k++){
  result=ae3_boot_alloc_slice(s,bus,budget);
  if(result==AE3_ALLOC_ZERO_FILL_REQUEST){
    assert(!cold);
    assert(s->cpu.pc==AE3_ALLOC_MEMSET_ENTRY);
    assert(ae3_boot_alloc_dispatch_zero(s,bus)==AE3_ALLOC_YIELDED);
  }else if(result==AE3_ALLOC_NEXT_CALL_PENDING||result==AE3_ALLOC_RETURNED)break;
  else assert(result==AE3_ALLOC_YIELDED);
 }
 if(cold){
   assert(result==AE3_ALLOC_RETURNED && s->cpu.pc==AE3_ALLOC_PARENT_RETURN);
   assert(!s->initialized_path && s->translated_zero_fills==0 && s->done);
   assert((uint32_t)s->cpu.gpr[2][0]==0x80008001);
   assert(s->cpu.gpr[16][0]==0x11223344);
   assert(s->cpu.gpr[17][0]==0x55667788);
 }else{assert(result==AE3_ALLOC_NEXT_CALL_PENDING);verify_positive(s,bus);}
}
int main(int argc,char **argv){
 assert(argc==2);
 size_t n=0;uint8_t *elf=load(argv[1],&n);
 uint8_t *a=calloc(AE3_EE_RAM_BYTES,1),*b=calloc(AE3_EE_RAM_BYTES,1);
 assert(a&&b);
 AE3_EEMemoryBus ba,bb; AE3_BootPostPatch priora,priorb;
 setup(&ba,&priora,a,elf,n,0);setup(&bb,&priorb,b,elf,n,0);
 AE3_BootAllocInit one,many;
 assert(ae3_boot_alloc_enter(&one,&priora)==AE3_ALLOC_YIELDED);
 assert(ae3_boot_alloc_enter(&many,&priorb)==AE3_ALLOC_YIELDED);
 /* Atomic retry of an unmapped stack store at the original prologue. */
 ba.ram_size=0x01000000u;
 for(unsigned i=0;i<3;i++)assert(ae3_boot_alloc_slice(&one,&ba,1)==AE3_ALLOC_YIELDED);
 AE3_BootAllocInit before=one;
 assert(ae3_boot_alloc_slice(&one,&ba,1)==AE3_ALLOC_MEMORY_FAULT);
 assert(!memcmp(&one,&before,sizeof(one)));
 ba.ram_size=AE3_EE_RAM_BYTES;
 /* The original instructions have retired, but the test-only helper itself
    may not touch RAM until a valid full-span mapping is available. */
 for(unsigned i=0;i<100;i++){
    AE3_AllocResult r=ae3_boot_alloc_slice(&one,&ba,1);
    if(r==AE3_ALLOC_ZERO_FILL_REQUEST)break;
    assert(r==AE3_ALLOC_YIELDED);
 }
 assert(one.cpu.pc==AE3_ALLOC_MEMSET_ENTRY);
 unsigned char old[AE3_ALLOC_CLEAR_BYTES];memcpy(old,a+AE3_ALLOC_AREA,sizeof(old));
 ba.ram_size=AE3_ALLOC_AREA+128u;
 before=one;
 assert(ae3_boot_alloc_dispatch_zero(&one,&ba)==AE3_ALLOC_MEMORY_FAULT);
 assert(!memcmp(&one,&before,sizeof(one)));
 assert(!memcmp(old,a+AE3_ALLOC_AREA,sizeof(old)));
 ba.ram_size=AE3_EE_RAM_BYTES;
 assert(ae3_boot_alloc_dispatch_zero(&one,&ba)==AE3_ALLOC_YIELDED);
 run(&one,&ba,1,0);
 run(&many,&bb,2000,0);
 assert(!memcmp(&one,&many,sizeof(one)));
 assert(!memcmp(a,b,AE3_EE_RAM_BYTES));
 /* Follow the exact retail JAL/delay-slot/addiu to AddIntcHandler (0x10).
    Never dispatch that BIOS syscall or invent an interrupt ID. */
 assert(one.cpu.gpr[4][0]==11 && one.cpu.gpr[5][0]==0x440a50);
 /* If any original wrapper word changes, reject without partial register
    writes. This is also a code-image integrity guard for this bounded lift. */
 AE3_BootAllocInit pre_intc=one;
 assert(ae3_ee_bus_write32(&ba,AE3_ALLOC_INTC_SYSCALL_PC,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_alloc_follow_intc(&one,&ba)==AE3_ALLOC_UNSUPPORTED);
 assert(!memcmp(&one,&pre_intc,sizeof(one)));
 assert(ae3_ee_bus_write32(&ba,AE3_ALLOC_INTC_SYSCALL_PC,0x0000000c)==AE3_EE_BUS_OK);
 assert(ae3_boot_alloc_follow_intc(&one,&ba)==AE3_ALLOC_INTC_SYSCALL);
 assert(ae3_boot_alloc_follow_intc(&many,&bb)==AE3_ALLOC_INTC_SYSCALL);
 assert(one.cpu.pc==AE3_ALLOC_INTC_SYSCALL_PC && one.cpu.gpr[3][0]==16);
 assert(one.cpu.gpr[7][0]==0 && one.cpu.gpr[31][0]==0x440560);
 assert(!memcmp(&one,&many,sizeof(one)));
 assert(ae3_boot_alloc_follow_intc(&one,&ba)==AE3_ALLOC_INTC_SYSCALL);

 /* Alternate RETAIL branch when the initialized sentinel is nonnegative. */
 setup(&ba,&priora,a,elf,n,1);
 assert(ae3_boot_alloc_enter(&one,&priora)==AE3_ALLOC_YIELDED);
 run(&one,&ba,1,1);
 for(size_t j=0;j<AE3_ALLOC_CLEAR_BYTES;j++)assert(a[AE3_ALLOC_AREA+j]==0xA5);
 free(elf);free(a);free(b);
 puts("PASS: retail 0x4404B0 allocator: 8192-byte translated zero fill, 128 original pointer stores, 64-node original free list, alternate return branch, atomic faults, batched equivalence and AddIntcHandler syscall 0x10 boundary");
 return 0;
}