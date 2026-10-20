/* Execute original retail words from a strictly pinned opcode fixture. */
#include "ae3_boot_timer2_finish.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM AE3_EE_RAM_BYTES
#define SP UINT32_C(0x01FE0000)
static void map_code(AE3_EEMemoryBus *b){
 FILE *f=fopen("retail_timer2_finish_words.tsv","r");assert(f);
 unsigned address,word;unsigned count=0;char line[128];
 while(fgets(line,sizeof line,f)){
  if(sscanf(line,"%x %x",&address,&word)==2){
   assert(ae3_ee_bus_write32(b,address,word)==AE3_EE_BUS_OK);count++;
  }
 }
 fclose(f);assert(count>=58);
}
static void sd(AE3_EEMemoryBus *b,uint32_t addr,uint64_t v){
 uint8_t data[8];for(unsigned i=0;i<8;i++)data[i]=(uint8_t)(v>>(i*8));
 assert(ae3_ee_bus_write(b,addr,data,8)==AE3_EE_BUS_OK);
}
static AE3_BootTimer2Recheck seed(AE3_EEMemoryBus *b,uint32_t s0){
 AE3_BootTimer2Recheck c={0};c.active=1;c.cpu.pc=AE3_T2_RECHECK_MMIO_LW;
 c.cpu.retired=323393;
 c.cpu.gpr[2][0]=AE3_T2_MODE;c.cpu.gpr[16][0]=s0;
 c.cpu.gpr[29][0]=SP-0x20u;c.cpu.gpr[31][0]=0x00440704;
 sd(b,SP-0x20,UINT64_C(0xFACE1234ABCDEF01));
 sd(b,SP-0x10,AE3_T2_FINISH_PARENT_NEXT);
 return c;
}
static void check_case(uint32_t mode,uint32_t initial_s0,int fault){
 uint8_t *ra=calloc(1,RAM),*rb=calloc(1,RAM);assert(ra&&rb);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,ra,RAM);ae3_ee_bus_init(&b,rb,RAM);
 map_code(&a);map_code(&b);
 AE3_EETimer2 ta,tb;ae3_ee_timer2_reset(&ta);ae3_ee_timer2_reset(&tb);
 assert(ae3_ee_timer2_write(&ta,AE3_T2_MODE,mode)==AE3_EE_BUS_OK);
 assert(ae3_ee_timer2_write(&tb,AE3_T2_MODE,mode)==AE3_EE_BUS_OK);
 ae3_ee_bus_attach_mmio(&a,ae3_ee_timer2_read,ae3_ee_timer2_write,&ta);
 ae3_ee_bus_attach_mmio(&b,ae3_ee_timer2_read,ae3_ee_timer2_write,&tb);
 AE3_BootTimer2Recheck parent=seed(&a,initial_s0);
 (void)seed(&b,initial_s0);
 AE3_BootTimer2Finish sa,sb;
 assert(ae3_boot_timer2_finish_enter(&sa,&parent)==AE3_T2F_YIELDED);
 assert(ae3_boot_timer2_finish_enter(&sb,&parent)==AE3_T2F_YIELDED);
 if(fault){
  AE3_BootTimer2Finish saved=sa;a.mmio_read32=NULL;
  assert(ae3_boot_timer2_finish_slice(&sa,&a,1)==AE3_T2F_MEMORY_FAULT);
  assert(memcmp(&sa,&saved,sizeof sa)==0);
  a.mmio_read32=ae3_ee_timer2_read;
 }
 const AE3_T2FinishResult expect=!(mode&0x80u)?AE3_T2F_SET_MODE_PENDING:
     initial_s0?AE3_T2F_EINTR_PENDING:AE3_T2F_RETURNED;
 AE3_T2FinishResult sr=AE3_T2F_YIELDED;
 for(unsigned i=0;i<90;i++){
  sr=ae3_boot_timer2_finish_slice(&sa,&a,1);
  if(sr!=AE3_T2F_YIELDED)break;
 }
 assert(sr==expect);
 assert(ae3_boot_timer2_finish_slice(&sb,&b,100)==expect);
 assert(!memcmp(&sa,&sb,sizeof sa));
 assert(memcmp(ra,rb,RAM)==0&&memcmp(&ta,&tb,sizeof ta)==0);
 assert(sa.observed_mode==(mode&0x3ffu)&&sa.read_performed);
 if(expect==AE3_T2F_RETURNED){
  assert(sa.cpu.pc==AE3_T2_FINISH_PARENT_NEXT&&sa.cpu.gpr[29][0]==SP);
  assert(sa.cpu.gpr[16][0]==UINT64_C(0xFACE1234ABCDEF01));
  assert(sa.cpu.gpr[2][0]==1);
  assert(sa.cpu.retired==parent.cpu.retired+10);
 }else if(expect==AE3_T2F_EINTR_PENDING){
  assert(sa.cpu.pc==AE3_T2_FINISH_EINTR_CALL&&sa.cpu.gpr[2][0]==1);
 }else{
  assert(sa.cpu.pc==AE3_T2_FINISH_MODE_CALL);
  assert((uint32_t)sa.cpu.gpr[4][0]==((mode&0x3ffu)&0xfffff3ffu));
  AE3_BootTimer2Setup na,nb;
  assert(ae3_boot_timer2_setup_recheck_enter(&na,&sa.cpu,sa.cop0_status)==AE3_BT2S_YIELDED);
  assert(ae3_boot_timer2_setup_recheck_enter(&nb,&sb.cpu,sb.cop0_status)==AE3_BT2S_YIELDED);
  AE3_BootTimer2SetupResult r=AE3_BT2S_YIELDED;
  for(unsigned i=0;i<80;i++){
   r=ae3_boot_timer2_setup_slice(&na,&a,1);
   if(r!=AE3_BT2S_YIELDED)break;
  }
  assert(r==AE3_BT2S_RECHECK_NEXT_CALL);
  assert(ae3_boot_timer2_setup_slice(&nb,&b,100)==AE3_BT2S_RECHECK_NEXT_CALL);
  assert(!memcmp(&na,&nb,sizeof na));assert(memcmp(&ta,&tb,sizeof ta)==0);
  assert(na.io_stores==1&&na.cpu.pc==AE3_T2_FINISH_NEXT_SERVICE);
  assert(ta.mode&0x80u);
  assert(ae3_boot_timer2_finish_resume_mode(&sa,&na)==AE3_T2F_NEXT_SERVICE_PENDING);
  assert(ae3_boot_timer2_finish_resume_mode(&sb,&nb)==AE3_T2F_NEXT_SERVICE_PENDING);
  assert(!memcmp(&sa,&sb,sizeof sa));
 }
 printf("PASS Timer2 second-init MODE=%04X savedDIntr=%u retired=%llu next=%08X\n",
       mode,initial_s0,(unsigned long long)(sa.cpu.retired-parent.cpu.retired),sa.cpu.pc);
 free(ra);free(rb);
}
static void check_faults(void){
 uint8_t *ram=calloc(1,RAM);assert(ram);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,RAM);map_code(&bus);
 AE3_EETimer2 timer;ae3_ee_timer2_reset(&timer);
 assert(ae3_ee_timer2_write(&timer,AE3_T2_MODE,0x80)==AE3_EE_BUS_OK);
 ae3_ee_bus_attach_mmio(&bus,ae3_ee_timer2_read,ae3_ee_timer2_write,&timer);
 AE3_BootTimer2Recheck p=seed(&bus,0);AE3_BootTimer2Finish s;
 assert(ae3_boot_timer2_finish_enter(&s,&p)==AE3_T2F_YIELDED);
 AE3_BootTimer2Finish old=s;
 assert(ae3_ee_bus_write32(&bus,0x00440710,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_timer2_finish_slice(&s,&bus,1)==AE3_T2F_UNSUPPORTED);
 assert(!memcmp(&old,&s,sizeof s));
 assert(ae3_ee_bus_write32(&bus,0x00440710,0x8c420000)==AE3_EE_BUS_OK);
 assert(ae3_boot_timer2_finish_slice(&s,&bus,6)==AE3_T2F_YIELDED);
 old=s;bus.ram_size=UINT32_C(0x1000000);
 assert(ae3_boot_timer2_finish_slice(&s,&bus,1)==AE3_T2F_MEMORY_FAULT);
 assert(!memcmp(&old,&s,sizeof s));bus.ram_size=RAM;
 assert(ae3_boot_timer2_finish_slice(&s,&bus,40)==AE3_T2F_RETURNED);
 p.cpu.gpr[2][0]=0;
 assert(ae3_boot_timer2_finish_enter(&s,&p)==AE3_T2F_INVALID);
 free(ram);
}
int main(void){
 check_case(0x80u,0,1);
 check_case(0x180u,0,0);
 check_case(0x80u,1,0);
 check_case(0,0,0);
 check_case(0x0003u,0,0);
 check_faults();
 puts("PASS original second Timer2 MODE LW, both status branches, nested original SetT2_MODE COP0/ERET, faults, step parity");
 return 0;
}
