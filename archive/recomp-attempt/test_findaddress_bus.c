#include "ae3_guest_find_address.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N (32u*1024u*1024u)
static uint8_t *read_file(const char *p, size_t *len){
 FILE *f=fopen(p,"rb");assert(f);assert(fseek(f,0,SEEK_END)==0);
 long n=ftell(f);assert(n>0);assert(fseek(f,0,SEEK_SET)==0);
 uint8_t *data=malloc((size_t)n);assert(data);assert(fread(data,1,(size_t)n,f)==(size_t)n);
 fclose(f);*len=(size_t)n;return data;
}
static AE3_GuestFindAddress started(unsigned which) {
 AE3_BootHookInit hook={0}; hook.guest_pending=1;
 hook.cpu.pc=0x43fd04;hook.registered=2;hook.invoked_handler_pc=0x43fcc0;
 hook.cpu.gpr[31][0]=which?0x43fdb8:0x43fda4;
 hook.cpu.gpr[4][0]=0xffffffff80000000ULL;
 hook.cpu.gpr[5][0]=0xffffffff80080000ULL;
 hook.cpu.gpr[6][0]=which?0x43fc88:0x43fcc0;
 AE3_GuestFindAddress find;
 assert(ae3_guest_find_begin(&find,&hook)==AE3_FIND_YIELDED);
 return find;
}
int main(int argc,char **argv){
 assert(argc==2||argc==3);
 size_t elf_size;uint8_t *elf=read_file(argv[1],&elf_size);
 uint8_t *ram=calloc(N,1);assert(ram);
 AE3_ElfImage image;assert(ae3_load_ee_elf32(elf,elf_size,ram,N,&image)==AE3_ELF_IMAGE_OK);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,N);
 /* The real guest MIPS handler consumes the KSEG0 alias of *main RAM*. */
 ram[0x0c]=0xc0;ram[0x0d]=0xfc;ram[0x0e]=0x43;
 ram[0x140]=0x88;ram[0x141]=0xfc;ram[0x142]=0x43;
 for(unsigned which=0;which<2;which++) {
   AE3_GuestFindAddress same=started(which),prior=same,linear=started(which);
   assert(ae3_guest_find_slice_bus(&same,&bus,1)==AE3_FIND_YIELDED);
   assert(ae3_guest_find_slice_bus(&same,&bus,4000)==AE3_FIND_RETURNED);
   assert(same.matched_guest_address==(which?0x80000140u:0x8000000cu));
   assert(ae3_guest_find_slice(&linear,ram,N,ram,0x80000000,0x80000,4000)==AE3_FIND_RETURNED);
   assert(!memcmp(&linear,&same,sizeof same));
   AE3_GuestFindAddress single=prior;
   AE3_FindResult status=AE3_FIND_YIELDED;
   for(unsigned t=0;t<4000;t++){
     status=ae3_guest_find_slice_bus(&single,&bus,1);
     if(status!=AE3_FIND_YIELDED)break;
   }
   assert(status==AE3_FIND_RETURNED && !memcmp(&same,&single,sizeof same));
 }
 /* In a clean original ELF load there is no BIOS kernel initialization:
    the first MiB is zero. This MUST NOT resolve real BIOS syscall hooks. */
 ram[0x0c]=ram[0x0d]=ram[0x0e]=0;
 ram[0x140]=ram[0x141]=ram[0x142]=0;
 for(unsigned which=0;which<2;which++){
    AE3_GuestFindAddress clean=started(which);
    AE3_FindResult r=ae3_guest_find_slice_bus(&clean,&bus,2000000);
    assert(r==AE3_FIND_RETURNED);
    assert(clean.matched_guest_address==0u);
 }
 /* Restore physically unmapped memory, verifying retry semantics. */
 ram[0]=0;ram[1]=0;ram[2]=0;ram[3]=0;
 AE3_GuestFindAddress fault=started(0),old=fault;
 bus.ram_size=0x00200000;
 assert(ae3_guest_find_slice_bus(&fault,&bus,100)==AE3_FIND_INVALID); /* incomplete guest EE RAM */
 assert(!memcmp(&fault,&old,sizeof old));
 bus.ram_size=N;
 /* Use a KSEG0 read that has no physical EE RAM behind it. */
 fault.cpu.gpr[4][0]=0xffffffff82000000ULL;
 old=fault;
 assert(ae3_guest_find_slice_bus(&fault,&bus,10)==AE3_FIND_KERNEL_UNMAPPED);
 assert(!memcmp(&fault,&old,sizeof old));
 if(argc==3) {
   size_t snapshot_size;uint8_t *snapshot=read_file(argv[2],&snapshot_size);
   assert(snapshot_size==N);memcpy(ram,snapshot,N);free(snapshot);
   for(unsigned which=0;which<2;which++) {
     AE3_GuestFindAddress actual=started(which),legacy=started(which);
     AE3_FindResult a=ae3_guest_find_slice_bus(&actual,&bus,2000000);
     AE3_FindResult b=ae3_guest_find_slice(&legacy,ram,N,ram,0x80000000,N,2000000);
     assert(a==AE3_FIND_RETURNED && b==AE3_FIND_RETURNED);
     assert(!memcmp(&actual,&legacy,sizeof actual));
     assert(actual.matched_guest_address==(which?0u:0x80014f0cu));
     printf("PASS captured post-boot EE physical RAM, syscall target %08X found at %08X after %u reads (NOT boot-time BIOS state)\n",
        which?0x43fc88u:0x43fcc0u,actual.matched_guest_address,actual.loads);
   }
 }
 free(ram);free(elf);
 puts("PASS real FindAddress MIPS handler against KSEG0 aliased EE physical RAM and atomic unmapped faults");
 return 0;
}