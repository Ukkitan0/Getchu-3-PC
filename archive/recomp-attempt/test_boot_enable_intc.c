/* Deterministic original-opcode test from the retail opcode inventory.
   The test fixture is pinned from the SHA-256-identified US title and
   deliberately exercises the original branch/delay/stack execution. */
#include "ae3_boot_timer2_recheck.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM AE3_EE_RAM_BYTES
#define SP UINT32_C(0x01fe0000)
static void store64(AE3_EEMemoryBus *bus,uint32_t address,uint64_t v){
 unsigned char b[8];for(unsigned i=0;i<8;i++)b[i]=(unsigned char)(v>>(i*8));
 assert(ae3_ee_bus_write(bus,address,b,8)==AE3_EE_BUS_OK);
}
static void fill(AE3_EEMemoryBus *bus){
 FILE *f=fopen("retail_enable_intc_words.tsv","rt");assert(f);
 char line[128];unsigned count=0;
 while(fgets(line,sizeof line,f)){
  unsigned addr,word;
  if(sscanf(line,"%x %x",&addr,&word)==2){
   assert(ae3_ee_bus_write32(bus,addr,word)==AE3_EE_BUS_OK);count++;
  }
 }
 fclose(f);assert(count>=60);
}
static AE3_BootTimer2Setup seed(AE3_EEMemoryBus *bus,uint64_t saved_intc){
 AE3_BootTimer2Setup prior={0};prior.active=1;
 prior.cpu.pc=AE3_ENABLE_INTC_CALL;prior.cop0_status=0;
 prior.cpu.gpr[29][0]=SP;prior.cpu.gpr[31][0]=AE3_ENABLE_INTC_CALL;
 prior.cpu.gpr[17][0]=saved_intc;
 prior.cpu.gpr[16][0]=UINT64_C(0x11223344);
 prior.cpu.gpr[18][0]=UINT64_C(0x22334455);
 prior.cpu.gpr[19][0]=UINT64_C(0x33445566);
 prior.cpu.gpr[20][0]=UINT64_C(0x44556677);
 prior.cpu.retired=323338;
 const uint64_t original[]={
  UINT64_C(0xAAAAAAAA11110000),UINT64_C(0xBBBBBBBB22220000),
  UINT64_C(0xCCCCCCCC33330000),UINT64_C(0xDDDDDDDD44440000),
  UINT64_C(0xEEEEEEEE55550000),AE3_ENABLE_INTC_PARENT_RETURN
 };
 for(unsigned i=0;i<6;i++)store64(bus,SP+i*0x10,original[i]);
 return prior;
}
static AE3_EnableIntcResult step_until(AE3_BootEnableIntc *s,
 AE3_EEMemoryBus *bus,unsigned budget){
 AE3_EnableIntcResult r=AE3_EINTC_YIELDED;
 for(unsigned i=0;i<256;i++){
  r=ae3_boot_enable_intc_slice(s,bus,budget);
  if(r!=AE3_EINTC_YIELDED)break;
 }
 return r;
}
static void check_case(uint64_t saved,int register_handler,int success_path){
 unsigned char *mem=calloc(1,RAM);assert(mem);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,mem,RAM);fill(&bus);
 AE3_BootTimer2Setup prior=seed(&bus,saved);
 AE3_EEIntcKernel k;ae3_ee_intc_init(&k);
 if(register_handler){
  assert(ae3_ee_intc_add(&k,&bus,11,0x440a50u,0)==1);
 }
 AE3_BootEnableIntc s;
 assert(ae3_boot_enable_intc_enter(&s,&prior)==AE3_EINTC_YIELDED);
 assert(step_until(&s,&bus,1)==AE3_EINTC_SYSCALL_PENDING);
 assert(s.cpu.gpr[3][0]==0x14u&&s.cpu.gpr[4][0]==11);
 /* Failed syscall instruction fetch must not mutate CPU or kernel. */
 AE3_BootEnableIntc old=s;AE3_EEIntcKernel oldk=k;
 assert(ae3_ee_bus_write32(&bus,AE3_ENABLE_INTC_SYSCALL,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_enable_intc_service(&s,&bus,&k)==AE3_EINTC_UNSUPPORTED);
 assert(!memcmp(&s,&old,sizeof s)&&!memcmp(&k,&oldk,sizeof k));
 assert(ae3_ee_bus_write32(&bus,AE3_ENABLE_INTC_SYSCALL,0x0000000c)==AE3_EE_BUS_OK);
 assert(ae3_boot_enable_intc_service(&s,&bus,&k)==AE3_EINTC_YIELDED);
 assert(s.last_return==(register_handler?0:-1));
 assert(ae3_ee_intc_is_enabled(&k,11)==register_handler);
 assert(ae3_boot_enable_intc_service(&s,&bus,&k)==AE3_EINTC_INVALID);
 AE3_EnableIntcResult reached=step_until(&s,&bus,1);
 assert(reached==(success_path?AE3_EINTC_PARENT_RETURNED:AE3_EINTC_EI_PENDING));
 if(success_path){
  assert(s.cpu.pc==AE3_ENABLE_INTC_PARENT_RETURN&&s.cpu.gpr[29][0]==SP+0x60);
  assert(s.cpu.gpr[31][0]==AE3_ENABLE_INTC_PARENT_RETURN);
  assert(s.cpu.gpr[20][0]==UINT64_C(0xEEEEEEEE55550000));
  assert(s.cpu.gpr[2][0]==0);
  AE3_BootTimer2Recheck rc;
  assert(ae3_boot_timer2_recheck_enter(&rc,&s)==AE3_T2RC_YIELDED);
  AE3_T2RecheckResult next=AE3_T2RC_YIELDED;
  for(unsigned i=0;i<64;i++){
      next=ae3_boot_timer2_recheck_slice(&rc,&bus,1);
      if(next!=AE3_T2RC_YIELDED)break;
  }
  assert(next==AE3_T2RC_MMIO_PENDING);
  assert(rc.cpu.pc==AE3_T2_RECHECK_MMIO_LW);
  assert(rc.cpu.gpr[2][0]==UINT64_C(0x10001010));
  assert(rc.cpu.retired==s.cpu.retired+18);
  puts("PASS: original next init JAL + DIntr/CP0 bypass reached Timer2 MODE LW");
 }else assert(s.cpu.pc==AE3_ENABLE_INTC_EI_BOUNDARY);
 printf("PASS guest EnableIntc: cause=11 present=%d prior-int=%llu next=%08X guest-retired=%llu\n",
 register_handler,(unsigned long long)saved,s.cpu.pc,
 (unsigned long long)(s.cpu.retired-prior.cpu.retired));
 free(mem);
}
static void compare_batched(void){
 unsigned char *a=calloc(1,RAM),*b=calloc(1,RAM);assert(a&&b);
 AE3_EEMemoryBus busa,busb;ae3_ee_bus_init(&busa,a,RAM);
 ae3_ee_bus_init(&busb,b,RAM);fill(&busa);fill(&busb);
 AE3_BootTimer2Setup first=seed(&busa,0);(void)seed(&busb,0);
 AE3_BootEnableIntc sa,sb;
 assert(ae3_boot_enable_intc_enter(&sa,&first)==AE3_EINTC_YIELDED);
 assert(ae3_boot_enable_intc_enter(&sb,&first)==AE3_EINTC_YIELDED);
 assert(step_until(&sa,&busa,1)==AE3_EINTC_SYSCALL_PENDING);
 assert(step_until(&sb,&busb,300)==AE3_EINTC_SYSCALL_PENDING);
 assert(!memcmp(&sa,&sb,sizeof sa)&&!memcmp(a,b,RAM));
 AE3_EEIntcKernel ka,kb;ae3_ee_intc_init(&ka);ae3_ee_intc_init(&kb);
 assert(ae3_ee_intc_add(&ka,&busa,11,0x440a50u,0)==1);
 assert(ae3_ee_intc_add(&kb,&busb,11,0x440a50u,0)==1);
 assert(ae3_boot_enable_intc_service(&sa,&busa,&ka)==AE3_EINTC_YIELDED);
 assert(ae3_boot_enable_intc_service(&sb,&busb,&kb)==AE3_EINTC_YIELDED);
 assert(step_until(&sa,&busa,1)==AE3_EINTC_PARENT_RETURNED);
 assert(step_until(&sb,&busb,300)==AE3_EINTC_PARENT_RETURNED);
 assert(!memcmp(&sa,&sb,sizeof sa)&&!memcmp(&ka,&kb,sizeof ka));
 assert(!memcmp(a,b,RAM));
 free(a);free(b);
}
static void check_faults(void){
 unsigned char *a=calloc(1,RAM);assert(a);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,a,RAM);fill(&bus);
 AE3_BootTimer2Setup prior=seed(&bus,0);
 AE3_BootEnableIntc s;assert(ae3_boot_enable_intc_enter(&s,&prior)==AE3_EINTC_YIELDED);
 assert(ae3_ee_bus_write32(&bus,AE3_ENABLE_INTC_CALL,0)==AE3_EE_BUS_OK);
 AE3_BootEnableIntc old=s;
 assert(ae3_boot_enable_intc_slice(&s,&bus,1)==AE3_EINTC_UNSUPPORTED);
 assert(!memcmp(&old,&s,sizeof s));
 assert(ae3_ee_bus_write32(&bus,AE3_ENABLE_INTC_CALL,0x0c10d98a)==AE3_EE_BUS_OK);
 assert(ae3_boot_enable_intc_slice(&s,&bus,3)==AE3_EINTC_YIELDED);
 /* The next instruction is a guest SD at a currently unmapped SP location. */
 old=s;bus.ram_size=0x800000u;
 assert(ae3_boot_enable_intc_slice(&s,&bus,1)==AE3_EINTC_MEMORY_FAULT);
 assert(!memcmp(&s,&old,sizeof s));
 bus.ram_size=RAM;
 assert(step_until(&s,&bus,1)==AE3_EINTC_SYSCALL_PENDING);
 assert(s.cpu.pc==AE3_ENABLE_INTC_SYSCALL);
 prior.cop0_status=UINT32_C(0x10000);
 assert(ae3_boot_enable_intc_enter(&s,&prior)==AE3_EINTC_INVALID);
 free(a);
}
static void check_kernel_mask(void){
 uint8_t *ram=calloc(1,RAM);assert(ram);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,RAM);
 AE3_EEIntcKernel k;ae3_ee_intc_init(&k);
 assert(ae3_ee_intc_enable(&k,11)==-1);
 assert(ae3_ee_intc_enable(&k,AE3_INTC_CAUSES)==-1);
 assert(k.enabled_mask==0&&k.generation==0);
 assert(ae3_ee_intc_add(&k,&bus,11,0x00440a50u,0)==1);
 assert(ae3_ee_intc_enable(&k,11)==0);
 const uint32_t after=k.generation;
 assert(ae3_ee_intc_enable(&k,11)==0&&k.generation==after);
 assert(ae3_ee_intc_is_enabled(&k,11));
 assert(ae3_ee_intc_remove(&k,11,1)==0);
 assert(!ae3_ee_intc_is_enabled(&k,11));
 assert(ae3_ee_intc_enable(&k,11)==-1);
 free(ram);
}
int main(void){
 check_kernel_mask();
 check_case(0,1,1);
 check_case(0,0,1);
 check_case(1,1,0);
 compare_batched();check_faults();
 puts("PASS original EnableIntc wrapper, atomic guest SD/LD, opcode guard, syscall 0x14 compatibility dispatch, failure/branch paths");
 return 0;
}
