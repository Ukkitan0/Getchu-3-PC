/* Bounded retail GetThreadId / ChangeThreadPriority continuation.
   Original instructions independently confirmed against user EE savestate. */
#include "ae3_boot_osd_entry.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM AE3_EE_RAM_BYTES
#define SP UINT32_C(0x01FDFD70)
static void put64(AE3_EEMemoryBus *b,uint32_t addr,uint64_t val){
 uint8_t a[8];for(unsigned i=0;i<8;i++)a[i]=(uint8_t)(val>>(i*8u));
 assert(ae3_ee_bus_write(b,addr,a,8)==AE3_EE_BUS_OK);
}
static void map(AE3_EEMemoryBus *bus){
 const char *tables[]={"retail_thread_priority_words.tsv","retail_osd_entry_words.tsv"};
 for(unsigned table=0;table<2;table++){
 FILE *f=fopen(tables[table],"r");assert(f);
 unsigned pc,op;char line[160];
 while(fgets(line,sizeof line,f)){
  if(sscanf(line,"%x %x",&pc,&op)==2)
   assert(ae3_ee_bus_write32(bus,pc,op)==AE3_EE_BUS_OK);
 }
 fclose(f);
 }
 assert(ae3_ee_bus_write32(bus,0x006a1788u,1u)==AE3_EE_BUS_OK);
 /* Genuine parent's saved EE stack frame layout. */
 put64(bus,SP+0x80u,0x0043fe90u);
 put64(bus,SP+0x70u,0x1111222233334444u);
 put64(bus,SP+0x60u,0x5555666677778888u);
 put64(bus,SP+0x50u,0x9999aaaabbbbccccu);
}
static AE3_BootThreadNext seed(void){
 AE3_BootThreadNext n={0};n.active=1;
 n.cpu.pc=AE3_TPR_GETID_SYSCALL;n.cpu.retired=323479;
 n.cpu.gpr[3][0]=0x2fu;n.cpu.gpr[31][0]=0x0043697cu;
 n.cpu.gpr[16][0]=0x006a0000u;n.cpu.gpr[29][0]=SP;
 return n;
}
static void run_happy(void){
 uint8_t *ra=calloc(1,RAM),*rb=calloc(1,RAM);assert(ra&&rb);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,ra,RAM);ae3_ee_bus_init(&b,rb,RAM);
 map(&a);map(&b);
 AE3_BootThreadNext initial=seed();AE3_BootThreadPriority x,y;
 AE3_EEThreadKernel ka,kb;ae3_ee_thread_kernel_init(&ka);ae3_ee_thread_kernel_init(&kb);
 assert(ae3_boot_thread_priority_enter(&x,&initial)==AE3_TPR_GETID_PENDING);
 assert(ae3_boot_thread_priority_enter(&y,&initial)==AE3_TPR_GETID_PENDING);
 AE3_BootThreadPriority saved=x;
 assert(ae3_boot_thread_priority_getid(&x,&a,&ka)==AE3_TPR_CONTEXT_MISSING);
 assert(!memcmp(&saved,&x,sizeof x));
 assert(ae3_ee_thread_kernel_bind_current(&ka,255,64)==0);
 assert(ae3_ee_thread_kernel_bind_current(&kb,255,64)==0);
 assert(ae3_ee_thread_kernel_bind_current(&ka,254,64)==-1);
 assert(ae3_ee_thread_kernel_get_current(&ka)==255);
 /* Original opcodes are always checked before services mutate state. */
 assert(ae3_ee_bus_write32(&a,AE3_TPR_GETID_SYSCALL,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_priority_getid(&x,&a,&ka)==AE3_TPR_UNSUPPORTED);
 assert(!memcmp(&saved,&x,sizeof x));
 assert(ae3_ee_bus_write32(&a,AE3_TPR_GETID_SYSCALL,0x0000000cu)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_priority_getid(&x,&a,&ka)==AE3_TPR_YIELDED);
 assert(ae3_boot_thread_priority_getid(&y,&b,&kb)==AE3_TPR_YIELDED);
 assert(!memcmp(&x,&y,sizeof x));
 AE3_ThreadPriorityResult result=AE3_TPR_YIELDED;
 for(unsigned i=0;i<30;i++){
  result=ae3_boot_thread_priority_slice(&x,&a,1);
  if(result!=AE3_TPR_YIELDED)break;
 }
 assert(result==AE3_TPR_PRIORITY_PENDING);
 assert(ae3_boot_thread_priority_slice(&y,&b,100)==AE3_TPR_PRIORITY_PENDING);
 assert(!memcmp(&x,&y,sizeof x)&&!memcmp(ra,rb,RAM));
 assert(x.cpu.pc==AE3_TPR_PRIORITY_SYSCALL&&x.cpu.gpr[4][0]==255u);
 assert(x.cpu.gpr[5][0]==1u&&x.cpu.gpr[3][0]==0x29u);
 saved=x;AE3_EEThreadKernel kernel_save=ka;
 assert(ae3_ee_bus_write32(&a,AE3_TPR_PRIORITY_SYSCALL,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_priority_change(&x,&a,&ka)==AE3_TPR_UNSUPPORTED);
 assert(!memcmp(&x,&saved,sizeof x)&&!memcmp(&ka,&kernel_save,sizeof ka));
 assert(ae3_ee_bus_write32(&a,AE3_TPR_PRIORITY_SYSCALL,12)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_priority_change(&x,&a,&ka)==AE3_TPR_YIELDED);
 assert(ae3_boot_thread_priority_change(&y,&b,&kb)==AE3_TPR_YIELDED);
 assert(ka.current_priority==1&&kb.current_priority==1);
 assert(!memcmp(&x,&y,sizeof x));
 /* Inject a recoverable guest-memory fault after the call return. */
 while(x.cpu.pc!=0x00436988u)
  assert(ae3_boot_thread_priority_slice(&x,&a,1)==AE3_TPR_YIELDED);
 saved=x;a.ram_size=0x00600000u;
 assert(ae3_boot_thread_priority_slice(&x,&a,1)==AE3_TPR_MEMORY_FAULT);
 assert(!memcmp(&x,&saved,sizeof x));a.ram_size=RAM;
 assert(ae3_boot_thread_priority_slice(&x,&a,100)==AE3_TPR_PARENT_RETURNED);
 assert(ae3_boot_thread_priority_slice(&y,&b,100)==AE3_TPR_PARENT_RETURNED);
 assert(!memcmp(&x,&y,sizeof x)&&!memcmp(ra,rb,RAM));
 assert(x.cpu.pc==AE3_TPR_PARENT_NEXT&&x.cpu.in_delay_slot==0);
 assert(x.cpu.retired==323496u&&x.cpu.gpr[29][0]==SP+0x90u);
 assert(x.cpu.gpr[2][0]==1u);/* saved CreateThread ID */
 assert(x.cpu.gpr[31][0]==0x0043fe90u);
 /* Continue ORIGINAL parent JAL through nested OSD configuration getter. */
 AE3_BootOsdEntry oa,ob;
 assert(ae3_boot_osd_enter(&oa,&x)==AE3_OSD_YIELDED);
 assert(ae3_boot_osd_enter(&ob,&y)==AE3_OSD_YIELDED);
 AE3_OsdEntryResult osd=AE3_OSD_YIELDED;
 for(unsigned i=0;i<50;i++){
  osd=ae3_boot_osd_slice(&oa,&a,1);
  if(osd!=AE3_OSD_YIELDED)break;
 }
 assert(osd==AE3_OSD_SYSCALL_PENDING);
 assert(ae3_boot_osd_slice(&ob,&b,100)==AE3_OSD_SYSCALL_PENDING);
 assert(!memcmp(&oa,&ob,sizeof oa)&&!memcmp(ra,rb,RAM));
 assert(oa.cpu.retired==323510u&&oa.cpu.pc==AE3_OSD_SYSCALL_PC);
 assert(oa.cpu.gpr[3][0]==0x4bu);
 assert(oa.cpu.gpr[4][0]==SP+0x90u-0x40u-0x30u);
 printf("PASS 14 original guest instructions: parent initializer reaches GetOsdConfigParam syscall %08X\n",oa.cpu.pc);
 AE3_BootOsdEntry fault_osd;
 assert(ae3_boot_osd_enter(&fault_osd,&x)==AE3_OSD_YIELDED);
 assert(ae3_boot_osd_slice(&fault_osd,&a,3)==AE3_OSD_YIELDED);
 AE3_BootOsdEntry old_osd=fault_osd;
 a.ram_size=0x01000000u;
 assert(ae3_boot_osd_slice(&fault_osd,&a,1)==AE3_OSD_MEMORY_FAULT);
 assert(!memcmp(&old_osd,&fault_osd,sizeof fault_osd));
 a.ram_size=RAM;
 assert(ae3_boot_osd_slice(&fault_osd,&a,100)==AE3_OSD_SYSCALL_PENDING);
 assert(!memcmp(&oa,&fault_osd,sizeof oa));
 /* Unknown original instructions fail instead of inventing execution. */
 assert(ae3_ee_bus_write32(&a,0x0043fe90u,0)==AE3_EE_BUS_OK);
 AE3_BootOsdEntry mutation;
 assert(ae3_boot_osd_enter(&mutation,&x)==AE3_OSD_YIELDED);
 old_osd=mutation;
 assert(ae3_boot_osd_slice(&mutation,&a,1)==AE3_OSD_UNSUPPORTED);
 assert(!memcmp(&mutation,&old_osd,sizeof mutation));
 assert(ae3_ee_bus_write32(&a,0x0043fe90u,0x0c10ffe0u)==AE3_EE_BUS_OK);
 printf("PASS 17 original guest instructions: GetThreadId ID=255, ChangeThreadPriority(255,1), return PC=%08X\n",x.cpu.pc);
 /* A guest-supplied invalid ID is NOT promoted to kernel success. */
 AE3_BootThreadPriority neg=saved;neg.cpu.pc=AE3_TPR_PRIORITY_SYSCALL;
 neg.cpu.gpr[3][0]=0x29u;neg.cpu.gpr[4][0]=17u;
 neg.cpu.gpr[5][0]=1u;neg.getid_serviced=1;neg.priority_serviced=0;
 AE3_EEThreadKernel before=ka;
 assert(ae3_boot_thread_priority_change(&neg,&a,&ka)==AE3_TPR_YIELDED);
 assert(neg.priority_result==-1&&neg.cpu.gpr[2][0]==UINT64_MAX);
 assert(!memcmp(&ka,&before,sizeof ka));
 /* The explicitly bound current ID is reserved against CreateThread reuse. */
 AE3_EEThreadKernel isolated;ae3_ee_thread_kernel_init(&isolated);
 assert(ae3_ee_thread_kernel_bind_current(&isolated,1,64)==0);
 assert(ae3_ee_thread_kernel_change_current_priority(&isolated,2,1)==-1);
 assert(isolated.current_priority==64);
 assert(ae3_ee_thread_kernel_change_current_priority(&isolated,1,128)==-1);
 assert(isolated.current_priority==64);
 free(ra);free(rb);
}
int main(void){run_happy();puts("PASS original GetThreadId/priority guest continuation and atomic fault recovery");return 0;}