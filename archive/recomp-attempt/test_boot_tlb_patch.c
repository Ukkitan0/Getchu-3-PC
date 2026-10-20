/* Deterministic fixtures never include retail game data or a BIOS.
 * Instruction opcodes are independently pinned to the user's EE RAM snapshot. */
#include "ae3_boot_tlb_finalize.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STACK UINT32_C(0x01fdfdf0)
static void setup(AE3_EEMemoryBus *bus){
 FILE *f=fopen("retail_tlb_patch_words.tsv","r");assert(f);
 char line[128];unsigned pc,word;
 while(fgets(line,sizeof line,f))if(sscanf(line,"%x %x",&pc,&word)==2)
   assert(ae3_ee_bus_write32(bus,pc,word)==AE3_EE_BUS_OK);
 fclose(f);
 const uint32_t table[]={0x5a,0x0043f418,0x5b,0x80075000,0x54,
                           0x0043f980,0x55,0,0x56,0,0x57,0,
                           0x58,0,0x59,0};
 for(unsigned i=0;i<sizeof table/sizeof table[0];i++)
   assert(ae3_ee_bus_write32(bus,0x6a1bc0u+i*4u,table[i])==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(bus,0x0043f980u,0x03e00008u)==AE3_EE_BUS_OK);
 for(unsigned i=0;i<AE3_TLB_COPY_BYTES/4u;i++)
  assert(ae3_ee_bus_write32(bus,AE3_TLB_COPY_SRC+4u*i,
      0xabc00000u^(i*0x01020408u))==AE3_EE_BUS_OK);
 /* Replace synthetic filler with actual retail GetEntry MIPS instructions
  * and five table entries plus the terminal syscall-3 entry. This is a
  * minimal instruction fixture, NOT the retail 816-byte binary payload. */
 f=fopen("retail_tlb_finalize_words.tsv","r");assert(f);
 while(fgets(line,sizeof line,f))if(sscanf(line,"%x %x",&pc,&word)==2)
   assert(ae3_ee_bus_write32(bus,pc,word)==AE3_EE_BUS_OK);
 fclose(f);
 const uint32_t entries[]={0x55,0x80075038,0x56,0x800750c8,
   0x57,0x80075108,0x58,0x80075158,0x59,0x800751a8,0x03,0x80075330};
 for(unsigned i=0;i<sizeof entries/sizeof entries[0];i++)
   assert(ae3_ee_bus_write32(bus,AE3_TLB_COPY_SRC+0x300u+i*4u,
       entries[i])==AE3_EE_BUS_OK);
}
static AE3_BootOsdRoundtrip seed(void){
 AE3_BootOsdRoundtrip o={0};o.active=1;o.read_count=2;o.write_count=2;
 o.cpu.pc=AE3_TLB_GUEST_ENTRY;o.cpu.retired=323554u;
 o.cpu.gpr[29][0]=STACK;o.cpu.gpr[31][0]=0x0043fe98u;
 o.cpu.gpr[16][0]=0x61616161u;o.cpu.gpr[17][0]=0x71717171u;
 o.cpu.gpr[18][0]=0x81818181u;
 return o;
}
static void run(void){
 uint8_t *mem1=calloc(1,AE3_EE_RAM_BYTES),*mem2=calloc(1,AE3_EE_RAM_BYTES);
 assert(mem1&&mem2);
 AE3_EEMemoryBus a,b;ae3_ee_bus_init(&a,mem1,AE3_EE_RAM_BYTES);
 ae3_ee_bus_init(&b,mem2,AE3_EE_RAM_BYTES);setup(&a);setup(&b);
 AE3_BootOsdRoundtrip o=seed();AE3_BootTlbPatch one,many;
 assert(ae3_boot_tlb_enter(&one,&o)==AE3_TLB_YIELDED);
 assert(ae3_boot_tlb_enter(&many,&o)==AE3_TLB_YIELDED);
 AE3_EESyscallKernel k1,k2;ae3_ee_syscall_kernel_init(&k1);
 ae3_ee_syscall_kernel_init(&k2);
 assert(ae3_ee_syscall_register_bus(&k1,0x5a,0x0043f408u,&a)==0);
 assert(ae3_ee_syscall_register_bus(&k2,0x5a,0x0043f408u,&b)==0);
 unsigned sets=0,flush=0,copies=0,fault_checked=0;
 AE3_TlbResult result=AE3_TLB_YIELDED;
 for(unsigned tick=0;tick<5000;tick++){
   /* One instruction vs run-to-next-syscall batching: compare at boundaries. */
   result=ae3_boot_tlb_slice(&one,&a,1);
   if(result==AE3_TLB_YIELDED)continue;
   assert(ae3_boot_tlb_slice(&many,&b,4096)==result);
   assert(!memcmp(&one,&many,sizeof one));
   assert(!memcmp(mem1,mem2,AE3_EE_RAM_BYTES));
   assert(!memcmp(&k1,&k2,sizeof k1));
   if(result==AE3_TLB_GET_ENTRY_PENDING)break;
   if(result<0)fprintf(stderr,"FAULT pc=%08X status=%d retired=%llu copywords=%u delay=%u branch=%08X\n",one.cpu.pc,result,(unsigned long long)one.cpu.retired,one.copy_words,one.cpu.in_delay_slot,one.cpu.branch_target);
   assert(result==AE3_TLB_SET_PENDING||result==AE3_TLB_COPY_PENDING||
          result==AE3_TLB_FLUSH_PENDING);
   AE3_BootTlbPatch before=one;
   assert(ae3_ee_bus_write32(&a,one.cpu.pc,0)==AE3_EE_BUS_OK);
   assert(ae3_boot_tlb_service(&one,&a,&k1)==AE3_TLB_UNSUPPORTED);
   assert(!memcmp(&one,&before,sizeof one));
   assert(ae3_ee_bus_write32(&a,one.cpu.pc,12u)==AE3_EE_BUS_OK);
   if(result==AE3_TLB_COPY_PENDING){
      a.ram_size=AE3_TLB_COPY_SRC;
      assert(ae3_boot_tlb_service(&one,&a,&k1)==AE3_TLB_MEMORY_FAULT);
      assert(!memcmp(&one,&before,sizeof one));
      a.ram_size=AE3_EE_RAM_BYTES;copies++;
   }else if(result==AE3_TLB_SET_PENDING)sets++;
   else flush++;
   {AE3_TlbResult srv=ae3_boot_tlb_service(&one,&a,&k1);if(srv!=AE3_TLB_YIELDED){fprintf(stderr,"FAIL event=%d service=%d pc=%08X a0=%08X a1=%08X v1=%08X regs=%u flush=%u copy=%u\n",result,srv,one.cpu.pc,(unsigned)one.cpu.gpr[4][0],(unsigned)one.cpu.gpr[5][0],(unsigned)one.cpu.gpr[3][0],one.registrations,one.flushes,one.copy_done);}assert(srv==AE3_TLB_YIELDED);}
   assert(ae3_boot_tlb_service(&many,&b,&k2)==AE3_TLB_YIELDED);
   assert(!memcmp(&one,&many,sizeof one));
   assert(!memcmp(&k1,&k2,sizeof k1));
 }
 assert(result==AE3_TLB_GET_ENTRY_PENDING);
 assert(sets==3&&copies==1&&flush==2);
 assert(one.cpu.pc==AE3_TLB_GUEST_GET_ENTRY);
 assert(one.copy_done==1&&one.copy_active==0&&one.copy_words==204);
 assert(one.registrations==3&&one.flushes==2&&one.flush_modes[0]==0&&
        one.flush_modes[1]==2);
 assert((uint32_t)one.cpu.gpr[4][0]==0x55u);
 uint32_t handler=0;
 assert(ae3_ee_syscall_lookup(&k1,0x5a,&handler)==0&&handler==0x43f418u);
 assert(ae3_ee_syscall_lookup(&k1,0x5b,&handler)==0&&handler==AE3_TLB_COPY_DST);
 assert(ae3_ee_syscall_lookup(&k1,0x54,&handler)==0&&handler==0x43f980u);
 assert(!memcmp(mem1,mem2,AE3_EE_RAM_BYTES));
 for(unsigned i=0;i<AE3_TLB_COPY_BYTES/4u;i++){
  uint32_t src=0,dst=0;
  assert(ae3_ee_bus_read32(&a,AE3_TLB_COPY_SRC+i*4u,&src)==AE3_EE_BUS_OK);
  assert(ae3_ee_bus_read32(&a,AE3_TLB_COPY_DST+i*4u,&dst)==AE3_EE_BUS_OK);
  assert(src==dst);
 }
 assert(ae3_boot_tlb_slice(&one,&a,10)==AE3_TLB_GET_ENTRY_PENDING);
 printf("PASS retail TLB function installer: %llu real guest instructions, 204 original Copy SW, 3 SetSyscall, 2 FlushCache, GetEntryAddress pending %08X\n",
  (unsigned long long)(one.cpu.retired-o.cpu.retired),one.cpu.pc);
 /* Execute six ORIGINAL copied GetEntryAddress MIPS table searches, five
    SetSyscall registrations, and the full original return/stack epilogue. */
 AE3_BootTlbFinalize fa,fb;
 assert(ae3_boot_tlb_finish_enter(&fa,&one)==AE3_TLBF_GET_ENTRY);
 assert(ae3_boot_tlb_finish_enter(&fb,&many)==AE3_TLBF_GET_ENTRY);
 AE3_TlbFinishResult fin=AE3_TLBF_YIELDED;
 unsigned get_calls=0,set_calls=0;
 for(unsigned tick=0;tick<4000;tick++){
   fin=ae3_boot_tlb_finish_slice(&fa,&a,1);
   if(fin==AE3_TLBF_YIELDED)continue;
   AE3_TlbFinishResult batch=ae3_boot_tlb_finish_slice(&fb,&b,4096);
   if(fin!=batch)fprintf(stderr,"FINISH divergence %d/%d pc=%08X/%08X\n",fin,batch,fa.cpu.pc,fb.cpu.pc);
   assert(fin==batch);
   assert(!memcmp(&fa,&fb,sizeof fa));
   assert(!memcmp(mem1,mem2,AE3_EE_RAM_BYTES));
   assert(!memcmp(&k1,&k2,sizeof k1));
   if(fin==AE3_TLBF_RETURNED)break;
   if(fin!=AE3_TLBF_GET_ENTRY&&fin!=AE3_TLBF_SET_SYSCALL)
     fprintf(stderr,"FINISH fault %d pc=%08X entries=%u sets=%u delayed=%u ra=%08X\n",fin,fa.cpu.pc,fa.entries,fa.sets,fa.cpu.in_delay_slot,(unsigned)fa.cpu.gpr[31][0]);
   assert(fin==AE3_TLBF_GET_ENTRY||fin==AE3_TLBF_SET_SYSCALL);
   AE3_BootTlbFinalize saved=fa;
   uint32_t word_saved=0;
   assert(ae3_ee_bus_read32(&a,fa.cpu.pc,&word_saved)==AE3_EE_BUS_OK);
   assert(ae3_ee_bus_write32(&a,fa.cpu.pc,0)==AE3_EE_BUS_OK);
   assert(ae3_boot_tlb_finish_service(&fa,&a,&k1)==AE3_TLBF_UNSUPPORTED);
   assert(!memcmp(&fa,&saved,sizeof fa));
   assert(ae3_ee_bus_write32(&a,fa.cpu.pc,word_saved)==AE3_EE_BUS_OK);
   if(fin==AE3_TLBF_GET_ENTRY)get_calls++;else set_calls++;
   {AE3_TlbFinishResult srv=ae3_boot_tlb_finish_service(&fa,&a,&k1);if(srv!=AE3_TLBF_YIELDED)fprintf(stderr,"FINISH service failed=%d evt=%d pc=%08X a0=%08X a1=%08X v1=%08X entries=%u sets=%u\n",srv,fin,fa.cpu.pc,(unsigned)fa.cpu.gpr[4][0],(unsigned)fa.cpu.gpr[5][0],(unsigned)fa.cpu.gpr[3][0],fa.entries,fa.sets);assert(srv==AE3_TLBF_YIELDED);}
   assert(ae3_boot_tlb_finish_service(&fb,&b,&k2)==AE3_TLBF_YIELDED);
   assert(!memcmp(&fa,&fb,sizeof fa));
 }
 assert(fin==AE3_TLBF_RETURNED && get_calls==6 && set_calls==5);
 assert(fa.cpu.pc==AE3_TLB_FINAL_RETURN&&fa.entries==6&&fa.sets==5);
 uint32_t final_entry=0;
 assert(ae3_ee_bus_read32(&a,0x6a1bb8u,&final_entry)==AE3_EE_BUS_OK);
 assert(final_entry==0x80075330u);
 for(unsigned i=0;i<5;i++){
   assert(ae3_ee_syscall_lookup(&k1,0x55u+i,&handler)==0);
   assert(handler==((const uint32_t[]){0x80075038u,0x800750c8u,
      0x80075108u,0x80075158u,0x800751a8u})[i]);
 }
 assert(!memcmp(mem1,mem2,AE3_EE_RAM_BYTES));
 assert(!memcmp(&k1,&k2,sizeof k1));
 printf("PASS TLB GetEntry: %llu more original MIPS instructions, 6 searches, 5 SetSyscall, final guest returned at %08X\n",
     (unsigned long long)(fa.cpu.retired-one.cpu.retired),fa.cpu.pc);
  /* At guest first load, missing EE RAM is an ordinary retryable fault. */
 AE3_BootTlbPatch missing;
 assert(ae3_boot_tlb_enter(&missing,&o)==AE3_TLB_YIELDED);
 uint32_t opcode=0;
 assert(ae3_ee_bus_read32(&a,AE3_TLB_GUEST_ENTRY,&opcode)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(&a,AE3_TLB_GUEST_ENTRY,0)==AE3_EE_BUS_OK);
 AE3_BootTlbPatch snapshot=missing;
 assert(ae3_boot_tlb_slice(&missing,&a,1)==AE3_TLB_UNSUPPORTED);
 assert(!memcmp(&missing,&snapshot,sizeof missing));
 assert(ae3_ee_bus_write32(&a,AE3_TLB_GUEST_ENTRY,opcode)==AE3_EE_BUS_OK);
 a.ram_size=AE3_TLB_GUEST_ENTRY;
 assert(ae3_boot_tlb_slice(&missing,&a,1)==AE3_TLB_MEMORY_FAULT);
 assert(!memcmp(&missing,&snapshot,sizeof missing));
 a.ram_size=AE3_EE_RAM_BYTES;
 assert(ae3_boot_tlb_slice(&missing,&a,1)==AE3_TLB_YIELDED);
 (void)fault_checked;
 free(mem1);free(mem2);
}
int main(void){run();puts("PASS: instruction-verified TLB copy and registry update, no synthetic BIOS execution");return 0;}