/* Execute retail constructor 0x0041A390 through the real internal routine;
   validates cold/warm lazy-init branches and return to guest dispatcher. */
#include "ae3_boot_first_constructor.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const uint32_t flags[]={
 UINT32_C(0x0072D608),UINT32_C(0x0072D600),UINT32_C(0x0072D5F0),
 UINT32_C(0x0072D5B0),UINT32_C(0x0072D638),UINT32_C(0x0072D5C8),
 UINT32_C(0x0072D610),UINT32_C(0x0072D5D8),UINT32_C(0x0072D5F8),
 UINT32_C(0x0072D658),UINT32_C(0x0072D5A8),UINT32_C(0x0072D5E0)
};
static void install_words(AE3_EEMemoryBus *b,const char *name){
 FILE *f=fopen(name,"rb");assert(f);
 uint32_t a,v;int got=0;
 while((got=fscanf(f,"%x %x",&a,&v))==2)
   assert(ae3_ee_bus_write32(b,a,v)==AE3_EE_BUS_OK);
 assert(got==EOF);fclose(f);
}
static void qstore(AE3_EEMemoryBus *b,uint32_t a,uint64_t v){
 uint8_t raw[8];for(unsigned j=0;j<8;j++)raw[j]=(uint8_t)(v>>(8*j));
 assert(ae3_ee_bus_write(b,a,raw,8)==AE3_EE_BUS_OK);
}
static uint64_t qload(const AE3_EEMemoryBus *b,uint32_t a){
 uint8_t raw[8];uint64_t v=0;
 assert(ae3_ee_bus_read(b,a,raw,8)==AE3_EE_BUS_OK);
 for(unsigned j=0;j<8;j++)v|=(uint64_t)raw[j]<<(8*j);
 return v;
}
static AE3_BootFirstConstructor begin(AE3_EEMemoryBus *b,uint32_t second){
 install_words(b,"retail_static_dispatch_words.tsv");
 install_words(b,"retail_first_constructor_words.tsv");
 assert(ae3_ee_bus_write32(b,AE3_STATIC_CONSTRUCTOR_TABLE,AE3_FIRST_CTOR_PC)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(b,AE3_STATIC_CONSTRUCTOR_TABLE-4,second)==AE3_EE_BUS_OK);
 AE3_BootMainResume pre={0};pre.active=1;pre.flush_requests=1;
 pre.cpu.pc=AE3_STATIC_INIT_ENTRY;pre.cpu.gpr[16][0]=UINT64_C(0x00628210);
 pre.cpu.gpr[29][0]=UINT64_C(0x01FED000);pre.cpu.gpr[31][0]=UINT64_C(0x0010032C);
 pre.cpu.retired=UINT64_C(329097);
 AE3_BootStaticDispatch statics;
 assert(ae3_boot_static_enter(&statics,&pre)==AE3_STATIC_YIELDED);
 assert(ae3_boot_static_slice(&statics,b,256)==AE3_STATIC_CONSTRUCTOR_PENDING);
 assert(statics.guest_ctor_word==AE3_FIRST_CTOR_PC);
 AE3_BootFirstConstructor first;
 assert(ae3_boot_first_ctor_enter(&first,&statics)==AE3_FCTOR_YIELDED);
 return first;
}
static void run_case(unsigned warm,uint32_t second,AE3_FirstCtorResult wanted){
 uint8_t *ra=calloc(1,AE3_EE_RAM_BYTES),*rb=calloc(1,AE3_EE_RAM_BYTES);
 assert(ra&&rb);
 AE3_EEMemoryBus ba,bb;ae3_ee_bus_init(&ba,ra,AE3_EE_RAM_BYTES);
 ae3_ee_bus_init(&bb,rb,AE3_EE_RAM_BYTES);
 AE3_BootFirstConstructor a=begin(&ba,second),b=begin(&bb,second);
 for(unsigned i=0;i<12;i++){
   qstore(&ba,flags[i],warm);qstore(&bb,flags[i],warm);
 }
 AE3_BootFirstConstructor frozen=a;
 uint32_t op=0;
 assert(ae3_ee_bus_read32(&ba,a.cpu.pc,&op)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(&ba,a.cpu.pc,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_first_ctor_slice(&a,&ba,1)==AE3_FCTOR_UNSUPPORTED);
 assert(!memcmp(&frozen,&a,sizeof a));
 assert(ae3_ee_bus_write32(&ba,a.cpu.pc,op)==AE3_EE_BUS_OK);
 ba.ram_size=a.cpu.pc;
 assert(ae3_boot_first_ctor_slice(&a,&ba,1)==AE3_FCTOR_MEMORY_FAULT);
 assert(!memcmp(&frozen,&a,sizeof a));
 ba.ram_size=AE3_EE_RAM_BYTES;
 AE3_FirstCtorResult result=AE3_FCTOR_YIELDED;
 int faulted=0;
 for(unsigned step=0;step<1000;step++){
   if(!warm && a.cpu.pc==UINT32_C(0x0041A0C8) && !faulted){
     AE3_BootFirstConstructor before=a;
     uint64_t flag_before=qload(&ba,flags[0]);
     ba.ram_size=flags[0];
     assert(ae3_boot_first_ctor_slice(&a,&ba,1)==AE3_FCTOR_MEMORY_FAULT);
     assert(!memcmp(&a,&before,sizeof a));
     ba.ram_size=AE3_EE_RAM_BYTES;
     assert(qload(&ba,flags[0])==flag_before);
     faulted=1;
   }
   result=ae3_boot_first_ctor_slice(&a,&ba,1);
   if(result!=AE3_FCTOR_YIELDED)break;
 }
 assert(result==wanted);
 assert(warm||faulted);
 assert(ae3_boot_first_ctor_slice(&b,&bb,1000)==wanted);
 assert(!memcmp(&a,&b,sizeof a));
 assert(!memcmp(ra,rb,AE3_EE_RAM_BYTES));
 assert(a.reference_count_stores==12u);
 assert(a.stub_calls==(warm?0u:12u));
 assert(a.cpu.gpr[29][0]==UINT64_C(0x01FECFB0)); /* dispatcher frame */
 assert(a.cpu.gpr[31][0]==UINT64_C(0x00428AF0));
 for(unsigned i=0;i<12;i++)assert(qload(&ba,flags[i])==(warm?2u:1u));
 if(wanted==AE3_FCTOR_SECOND_PENDING){
  assert(a.cpu.pc==UINT32_C(0x00428AE8));
  assert(a.next_constructor==AE3_SECOND_CTOR_PC);
  assert((uint32_t)a.cpu.gpr[14][0]==AE3_SECOND_CTOR_PC);
 }else{
  assert(a.cpu.pc==UINT32_C(0x00428AFC));
  assert(a.next_constructor==UINT32_C(0xFFFFFFFF));
 }
 printf("PASS first guest constructor %s: %llu retired MIPS; %u reference-counter stores and %u genuine stub calls; next PC %08X\n",
   warm?"already initialized":"cold",(unsigned long long)(a.cpu.retired-frozen.cpu.retired),
   a.reference_count_stores,a.stub_calls,a.cpu.pc);
 free(ra);free(rb);
}
int main(void){
 run_case(0,AE3_SECOND_CTOR_PC,AE3_FCTOR_SECOND_PENDING);
 run_case(1,AE3_SECOND_CTOR_PC,AE3_FCTOR_SECOND_PENDING);
 run_case(0,UINT32_C(0xFFFFFFFF),AE3_FCTOR_DISPATCH_DONE);
 puts("PASS original first constructor executed through JALR, tail-call, 12 flag checks, stub returns, dispatcher loop");
 return 0;
}