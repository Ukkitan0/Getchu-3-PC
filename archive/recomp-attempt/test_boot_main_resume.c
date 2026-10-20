/* Retail post-memset return through initial main FlushCache/next JALR. */
#include "ae3_boot_main_resume.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void code(AE3_EEMemoryBus *bus){
 FILE *f=fopen("retail_main_resume_words.tsv","r");assert(f);
 char line[128];unsigned a,w;
 while(fgets(line,sizeof line,f))if(sscanf(line,"%x %x",&a,&w)==2)
   assert(ae3_ee_bus_write32(bus,a,w)==AE3_EE_BUS_OK);
 fclose(f);
}
static AE3_BootPostTlb seed(void){
 AE3_BootPostTlb s={0};s.active=1;s.zero_stores=512;s.global_stores=2;
 s.cpu.pc=AE3_POST_TLB_RETURN_PC;s.cpu.gpr[31][0]=AE3_MAIN_CALLER_PC;
 s.cpu.gpr[4][0]=AE3_POST_TLB_ZERO_START;s.cpu.retired=329085;
 return s;
}
int main(void){
 uint8_t*r1=calloc(AE3_EE_RAM_BYTES,1),*r2=calloc(AE3_EE_RAM_BYTES,1);
 assert(r1&&r2);AE3_EEMemoryBus a,b;
 ae3_ee_bus_init(&a,r1,AE3_EE_RAM_BYTES);
 ae3_ee_bus_init(&b,r2,AE3_EE_RAM_BYTES);code(&a);code(&b);
 AE3_BootPostTlb prior=seed();AE3_BootMainResume x,y;
 assert(ae3_boot_main_resume_enter(&x,&prior)==AE3_MAIN_YIELDED);
 assert(ae3_boot_main_resume_enter(&y,&prior)==AE3_MAIN_YIELDED);
 prior.cpu.gpr[31][0]=0xdeadbeefu;
 assert(ae3_boot_main_resume_enter(&y,&prior)==AE3_MAIN_INVALID);
 prior.cpu.gpr[31][0]=AE3_MAIN_CALLER_PC;
 assert(ae3_boot_main_resume_enter(&y,&prior)==AE3_MAIN_YIELDED);
 AE3_BootMainResume save=x;
 assert(ae3_ee_bus_write32(&a,0x0042b3d8u,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_main_resume_slice(&x,&a,1)==AE3_MAIN_UNSUPPORTED);
 assert(!memcmp(&x,&save,sizeof x));
 assert(ae3_ee_bus_write32(&a,0x0042b3d8u,0x03e00008u)==AE3_EE_BUS_OK);
 a.ram_size=0x0042b3d8u;
 assert(ae3_boot_main_resume_slice(&x,&a,1)==AE3_MAIN_MEMORY_FAULT);
 assert(!memcmp(&x,&save,sizeof x));a.ram_size=AE3_EE_RAM_BYTES;
 AE3_MainResumeResult evt=AE3_MAIN_YIELDED;
 for(int step=0;step<40;step++){
   evt=ae3_boot_main_resume_slice(&x,&a,1);
   if(evt!=AE3_MAIN_YIELDED)break;
 }
 assert(evt==AE3_MAIN_FLUSH_PENDING&&x.cpu.pc==AE3_MAIN_FLUSH_SYSCALL);
 assert(ae3_boot_main_resume_slice(&y,&b,40)==evt);
 assert(!memcmp(&x,&y,sizeof x));
 save=x;
 assert(ae3_ee_bus_write32(&a,x.cpu.pc,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_main_resume_flush(&x,&a)==AE3_MAIN_UNSUPPORTED);
 assert(!memcmp(&x,&save,sizeof x));
 assert(ae3_ee_bus_write32(&a,x.cpu.pc,12u)==AE3_EE_BUS_OK);
 a.ram_size=x.cpu.pc;
 assert(ae3_boot_main_resume_flush(&x,&a)==AE3_MAIN_MEMORY_FAULT);
 assert(!memcmp(&x,&save,sizeof x));
 a.ram_size=AE3_EE_RAM_BYTES;
 assert(ae3_boot_main_resume_flush(&x,&a)==AE3_MAIN_YIELDED);
 assert(ae3_boot_main_resume_flush(&y,&b)==AE3_MAIN_YIELDED);
 assert(!memcmp(&x,&y,sizeof x));
 for(int step=0;step<40;step++){
   evt=ae3_boot_main_resume_slice(&x,&a,1);
   if(evt!=AE3_MAIN_YIELDED)break;
 }
 assert(evt==AE3_MAIN_NEXT_CALL);
 assert(ae3_boot_main_resume_slice(&y,&b,40)==evt);
 assert(!memcmp(&x,&y,sizeof x));assert(!memcmp(r1,r2,AE3_EE_RAM_BYTES));
 assert(x.cpu.pc==AE3_MAIN_NEXT_JALR);
 assert(x.cpu.gpr[16][0]==AE3_MAIN_DYNAMIC_TARGET);
 assert(x.cpu.gpr[2][0]==AE3_POST_TLB_ZERO_START);
 assert(x.flush_requests==1&&x.flush_mode==0);
 assert(x.cpu.gpr[31][0]==0x0010032cu);
 printf("PASS original main return -> FlushCache -> JALR boundary: %llu more original guest instructions; target=%08X\n",
        (unsigned long long)(x.cpu.retired-prior.cpu.retired),(unsigned)x.cpu.gpr[16][0]);
 free(r1);free(r2);return 0;
}