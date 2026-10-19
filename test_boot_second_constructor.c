/* Original second table callback, two guest no-op stubs, nested numeric call.
   The numeric conversion routine remains a real UNSERVICED guest boundary. */
#include "ae3_boot_second_numeric.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static void pin(AE3_EEMemoryBus *bus,const char *name){
 FILE *f=fopen(name,"r");assert(f);uint32_t a,v;int ret;
 while((ret=fscanf(f,"%x %x",&a,&v))==2)
   assert(ae3_ee_bus_write32(bus,a,v)==AE3_EE_BUS_OK);
 assert(ret==EOF);fclose(f);
}
static AE3_BootSecondConstructor setup(AE3_EEMemoryBus *bus){
 pin(bus,"retail_static_dispatch_words.tsv");
 pin(bus,"retail_first_constructor_words.tsv");
 pin(bus,"retail_second_constructor_words.tsv");
 pin(bus,"retail_second_numeric_words.tsv");
 assert(ae3_ee_bus_write32(bus,AE3_STATIC_CONSTRUCTOR_TABLE,
                             AE3_FIRST_CTOR_PC)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(bus,AE3_STATIC_CONSTRUCTOR_TABLE-4,
                             AE3_SECOND_CTOR_PC)==AE3_EE_BUS_OK);
 AE3_BootMainResume pre={0};pre.active=1;pre.flush_requests=1;
 pre.cpu.pc=AE3_STATIC_INIT_ENTRY;pre.cpu.gpr[16][0]=UINT64_C(0x00628210);
 pre.cpu.gpr[29][0]=UINT64_C(0x01FED000);pre.cpu.gpr[31][0]=UINT64_C(0x0010032C);
 pre.cpu.retired=329097;
 AE3_BootStaticDispatch statics;
 assert(ae3_boot_static_enter(&statics,&pre)==AE3_STATIC_YIELDED);
 assert(ae3_boot_static_slice(&statics,bus,256)==AE3_STATIC_CONSTRUCTOR_PENDING);
 AE3_BootFirstConstructor first;
 assert(ae3_boot_first_ctor_enter(&first,&statics)==AE3_FCTOR_YIELDED);
 assert(ae3_boot_first_ctor_slice(&first,bus,1000)==AE3_FCTOR_SECOND_PENDING);
 AE3_BootSecondConstructor second;
 assert(ae3_boot_second_ctor_enter(&second,&first)==AE3_SC_YIELDED);
 return second;
}
int main(void){
 uint8_t *ra=calloc(1,AE3_EE_RAM_BYTES),*rb=calloc(1,AE3_EE_RAM_BYTES);
 assert(ra&&rb);
 AE3_EEMemoryBus ba,bb;ae3_ee_bus_init(&ba,ra,AE3_EE_RAM_BYTES);
 ae3_ee_bus_init(&bb,rb,AE3_EE_RAM_BYTES);
 AE3_BootSecondConstructor a=setup(&ba),b=setup(&bb),saved=a;
 assert(a.cpu.pc==0x00428AE8);
 uint32_t original=0;assert(ae3_ee_bus_read32(&ba,a.cpu.pc,&original)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(&ba,a.cpu.pc,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_second_ctor_slice(&a,&ba,1)==AE3_SC_UNSUPPORTED);
 assert(!memcmp(&a,&saved,sizeof a));
 assert(ae3_ee_bus_write32(&ba,a.cpu.pc,original)==AE3_EE_BUS_OK);
 ba.ram_size=a.cpu.pc;
 assert(ae3_boot_second_ctor_slice(&a,&ba,1)==AE3_SC_MEMORY_FAULT);
 assert(!memcmp(&a,&saved,sizeof a));ba.ram_size=AE3_EE_RAM_BYTES;
 int faulted=0;AE3_SecondCtorResult step=AE3_SC_YIELDED;
 for(unsigned i=0;i<200;i++){
  if(a.cpu.pc==0x00417B58 && !faulted){
   AE3_BootSecondConstructor freeze=a;
   ba.ram_size=(size_t)(uint32_t)a.cpu.gpr[29][0]+8u;
   assert(ae3_boot_second_ctor_slice(&a,&ba,1)==AE3_SC_MEMORY_FAULT);
   assert(!memcmp(&a,&freeze,sizeof a));
   ba.ram_size=AE3_EE_RAM_BYTES;faulted=1;
  }
  step=ae3_boot_second_ctor_slice(&a,&ba,1);
  if(step!=AE3_SC_YIELDED)break;
 }
 assert(faulted&&step==AE3_SC_NUMERIC_PENDING);
 assert(ae3_boot_second_ctor_slice(&b,&bb,200)==AE3_SC_NUMERIC_PENDING);
 assert(memcmp(&a,&b,sizeof a)==0);
 assert(memcmp(ra,rb,AE3_EE_RAM_BYTES)==0);
 assert(a.cpu.pc==AE3_SECOND_NUMERIC_BOUNDARY);
 assert(a.cpu.gpr[31][0]==UINT64_C(0x00417BD4));
 assert(a.stub_calls==2&&a.wrapper_visits==1&&a.saved_first_stores==12);
 assert(a.cpu.gpr[4][0]==(UINT64_C(0x1009)<<50));
 AE3_BootSecondNumeric ma,mb;
 assert(ae3_boot_second_numeric_enter(&ma,&a)==AE3_SN_YIELDED);
 assert(ae3_boot_second_numeric_enter(&mb,&b)==AE3_SN_YIELDED);
 AE3_BootSecondNumeric frozen_math=ma;
 uint32_t original_math=0;
 assert(ae3_ee_bus_read32(&ba,ma.cpu.pc,&original_math)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(&ba,ma.cpu.pc,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_second_numeric_slice(&ma,&ba,1)==AE3_SN_UNSUPPORTED);
 assert(!memcmp(&ma,&frozen_math,sizeof ma));
 assert(ae3_ee_bus_write32(&ba,ma.cpu.pc,original_math)==AE3_EE_BUS_OK);
 AE3_SecondNumericResult ms=AE3_SN_YIELDED;
 int math_faulted=0;
 for(unsigned i=0;i<150;i++){
   if(ma.cpu.pc==UINT32_C(0x0041EFD8) && !math_faulted){
     AE3_BootSecondNumeric frozen=ma;
     ba.ram_size=(size_t)(uint32_t)ma.cpu.gpr[29][0]+16u;
     assert(ae3_boot_second_numeric_slice(&ma,&ba,1)==AE3_SN_MEMORY_FAULT);
     assert(!memcmp(&ma,&frozen,sizeof ma));
     ba.ram_size=AE3_EE_RAM_BYTES;
     math_faulted=1;
   }
   ms=ae3_boot_second_numeric_slice(&ma,&ba,1);
   if(ms!=AE3_SN_YIELDED)break;
 }
 assert(math_faulted&&ms==AE3_SN_MATH_CALL_PENDING);
 assert(ae3_boot_second_numeric_slice(&mb,&bb,150)==AE3_SN_MATH_CALL_PENDING);
 assert(!memcmp(&ma,&mb,sizeof ma));
 assert(!memcmp(ra,rb,AE3_EE_RAM_BYTES));
 assert(ma.cpu.pc==AE3_SECOND_MATH_CALLEE_JAL);
 uint32_t call=0;
 assert(ae3_ee_bus_read32(&ba,ma.cpu.pc,&call)==AE3_EE_BUS_OK);
 assert((call>>26)==3u); /* Original MIPS JAL, not a fabricated dispatch. */
 assert((((ma.cpu.pc+4u)&UINT32_C(0xf0000000))|
             ((call&UINT32_C(0x03ffffff))<<2))==AE3_SECOND_MATH_CALLEE);
 assert(ma.double_high_word==UINT32_C(0x40240000));
 assert(ma.cpu.gpr[4][0]!=UINT64_C(0));
 printf("PASS numeric pre-call: %llu further retail MIPS, double high=%08X, next JAL at %08X\n",
   (unsigned long long)(ma.cpu.retired-frozen_math.cpu.retired),ma.double_high_word,ma.cpu.pc);
 printf("PASS second retail constructor: %llu original MIPS instructions, two real stub calls, numeric routine pending at %08X\n",
   (unsigned long long)(a.cpu.retired-saved.cpu.retired),a.cpu.pc);
 free(ra);free(rb);return 0;
}