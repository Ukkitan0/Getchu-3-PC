#include "ae3_boot_event_create.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM AE3_EE_RAM_BYTES
#define SP UINT32_C(0x01FDFE00)
static void map(AE3_EEMemoryBus *b){
 FILE *f=fopen("retail_event_init_words.tsv","r");assert(f);
 char line[128];unsigned a,w,n=0;
 while(fgets(line,sizeof line,f))if(sscanf(line,"%x %x",&a,&w)==2){
   assert(ae3_ee_bus_write32(b,a,w)==AE3_EE_BUS_OK);n++;
 }
 fclose(f);assert(n>=28);
}
static AE3_BootTimer2Finish seed(AE3_EEMemoryBus *b,uint32_t flag){
 AE3_BootTimer2Finish t={0};t.entered=1;t.cpu.pc=AE3_T2_FINISH_PARENT_NEXT;
 t.cpu.gpr[29][0]=SP;t.cpu.gpr[16][0]=UINT64_C(0x123456789ABCDEF0);
 t.cpu.gpr[17][0]=UINT64_C(0x2222222233333333);
 t.cpu.gpr[18][0]=UINT64_C(0x4444444455555555);
 t.cpu.gpr[31][0]=0x0043FE80;
 t.cpu.retired=323403;
 assert(ae3_ee_bus_write32(b,AE3_EVENT_GUEST_STATE,flag)==AE3_EE_BUS_OK);
 return t;
}
static void check(uint32_t flag){
 uint8_t *ra=calloc(1,RAM),*rb=calloc(1,RAM);assert(ra&&rb);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,ra,RAM);ae3_ee_bus_init(&b,rb,RAM);
 map(&a);map(&b);
 AE3_BootTimer2Finish prior=seed(&a,flag);(void)seed(&b,flag);
 AE3_BootEventInit sa,sb;
 assert(ae3_boot_event_enter(&sa,&prior)==AE3_EVT_YIELDED);
 assert(ae3_boot_event_enter(&sb,&prior)==AE3_EVT_YIELDED);
 AE3_EventInitResult result=AE3_EVT_YIELDED;
 for(unsigned i=0;i<100;i++){
  result=ae3_boot_event_slice(&sa,&a,1);
  if(result!=AE3_EVT_YIELDED)break;
 }
 assert(result==(flag>0&&(int32_t)flag>0?AE3_EVT_RETURNED:AE3_EVT_SYSCALL_PENDING));
 assert(ae3_boot_event_slice(&sb,&b,100)==result);
 assert(!memcmp(&sa,&sb,sizeof sa));assert(!memcmp(ra,rb,RAM));
 assert(sa.observed_global==flag);
 if(result==AE3_EVT_SYSCALL_PENDING){
  assert(sa.cpu.pc==AE3_EVENT_GUEST_SYSCALL&&sa.cpu.gpr[3][0]==0x40);
  assert(sa.cpu.gpr[4][0]==SP-0x60);
  uint32_t v=0;
  assert(ae3_ee_bus_read32(&a,SP-0x90+0x34,&v)==AE3_EE_BUS_OK&&v==0xff);
  assert(ae3_ee_bus_read32(&a,SP-0x90+0x38,&v)==AE3_EE_BUS_OK&&v==0);
  assert(ae3_ee_bus_read32(&a,SP-0x90+0x44,&v)==AE3_EE_BUS_OK&&v==0x0072b330);
 }else{
  assert(sa.cpu.pc==AE3_EVENT_PARENT_NEXT&&sa.cpu.gpr[29][0]==SP);
  assert(sa.cpu.gpr[2][0]==UINT64_MAX&&sa.cpu.gpr[16][0]==prior.cpu.gpr[16][0]);
 }
 printf("PASS original event init global=%08X next=%08X retired=%llu\n",
    flag,sa.cpu.pc,(unsigned long long)(sa.cpu.retired-prior.cpu.retired));
 free(ra);free(rb);
}
static void faults(void){
 uint8_t *ram=calloc(1,RAM);assert(ram);
 AE3_EEMemoryBus b;ae3_ee_bus_init(&b,ram,RAM);map(&b);
 AE3_BootTimer2Finish prior=seed(&b,0);AE3_BootEventInit s;
 assert(ae3_boot_event_enter(&s,&prior)==AE3_EVT_YIELDED);
 AE3_BootEventInit original=s;
 assert(ae3_ee_bus_write32(&b,0x43fe88,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_event_slice(&s,&b,1)==AE3_EVT_UNSUPPORTED);
 assert(!memcmp(&s,&original,sizeof s));
 assert(ae3_ee_bus_write32(&b,0x43fe88,0x0c10da2e)==AE3_EE_BUS_OK);
 assert(ae3_boot_event_slice(&s,&b,3)==AE3_EVT_YIELDED);
 AE3_BootEventInit old=s;b.ram_size=0x800000;
 assert(ae3_boot_event_slice(&s,&b,1)==AE3_EVT_MEMORY_FAULT);
 assert(!memcmp(&s,&old,sizeof s));b.ram_size=RAM;
 assert(ae3_boot_event_slice(&s,&b,100)==AE3_EVT_SYSCALL_PENDING);
 free(ram);
}
int main(void){check(0);check(UINT32_C(0xffffffff));check(1);faults();
 puts("PASS original event initialization and 0x40 syscall boundary; single-step parity, opcode and memory-fault recovery");return 0;
}
