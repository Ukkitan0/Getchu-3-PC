#include "ae3_boot_osd_roundtrip.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM AE3_EE_RAM_BYTES
#define SP UINT32_C(0x01FDFD90)
static void save64(AE3_EEMemoryBus *b,uint32_t addr,uint64_t v){
 uint8_t bytes[8];for(unsigned i=0;i<8;i++)bytes[i]=(uint8_t)(v>>(i*8));
 assert(ae3_ee_bus_write(b,addr,bytes,8)==AE3_EE_BUS_OK);
}
static void map(AE3_EEMemoryBus *b){
 FILE *f=fopen("retail_osd_roundtrip_words.tsv","r");assert(f);
 char line[160];unsigned pc,word;
 while(fgets(line,sizeof line,f))if(sscanf(line,"%x %x",&pc,&word)==2)
  assert(ae3_ee_bus_write32(b,pc,word)==AE3_EE_BUS_OK);
 fclose(f);
 save64(b,SP+0x10u,0x0000111122223333u); /* nested saved s0 */
 save64(b,SP+0x20u,0x0043ff98u); /* nested saved ra */
 save64(b,SP+0x30u,0x0000444455556666u); /* parent saved s0 */
 save64(b,SP+0x40u,0x0000777788889999u); /* parent saved s1 */
 save64(b,SP+0x50u,0x0000aaaabbbbccccu); /* parent saved s2 */
 save64(b,SP+0x60u,0x0043fe98u); /* parent saved ra */
}
static AE3_BootOsdEntry seed(void){
 AE3_BootOsdEntry s={0};s.active=1;s.cpu.pc=AE3_OSD_GET_PC;
 s.cpu.retired=323510u;s.cpu.gpr[3][0]=0x4bu;
 s.cpu.gpr[4][0]=SP;s.cpu.gpr[29][0]=SP;
 s.cpu.gpr[31][0]=0x0043ff2cu;
 return s;
}
static void run(void){
 uint8_t *ra=calloc(1,RAM),*rb=calloc(1,RAM);assert(ra&&rb);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,ra,RAM);ae3_ee_bus_init(&b,rb,RAM);
 map(&a);map(&b);
 AE3_BootOsdEntry initial=seed();
 AE3_BootOsdRoundtrip x,y;
 assert(ae3_boot_osd_roundtrip_enter(&x,&initial)==AE3_OSDR_GET_PENDING);
 assert(ae3_boot_osd_roundtrip_enter(&y,&initial)==AE3_OSDR_GET_PENDING);
 AE3_EEOsdConfig ka,kb;ae3_ee_osd_reset(&ka);ae3_ee_osd_reset(&kb);
 AE3_BootOsdRoundtrip before=x;
 assert(ae3_boot_osd_roundtrip_get(&x,&a,&ka)==AE3_OSDR_UNBOUND);
 assert(!memcmp(&x,&before,sizeof x));
 assert(ae3_ee_osd_bind(&ka,0x00004019u)==0);
 assert(ae3_ee_osd_bind(&kb,0x00004019u)==0);
 assert(ae3_ee_osd_bind(&ka,0u)==-1);
 uint32_t prior=0;
 assert(ae3_ee_bus_read32(&a,SP,&prior)==AE3_EE_BUS_OK&&prior==0);
 assert(ae3_ee_bus_write32(&a,AE3_OSD_GET_PC,0u)==AE3_EE_BUS_OK);
 assert(ae3_boot_osd_roundtrip_get(&x,&a,&ka)==AE3_OSDR_UNSUPPORTED);
 assert(!memcmp(&x,&before,sizeof x));
 assert(ae3_ee_bus_write32(&a,AE3_OSD_GET_PC,12u)==AE3_EE_BUS_OK);
 a.ram_size=SP;
 assert(ae3_boot_osd_roundtrip_get(&x,&a,&ka)==AE3_OSDR_MEMORY_FAULT);
 assert(!memcmp(&x,&before,sizeof x));
 a.ram_size=RAM;
 assert(ae3_boot_osd_roundtrip_get(&x,&a,&ka)==AE3_OSDR_YIELDED);
 assert(ae3_boot_osd_roundtrip_get(&y,&b,&kb)==AE3_OSDR_YIELDED);
 assert(!memcmp(&x,&y,sizeof x));
 assert(ae3_ee_bus_read32(&a,SP,&prior)==AE3_EE_BUS_OK&&prior==0x4019u);
 unsigned calls=1;
 for(unsigned n=0;n<160;n++){
  AE3_OsdRoundtripResult rx=ae3_boot_osd_roundtrip_slice(&x,&a,1);
  if(rx==AE3_OSDR_YIELDED)continue;
  AE3_OsdRoundtripResult ry=ae3_boot_osd_roundtrip_slice(&y,&b,200);
  assert(rx==ry);
  assert(!memcmp(&x,&y,sizeof x)&&!memcmp(ra,rb,RAM));
  if(rx==AE3_OSDR_PARENT_RETURNED)break;
  assert(rx==AE3_OSDR_GET_PENDING||rx==AE3_OSDR_SET_PENDING);
  AE3_BootOsdRoundtrip state_before=x;AE3_EEOsdConfig kernel_before=ka;
  if(rx==AE3_OSDR_SET_PENDING){
   assert(ae3_ee_bus_write32(&a,AE3_OSD_SET_PC,0u)==AE3_EE_BUS_OK);
   assert(ae3_boot_osd_roundtrip_set(&x,&a,&ka)==AE3_OSDR_UNSUPPORTED);
   assert(!memcmp(&x,&state_before,sizeof x));
   assert(!memcmp(&ka,&kernel_before,sizeof ka));
   assert(ae3_ee_bus_write32(&a,AE3_OSD_SET_PC,12u)==AE3_EE_BUS_OK);
   if(x.write_count==0u){
    a.ram_size=SP;assert(ae3_boot_osd_roundtrip_set(&x,&a,&ka)==AE3_OSDR_MEMORY_FAULT);
    assert(!memcmp(&x,&state_before,sizeof x));
    assert(!memcmp(&ka,&kernel_before,sizeof ka));a.ram_size=RAM;
   }
   assert(ae3_boot_osd_roundtrip_set(&x,&a,&ka)==AE3_OSDR_YIELDED);
   assert(ae3_boot_osd_roundtrip_set(&y,&b,&kb)==AE3_OSDR_YIELDED);
   if(x.write_count==1)assert(ka.packed==0x00002019u);
   if(x.write_count==2)assert(ka.packed==0x00004019u);
  }else{
   assert(ae3_boot_osd_roundtrip_get(&x,&a,&ka)==AE3_OSDR_YIELDED);
   assert(ae3_boot_osd_roundtrip_get(&y,&b,&kb)==AE3_OSDR_YIELDED);
  }
  assert(!memcmp(&x,&y,sizeof x)&&!memcmp(&ka,&kb,sizeof ka));
  calls++;
 }
 assert(calls==4 && x.read_count==2 && x.write_count==2);
 assert(x.cpu.pc==AE3_OSD_PARENT_RETURN&&x.cpu.in_delay_slot==0);
 assert(x.cpu.gpr[29][0]==SP+0x70u);
 assert(x.cpu.gpr[31][0]==0x0043fe98u);
 assert(ka.packed==0x4019u&&ka.generation==3);
 assert(!memcmp(&x,&y,sizeof x)&&!memcmp(ra,rb,RAM));
 assert(ae3_boot_osd_roundtrip_slice(&x,&a,1)==AE3_OSDR_PARENT_RETURNED);
 printf("PASS OSD round-trip: original Get/Set twice, temporary %08X, restored %08X, return %08X; %llu guest instructions\n",
   0x2019u,ka.packed,x.cpu.pc,(unsigned long long)(x.cpu.retired-initial.cpu.retired));
 free(ra);free(rb);
}
int main(void){run();puts("PASS: bounded retail OSD getter/setter round-trip and atomic kernel state");return 0;}