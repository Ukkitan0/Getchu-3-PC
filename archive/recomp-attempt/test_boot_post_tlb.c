/* Retail startup caller tail and 512 real MIPS SB stores. No game files. */
#include "ae3_boot_post_tlb.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK UINT32_C(0x01fdfdf0)
static void load_ops(AE3_EEMemoryBus *b){
 FILE *f=fopen("retail_post_tlb_words.tsv","r");assert(f);
 char line[128];unsigned pc=0,word=0;
 while(fgets(line,sizeof line,f))
   if(sscanf(line,"%x %x",&pc,&word)==2)
     assert(ae3_ee_bus_write32(b,pc,word)==AE3_EE_BUS_OK);
 fclose(f);
 assert(ae3_ee_bus_write32(b,0x006a2e40u,0xf00dcafeu)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(b,0x006a2e44u,0xbabef00du)==AE3_EE_BUS_OK);
 uint8_t data[AE3_POST_TLB_ZERO_BYTES+2];memset(data,0xa5,sizeof data);
 assert(ae3_ee_bus_write(b,AE3_POST_TLB_ZERO_START-1,data,sizeof data)==AE3_EE_BUS_OK);
 uint8_t saved_ra[8]={0x78,0x56,0x34,0x12,0,0,0,0};
 assert(ae3_ee_bus_write(b,STACK,saved_ra,8)==AE3_EE_BUS_OK);
}
static AE3_BootTlbFinalize prior(void){
 AE3_BootTlbFinalize s={0};s.active=1;s.done=1;s.entries=6;s.sets=5;
 s.cpu.pc=AE3_POST_TLB_ENTRY;s.cpu.gpr[29][0]=STACK;
 s.cpu.retired=325476u;
 return s;
}
int main(void){
 uint8_t *r1=calloc(AE3_EE_RAM_BYTES,1),*r2=calloc(AE3_EE_RAM_BYTES,1);
 assert(r1&&r2);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,r1,AE3_EE_RAM_BYTES);
 ae3_ee_bus_init(&b,r2,AE3_EE_RAM_BYTES);load_ops(&a);load_ops(&b);
 AE3_BootTlbFinalize seed=prior();AE3_BootPostTlb one,two;
 assert(ae3_boot_post_tlb_enter(&one,&seed)==AE3_PTL_YIELDED);
 assert(ae3_boot_post_tlb_enter(&two,&seed)==AE3_PTL_YIELDED);
 /* Opcode mutation is detected before any guest state changes. */
 AE3_BootPostTlb snapshot=one;
 assert(ae3_ee_bus_write32(&a,AE3_POST_TLB_ENTRY,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_post_tlb_slice(&one,&a,1)==AE3_PTL_UNSUPPORTED);
 assert(memcmp(&snapshot,&one,sizeof one)==0);
 assert(ae3_ee_bus_write32(&a,AE3_POST_TLB_ENTRY,0xdfbf0000u)==AE3_EE_BUS_OK);
 /* A missing saved return address is a recoverable aligned LD fault. */
 a.ram_size=STACK;
 assert(ae3_boot_post_tlb_slice(&one,&a,1)==AE3_PTL_MEMORY_FAULT);
 assert(memcmp(&snapshot,&one,sizeof one)==0);
 a.ram_size=AE3_EE_RAM_BYTES;
 /* Guest saved return address and every byte store are actually executed. */
 int faulted=0;AE3_PostTlbResult status=AE3_PTL_YIELDED;
 for(unsigned i=0;i<5000;i++){
   if(!faulted&&one.cpu.pc==0x0042b3bcu){
     AE3_BootPostTlb state=one;
     a.ram_size=AE3_POST_TLB_ZERO_START;
     assert(ae3_boot_post_tlb_slice(&one,&a,1)==AE3_PTL_MEMORY_FAULT);
     assert(!memcmp(&one,&state,sizeof one));
     a.ram_size=AE3_EE_RAM_BYTES;faulted=1;
   }
   status=ae3_boot_post_tlb_slice(&one,&a,1);
   if(status!=AE3_PTL_YIELDED)break;
 }
 assert(status==AE3_PTL_RETURN_PENDING&&faulted);
 assert(ae3_boot_post_tlb_slice(&two,&b,5000)==AE3_PTL_RETURN_PENDING);
 assert(!memcmp(&one,&two,sizeof one));
 assert(!memcmp(r1,r2,AE3_EE_RAM_BYTES));
 assert(one.cpu.pc==AE3_POST_TLB_RETURN_PC);
 assert(one.cpu.gpr[31][0]==UINT64_C(0x12345678));
 assert(one.cpu.gpr[29][0]==STACK+16u);
 assert(one.zero_stores==AE3_POST_TLB_ZERO_BYTES&&one.global_stores==2);
 assert(one.cpu.gpr[2][0]==UINT64_C(0xffffffffffffffff));
 assert(one.cpu.gpr[3][0]==AE3_POST_TLB_ZERO_START+AE3_POST_TLB_ZERO_BYTES);
 assert(one.cpu.gpr[6][0]==UINT64_C(0xffffffffffffffff));
 uint32_t result=1;
 assert(ae3_ee_bus_read32(&a,0x006a2e40u,&result)==AE3_EE_BUS_OK&&result==0);
 result=1;assert(ae3_ee_bus_read32(&a,0x006a2e44u,&result)==AE3_EE_BUS_OK&&result==0);
 uint8_t *contents=r1+AE3_POST_TLB_ZERO_START;
 for(unsigned i=0;i<AE3_POST_TLB_ZERO_BYTES;i++)assert(contents[i]==0);
 assert(contents[-1]==0xa5u&&contents[AE3_POST_TLB_ZERO_BYTES]==0xa5u);
 assert(ae3_boot_post_tlb_slice(&one,&a,16)==AE3_PTL_RETURN_PENDING);
 printf("PASS: retail 512 SB byte loop and 2 global SW, %llu guest instructions, return at %08X\n",
   (unsigned long long)(one.cpu.retired-seed.cpu.retired),one.cpu.pc);
 free(r1);free(r2);return 0;
}