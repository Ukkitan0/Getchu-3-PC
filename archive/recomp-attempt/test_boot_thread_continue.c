#include "ae3_boot_thread_next.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM AE3_EE_RAM_BYTES
#define SP UINT32_C(0x01FDFE00)
static void map(AE3_EEMemoryBus *bus){
 const char *files[]={"retail_event_init_words.tsv","retail_thread_continue_words.tsv","retail_thread_next_words.tsv"};
 for(unsigned f=0;f<3;f++){
  FILE *in=fopen(files[f],"r");assert(in);
  char line[128];unsigned a,v;
  while(fgets(line,sizeof line,in))
    if(sscanf(line,"%x %x",&a,&v)==2)
      assert(ae3_ee_bus_write32(bus,a,v)==AE3_EE_BUS_OK);
  fclose(in);
 }
 /* The original function pointer at 0x004367E0 must be mapped. Its
  * first instruction is known from the EE snapshot. */
 assert(ae3_ee_bus_write32(bus,0x004367e0u,0x27bdff80u)==AE3_EE_BUS_OK);
}
static AE3_BootEventInit prepare(AE3_EEMemoryBus *bus,AE3_EESemaKernel *sema){
 AE3_BootTimer2Finish prior={0};prior.entered=1;
 prior.cpu.pc=AE3_T2_FINISH_PARENT_NEXT;prior.cpu.retired=323403;
 prior.cpu.gpr[29][0]=SP;prior.cpu.gpr[31][0]=0x0043fe80u;
 prior.cpu.gpr[16][0]=UINT64_C(0x1111222233334444);
 prior.cpu.gpr[17][0]=UINT64_C(0x5555666677778888);
 prior.cpu.gpr[18][0]=UINT64_C(0x9999aaaabbbbcccc);
 assert(ae3_ee_bus_write32(bus,AE3_EVENT_GUEST_STATE,0)==AE3_EE_BUS_OK);
 AE3_BootEventInit s;
 assert(ae3_boot_event_enter(&s,&prior)==AE3_EVT_YIELDED);
 assert(ae3_boot_event_slice(&s,bus,160)==AE3_EVT_SYSCALL_PENDING);
 assert(ae3_boot_event_service_sema(&s,bus,sema)==AE3_EVT_YIELDED);
 assert(s.sema_result==1);
 assert(ae3_boot_event_slice(&s,bus,160)==AE3_EVT_THREAD_SYSCALL_PENDING);
 return s;
}
static void create_success(void){
 uint8_t *ra=calloc(1,RAM),*rb=calloc(1,RAM);assert(ra&&rb);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,ra,RAM);ae3_ee_bus_init(&b,rb,RAM);
 map(&a);map(&b);
 AE3_EESemaKernel semaa,semab;ae3_ee_sema_kernel_init(&semaa);ae3_ee_sema_kernel_init(&semab);
 AE3_BootEventInit ea=prepare(&a,&semaa),eb=prepare(&b,&semab);
 AE3_BootThreadContinue sa,sb;
 AE3_EEThreadKernel ka,kb;ae3_ee_thread_kernel_init(&ka);ae3_ee_thread_kernel_init(&kb);
 assert(ae3_boot_thread_continue_enter(&sa,&ea)==AE3_THR_CREATE_PENDING);
 assert(ae3_boot_thread_continue_enter(&sb,&eb)==AE3_THR_CREATE_PENDING);
 assert(sa.cpu.pc==0x4358e4u&&sa.cpu.gpr[4][0]==SP-0x90);
 /* Service is atomic in the face of malformed instruction and unmapped RAM. */
 AE3_BootThreadContinue original=sa;AE3_EEThreadKernel empty=ka;
 assert(ae3_ee_bus_write32(&a,AE3_THR_CREATE_PC,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_create(&sa,&a,&ka)==AE3_THR_UNSUPPORTED);
 assert(!memcmp(&original,&sa,sizeof sa)&&!memcmp(&empty,&ka,sizeof ka));
 assert(ae3_ee_bus_write32(&a,AE3_THR_CREATE_PC,12)==AE3_EE_BUS_OK);
 a.ram_size=0x800000u;
 assert(ae3_boot_thread_create(&sa,&a,&ka)==AE3_THR_MEMORY_FAULT);
 assert(!memcmp(&original,&sa,sizeof sa)&&!memcmp(&empty,&ka,sizeof ka));
 a.ram_size=RAM;
 assert(ae3_boot_thread_create(&sa,&a,&ka)==AE3_THR_YIELDED);
 assert(ae3_boot_thread_create(&sb,&b,&kb)==AE3_THR_YIELDED);
 assert(sa.thread_id==1&&sa.descriptor_address==SP-0x90);
 assert(!memcmp(&sa,&sb,sizeof sa));assert(!memcmp(&ka,&kb,sizeof ka));
 assert(ae3_boot_thread_create(&sa,&a,&ka)==AE3_THR_INVALID);
 AE3_EEThread entry={0};
 assert(ae3_ee_thread_kernel_lookup(&ka,1,&entry)==0&&entry.allocated);
 assert(entry.fields[1]==0x004367e0u&&entry.fields[2]==0x00779790u);
 assert(entry.fields[3]==0x400u&&entry.fields[4]==0x00734070u);
 assert(entry.fields[5]==0u&&entry.fields[8]==0x0072b330u);
 AE3_ThreadBootResult ret=AE3_THR_YIELDED;
 for(unsigned i=0;i<80;i++){
  ret=ae3_boot_thread_continue_slice(&sa,&a,1);
  if(ret!=AE3_THR_YIELDED)break;
 }
 assert(ret==AE3_THR_NEXT_CALL);
 assert(ae3_boot_thread_continue_slice(&sb,&b,80)==AE3_THR_NEXT_CALL);
 assert(!memcmp(&sa,&sb,sizeof sa));assert(!memcmp(ra,rb,RAM));
 assert(sa.cpu.pc==AE3_THR_NEXT_CALLEE&&sa.cpu.gpr[5][0]==0x00779b98u);
 assert(sa.cpu.gpr[31][0]==0x00436974u&&sa.cpu.gpr[2][0]==0x00770000u);
 uint32_t value=123;
 assert(ae3_ee_bus_read32(&a,AE3_EVENT_GUEST_STATE,&value)==AE3_EE_BUS_OK&&value==1);
 assert(ae3_ee_bus_read32(&a,0x00779b98u,&value)==AE3_EE_BUS_OK&&value==0);
 assert(ae3_ee_bus_read32(&a,0x00779b9cu,&value)==AE3_EE_BUS_OK&&value==0);
 assert(semaa.entries[0].allocated);
 /* Enter next actual retail callee 0x436C20 with an EXPLICIT CP0 Status=0.
  * It executes the game's 0x4365B0 COP0 query and returns to GetThreadId. */
 AE3_BootThreadNext nx,ny;
 assert(ae3_boot_thread_next_enter(&nx,&sa,0)==AE3_TNEXT_YIELDED);
 assert(ae3_boot_thread_next_enter(&ny,&sb,0)==AE3_TNEXT_YIELDED);
 const uint64_t prior_count=sa.cpu.retired;
 AE3_ThreadNextResult nested=AE3_TNEXT_YIELDED;
 for(unsigned i=0;i<100;i++){
  nested=ae3_boot_thread_next_slice(&nx,&a,1);
  if(nested!=AE3_TNEXT_YIELDED)break;
 }
 assert(nested==AE3_TNEXT_GETID_PENDING);
 assert(ae3_boot_thread_next_slice(&ny,&b,100)==AE3_TNEXT_GETID_PENDING);
 assert(!memcmp(&nx,&ny,sizeof nx)&&!memcmp(ra,rb,RAM));
 assert(nx.cpu.retired==prior_count+23u);
 assert(nx.cpu.pc==AE3_TNEXT_GET_THREAD_ID);
 assert(nx.cpu.gpr[3][0]==0x2fu && nx.cpu.gpr[29][0]==sa.cpu.gpr[29][0]);
 assert(nx.cpu.gpr[2][0]==UINT64_MAX);
 printf("PASS next original 0x436C20 and COP0 query: 23 instructions to GetThreadId syscall 0x2F at %08X\n",nx.cpu.pc);
 /* CP0 Status bit0 set does not run unimplemented interrupt-sensitive code. */
 AE3_BootThreadNext critical;
 assert(ae3_boot_thread_next_enter(&critical,&sa,1)==AE3_TNEXT_YIELDED);
 assert(ae3_boot_thread_next_slice(&critical,&a,100)==AE3_TNEXT_CRITICAL_PENDING);
 assert(critical.cpu.pc==AE3_TNEXT_CRITICAL_CALL);
 /* Invalid host/guest COP0 EIE cannot silently enter the narrow translator. */
 assert(ae3_boot_thread_next_enter(&critical,&sa,0x10000u)==AE3_TNEXT_INVALID);
 printf("PASS CreateThread(0x20) id=1 original code reaches %08X: %llu instructions after syscall boundary\n",
 sa.cpu.pc,(unsigned long long)(sa.cpu.retired-ea.cpu.retired));
 free(ra);free(rb);
}
static void failure_cleanup(void){
 uint8_t *ram=calloc(1,RAM);assert(ram);AE3_EEMemoryBus bus;
 ae3_ee_bus_init(&bus,ram,RAM);map(&bus);
 AE3_EESemaKernel sema;ae3_ee_sema_kernel_init(&sema);
 AE3_BootEventInit event=prepare(&bus,&sema);
 AE3_BootThreadContinue s;AE3_EEThreadKernel threads;
 ae3_ee_thread_kernel_init(&threads);
 /* Exhaust compatibility pool; CreateThread returns a generic negative code. */
 for(unsigned i=0;i<AE3_EE_THREAD_CAPACITY;i++)threads.entries[i].allocated=1;
 AE3_EEThreadKernel before=threads;
 assert(ae3_boot_thread_continue_enter(&s,&event)==AE3_THR_CREATE_PENDING);
 assert(ae3_boot_thread_create(&s,&bus,&threads)==AE3_THR_YIELDED);
 assert(s.thread_id==-1&&!memcmp(&before,&threads,sizeof threads));
 assert(ae3_boot_thread_continue_slice(&s,&bus,90)==AE3_THR_DELETE_PENDING);
 assert(s.cpu.gpr[3][0]==0x41&&s.cpu.gpr[4][0]==1);
 uint32_t value=0;
 assert(ae3_ee_bus_read32(&bus,AE3_EVENT_GUEST_STATE,&value)==AE3_EE_BUS_OK&&value==UINT32_MAX);
 AE3_BootThreadContinue old=s;AE3_EESemaKernel old_sema=sema;
 assert(ae3_ee_bus_write32(&bus,AE3_THR_DELETE_PC,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_delete_sema(&s,&bus,&sema)==AE3_THR_UNSUPPORTED);
 assert(!memcmp(&s,&old,sizeof s)&&!memcmp(&sema,&old_sema,sizeof sema));
 assert(ae3_ee_bus_write32(&bus,AE3_THR_DELETE_PC,12)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_delete_sema(&s,&bus,&sema)==AE3_THR_YIELDED);
 assert(!sema.entries[0].allocated);
 assert(ae3_boot_thread_delete_sema(&s,&bus,&sema)==AE3_THR_INVALID);
 assert(ae3_boot_thread_continue_slice(&s,&bus,100)==AE3_THR_RETURNED);
 assert(s.cpu.pc==AE3_THR_PARENT_NEXT&&s.cpu.gpr[29][0]==SP);
 assert(s.cpu.gpr[2][0]==UINT64_MAX);
 printf("PASS CreateThread failure -> original DeleteSema(0x41) cleanup -> return %08X\n",s.cpu.pc);
 free(ram);
}
static void fault_recovery(void){
 uint8_t *ram=calloc(1,RAM);assert(ram);AE3_EEMemoryBus bus;
 ae3_ee_bus_init(&bus,ram,RAM);map(&bus);
 AE3_EESemaKernel sema;ae3_ee_sema_kernel_init(&sema);
 AE3_BootEventInit e=prepare(&bus,&sema);
 AE3_BootThreadContinue s;AE3_EEThreadKernel k;ae3_ee_thread_kernel_init(&k);
 assert(ae3_boot_thread_continue_enter(&s,&e)==AE3_THR_CREATE_PENDING);
 assert(ae3_boot_thread_create(&s,&bus,&k)==AE3_THR_YIELDED);
 while(s.cpu.pc!=0x00436940u)
    assert(ae3_boot_thread_continue_slice(&s,&bus,1)==AE3_THR_YIELDED);
 AE3_BootThreadContinue saved=s;
 assert(ae3_ee_bus_write32(&bus,0x00436940u,0)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_continue_slice(&s,&bus,1)==AE3_THR_UNSUPPORTED);
 assert(!memcmp(&s,&saved,sizeof s));
 assert(ae3_ee_bus_write32(&bus,0x00436940u,0x0040202du)==AE3_EE_BUS_OK);
 while(s.cpu.pc!=0x00436948u)
    assert(ae3_boot_thread_continue_slice(&s,&bus,1)==AE3_THR_YIELDED);
 saved=s;bus.ram_size=0x00400000u;
 assert(ae3_boot_thread_continue_slice(&s,&bus,1)==AE3_THR_MEMORY_FAULT);
 assert(!memcmp(&s,&saved,sizeof s));
 bus.ram_size=RAM;
 assert(ae3_boot_thread_continue_slice(&s,&bus,50)==AE3_THR_NEXT_CALL);
 printf("PASS opcode mutation and guest-store fault leave CreateThread continuation recoverable\n");
 free(ram);
}
static void validation(void){
 uint8_t *ram=calloc(1,RAM);assert(ram);AE3_EEMemoryBus bus;
 ae3_ee_bus_init(&bus,ram,RAM);map(&bus);
 AE3_EESemaKernel sema;ae3_ee_sema_kernel_init(&sema);
 AE3_BootEventInit e=prepare(&bus,&sema);
 AE3_BootThreadContinue s;AE3_EEThreadKernel k;ae3_ee_thread_kernel_init(&k);
 assert(ae3_boot_thread_continue_enter(&s,&e)==AE3_THR_CREATE_PENDING);
 uint32_t where=(uint32_t)s.cpu.gpr[4][0],size=0;
 assert(ae3_ee_bus_read32(&bus,where+12u,&size)==AE3_EE_BUS_OK&&size==0x400u);
 assert(ae3_ee_bus_write32(&bus,where+12u,7)==AE3_EE_BUS_OK);
 assert(ae3_boot_thread_create(&s,&bus,&k)==AE3_THR_YIELDED);
 assert(s.thread_id==-1&&k.generation==0);
 /* A negative compatibility kernel response must not be silently promoted
  * to an allocated thread; no native scheduling or guest thread execution. */
 AE3_EEThread record;
 assert(ae3_ee_thread_kernel_lookup(&k,1,&record)!=0);
 puts("PASS invalid thread descriptor gives negative CreateThread without registry mutation");
 free(ram);
}
int main(void){create_success();failure_cleanup();fault_recovery();validation();
 puts("PASS original guest CreateThread return / cleanup + single-step parity and faults");return 0;}