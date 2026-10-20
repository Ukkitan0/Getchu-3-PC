#include "ae3_boot_intc_critical.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t *read_elf(const char *p,size_t *size){
 FILE *f=fopen(p,"rb");assert(f);assert(fseek(f,0,SEEK_END)==0);
 long n=ftell(f);assert(n>0);assert(fseek(f,0,SEEK_SET)==0);
 uint8_t *buf=malloc((size_t)n);assert(buf);
 assert(fread(buf,1,(size_t)n,f)==(size_t)n);fclose(f);*size=(size_t)n;return buf;
}
static void make_fixture(AE3_EEMemoryBus *bus,AE3_BootAllocInit *alloc,
 uint8_t *ram,const uint8_t *elf,size_t size){
 AE3_ElfImage img;
 assert(ae3_load_ee_elf32(elf,size,ram,AE3_EE_RAM_BYTES,&img)==AE3_ELF_IMAGE_OK);
 ae3_ee_bus_init(bus,ram,AE3_EE_RAM_BYTES);
 AE3_BootPostPatch prior={0};prior.done=1;prior.cpu.pc=AE3_ALLOC_PARENT_JAL;
 prior.cpu.gpr[29][0]=0x01fe0080;
 assert(ae3_boot_alloc_enter(alloc,&prior)==AE3_ALLOC_YIELDED);
 int ok=0;
 for(unsigned i=0;i<5000;i++){
   AE3_AllocResult a=ae3_boot_alloc_slice(alloc,bus,2000);
   if(a==AE3_ALLOC_ZERO_FILL_REQUEST){assert(ae3_boot_alloc_dispatch_zero(alloc,bus)==AE3_ALLOC_YIELDED);}
   else if(a==AE3_ALLOC_NEXT_CALL_PENDING){ok=1;break;}
   else assert(a==AE3_ALLOC_YIELDED);
 }
 assert(ok);
 assert(ae3_boot_alloc_follow_intc(alloc,bus)==AE3_ALLOC_INTC_SYSCALL);
}
static AE3_PostIntcResult run(AE3_BootPostIntc *s,const AE3_EEMemoryBus *bus,unsigned budget){
 AE3_PostIntcResult a=AE3_POST_INTC_YIELDED;
 for(unsigned i=0;i<100;i++){
   a=ae3_boot_post_intc_slice(s,bus,budget);
   if(a!=AE3_POST_INTC_YIELDED)return a;
 }
 assert(!"post_intc instruction budget exceeded");return a;
}
int main(int argc,char **argv){
 assert(argc==2);
 size_t n=0;uint8_t *elf=read_elf(argv[1],&n);
 uint8_t *ram_a=calloc(1,AE3_EE_RAM_BYTES),*ram_b=calloc(1,AE3_EE_RAM_BYTES);
 assert(ram_a&&ram_b);
 AE3_EEMemoryBus bus_a,bus_b;AE3_BootAllocInit alloc_a,alloc_b;
 make_fixture(&bus_a,&alloc_a,ram_a,elf,n);
 make_fixture(&bus_b,&alloc_b,ram_b,elf,n);
 AE3_BootPostIntc a,b;
 assert(ae3_boot_post_intc_enter(&a,&alloc_a)==AE3_POST_INTC_YIELDED);
 assert(ae3_boot_post_intc_enter(&b,&alloc_b)==AE3_POST_INTC_YIELDED);
 assert(!memcmp(&a,&b,sizeof a));
 AE3_EEIntcKernel ka,kb;ae3_ee_intc_init(&ka);ae3_ee_intc_init(&kb);
 assert(ka.next_id==1&&ka.generation==0);
 /* A failed opcode fetch must leave the guest CPU AND kernel untouched. */
 AE3_BootPostIntc before=a;AE3_EEIntcKernel old=ka;
 bus_a.ram_size=0x00400000u;
 assert(ae3_boot_post_intc_service(&a,&ka,&bus_a)==AE3_POST_INTC_MEMORY_FAULT);
 assert(!memcmp(&a,&before,sizeof(a))&&!memcmp(&ka,&old,sizeof(ka)));
 bus_a.ram_size=AE3_EE_RAM_BYTES;
 assert(ae3_boot_post_intc_service(&a,&ka,&bus_a)==AE3_POST_INTC_YIELDED);
 assert(ae3_boot_post_intc_service(&b,&kb,&bus_b)==AE3_POST_INTC_YIELDED);
 assert(!memcmp(&a,&b,sizeof a)&&!memcmp(&ka,&kb,sizeof ka));
 assert(a.handler_id==1&&a.cpu.pc==0x004357d8u&&a.cpu.retired==before.cpu.retired+1);
 AE3_EEIntcHandler got={0};
 assert(ae3_ee_intc_lookup(&ka,1,&got)==0);
 assert(got.id==1&&got.cause==11&&got.entry==0x00440a50u&&got.next==0);
 assert(run(&a,&bus_a,1)==AE3_POST_INTC_NEXT_CALL);
 assert(run(&b,&bus_b,100)==AE3_POST_INTC_NEXT_CALL);
 assert(!memcmp(&a,&b,sizeof a));
 assert(!memcmp(ram_a,ram_b,AE3_EE_RAM_BYTES));
 assert(a.cpu.pc==0x00440578&&a.cpu.in_delay_slot==0);
 assert(a.cpu.gpr[3][0]==1);
 assert((uint32_t)a.cpu.gpr[2][0]==0x006a2e20u);
 /* All 6 guest instructions from syscall through success branch-likely. */
 assert(a.cpu.retired==before.cpu.retired+6u);
 /* Execute the real JAL to the COP0 status helper with an EXPLICIT zero-EIE
    guest CP0 snapshot, stop at the next unresolved physical timer register. */
 AE3_BootIntcCritical critical_a,critical_b;
 assert(ae3_boot_intc_critical_enter(&critical_a,&a,0x10000u)==AE3_INTC_CRITICAL_UNSUPPORTED);
 assert(ae3_boot_intc_critical_enter(&critical_a,&a,0)==AE3_INTC_CRITICAL_YIELDED);
 assert(ae3_boot_intc_critical_enter(&critical_b,&b,0)==AE3_INTC_CRITICAL_YIELDED);
 /* Guest branch-delay slot SW must not mutate CPU or RAM on store fault. */
 assert(ae3_boot_intc_critical_slice(&critical_a,&bus_a,1)==AE3_INTC_CRITICAL_YIELDED);
 AE3_BootIntcCritical cbefore=critical_a;
 uint32_t old_id=0;
 assert(ae3_ee_bus_read32(&bus_a,0x006a2e28u,&old_id)==AE3_EE_BUS_OK);
 bus_a.ram_size=0x00500000u;
 assert(ae3_boot_intc_critical_slice(&critical_a,&bus_a,1)==AE3_INTC_CRITICAL_MEMORY_FAULT);
 assert(!memcmp(&critical_a,&cbefore,sizeof critical_a));
 bus_a.ram_size=AE3_EE_RAM_BYTES;
 uint32_t still=0;
 assert(ae3_ee_bus_read32(&bus_a,0x006a2e28u,&still)==AE3_EE_BUS_OK&&still==old_id);
 AE3_IntcCriticalResult ca=AE3_INTC_CRITICAL_YIELDED;
 for(unsigned k=0;k<100;k++){
  ca=ae3_boot_intc_critical_slice(&critical_a,&bus_a,1);
  if(ca!=AE3_INTC_CRITICAL_YIELDED)break;
 }
 assert(ca==AE3_INTC_CRITICAL_T2_MODE_PENDING);
 assert(ae3_boot_intc_critical_slice(&critical_b,&bus_b,100)==AE3_INTC_CRITICAL_T2_MODE_PENDING);
 assert(!memcmp(&critical_a,&critical_b,sizeof critical_a));
 assert(!memcmp(ram_a,ram_b,AE3_EE_RAM_BYTES));
 uint32_t got_id=0;
 assert(ae3_ee_bus_read32(&bus_a,0x006a2e28u,&got_id)==AE3_EE_BUS_OK && got_id==1);
 assert(critical_a.cpu.pc==0x00440590u && (uint32_t)critical_a.cpu.gpr[3][0]==AE3_T2_MODE_REGISTER);
 assert(critical_a.cpu.gpr[17][0]==0 && critical_a.cpu.retired==a.cpu.retired+14u);
 /* A real mapped instruction overwrite is caught before state mutation. */
 b=before;b.serviced=1;b.cpu.pc=0x004357d8u;
 assert(ae3_ee_bus_write32(&bus_b,0x004357d8u,0)==AE3_EE_BUS_OK);
 AE3_BootPostIntc snapshot=b;
 assert(ae3_boot_post_intc_slice(&b,&bus_b,1)==AE3_POST_INTC_UNSUPPORTED);
 assert(!memcmp(&b,&snapshot,sizeof(b)));
 assert(ae3_ee_bus_write32(&bus_b,0x004357d8u,0x03e00008u)==AE3_EE_BUS_OK);
 /* An explicit TEST-ONLY kernel policy failure follows the real negative
    bgezl path and annuls the guest success delay-slot entirely. */
 AE3_BootPostIntc fail;AE3_BootAllocInit fail_alloc=alloc_a;
 assert(ae3_boot_post_intc_enter(&fail,&fail_alloc)==AE3_POST_INTC_YIELDED);
 AE3_EEIntcKernel bad;ae3_ee_intc_init(&bad);bad.next_id=0;
 assert(ae3_boot_post_intc_service(&fail,&bad,&bus_a)==AE3_POST_INTC_YIELDED);
 assert(fail.handler_id==0&&bad.generation==0);
 assert(run(&fail,&bus_a,1)==AE3_POST_INTC_FAILED_PATH);
 assert(fail.cpu.pc==0x004405e4u&&fail.cpu.gpr[3][0]==UINT64_MAX);
 assert((uint32_t)fail.cpu.gpr[2][0]==0x80009021u);
 assert(fail.cpu.in_delay_slot==0);
 assert(ae3_ee_intc_remove(&ka,11,1)==0);
 assert(ae3_ee_intc_lookup(&ka,1,&got)==-1);
 /* Reject invalid guest handler addresses and causes without changing table. */
 old=ka;
 assert(ae3_ee_intc_add(&ka,&bus_a,16,0x00440a50u,0)==-1);
 assert(ae3_ee_intc_add(&ka,&bus_a,11,0x00440a51u,0)==-1);
 assert(ae3_ee_intc_add(&ka,&bus_a,11,0x1f000000u,0)==-1);
 assert(!memcmp(&ka,&old,sizeof ka));
 free(elf);free(ram_a);free(ram_b);
 puts("PASS: retail AddIntcHandler original syscall/wrapper/bgezl paths; modeled EE handler IDs; atomic faults; single vs batched equivalence; negative return and delay annulment");
 return 0;
}