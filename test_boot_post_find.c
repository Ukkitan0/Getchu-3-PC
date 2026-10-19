/* Original EE post-FindAddress search/reconciliation regression.
 * Tests both the original equal-address exit and an actual third syscall.
 * A successful equality fixture is synthetic kernel data, NOT a BIOS boot. */
#include "ae3_boot_post_find.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N (32u*1024u*1024u)
static uint32_t read32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void write32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8*i));}
static void write64(uint8_t *p,uint64_t v){for(unsigned i=0;i<8;i++)p[i]=(uint8_t)(v>>(8*i));}
static uint8_t *load(const char*path,size_t*n){
 FILE*f=fopen(path,"rb");assert(f);assert(!fseek(f,0,SEEK_END));long count=ftell(f);
 assert(count>0&&!fseek(f,0,SEEK_SET));uint8_t*b=malloc((size_t)count);assert(b);
 assert(fread(b,1,(size_t)count,f)==(size_t)count);fclose(f);*n=(size_t)count;return b;
}
static AE3_BootHookInit seed(AE3_EEMemoryBus *bus,int equal){
 AE3_BootHookInit hook={0};hook.guest_completed=2;hook.registered=2;
 hook.cpu.pc=AE3_POST_FIND_ENTRY;
 hook.cpu.gpr[29][0]=UINT64_C(0x01fe0000);
 hook.cpu.gpr[31][0]=0x43fdb8;
 hook.cpu.gpr[21][0]=0x440000;
 hook.cpu.gpr[20][0]=0x440000;
 /* On actual entry the search pointers are computed by ADDIU (sign-extended). */
 if(equal){
    hook.cpu.gpr[17][0]=UINT64_C(0xffffffff800000f4);
    hook.cpu.gpr[16][0]=UINT64_C(0xffffffff800000f4);
 } else {
    hook.cpu.gpr[17][0]=UINT64_C(0xfffffffffffffdf4);
    hook.cpu.gpr[16][0]=UINT64_C(0xfffffffffffffe98);
    hook.cpu.gpr[19][0]=0;
    hook.cpu.gpr[18][0]=0;
 }
 for(unsigned i=0;i<8;i++){
     uint64_t old=UINT64_C(0x1234000000000000)+i;
     /* Saved real callee-saved registers in the 0x80-byte stack frame. */
     write64(bus->ram+0x01fe0000+i*16,old);
 }
 write64(bus->ram+0x01fe0070,AE3_POST_FIND_NEXT_CALL);
 return hook;
}
static void equality_case(AE3_EEMemoryBus *bus){
 AE3_BootHookInit hook=seed(bus,1);
 AE3_BootPostFind batch,one;
 assert(ae3_boot_post_find_enter(&batch,&hook)==AE3_POST_YIELDED);
 assert(ae3_boot_post_find_enter(&one,&hook)==AE3_POST_YIELDED);
 const uint64_t before=batch.cpu.retired;
 assert(ae3_boot_post_find_slice(&batch,bus,80)==AE3_POST_NEXT_INIT);
 assert(batch.done && batch.cpu.pc==AE3_POST_FIND_NEXT_CALL);
 assert(read32(bus->ram+0x006a1e90)==0x800000f4u);
 assert(batch.cpu.gpr[29][0]==0x01fe0080ULL);
 assert(batch.cpu.retired-before==13); /* exact original branch / 8 LDs / JR */
 /* Reload original stack memory; one-instruction slices must match bytewise. */
 for(unsigned i=0;i<80;i++){
    AE3_PostFindResult status=ae3_boot_post_find_slice(&one,bus,1);
    if(status==AE3_POST_NEXT_INIT)break;
    assert(status==AE3_POST_YIELDED);
 }
 assert(!memcmp(&one,&batch,sizeof one));
 assert(ae3_boot_post_find_slice(&one,bus,1)==AE3_POST_NEXT_INIT);
}
static void search_case(AE3_EEMemoryBus *bus){
 AE3_BootHookInit hook=seed(bus,0);
 AE3_BootPostFind batch,one;
 assert(ae3_boot_post_find_enter(&batch,&hook)==AE3_POST_YIELDED);
 assert(ae3_boot_post_find_enter(&one,&hook)==AE3_POST_YIELDED);
 uint64_t start=batch.cpu.retired;
 assert(ae3_boot_post_find_slice(&batch,bus,80)==AE3_POST_FIND_REQUEST);
 assert(batch.cpu.pc==AE3_POST_FIND_SYSCALL);
 assert(batch.cpu.gpr[31][0]==0x43fde4u);
 assert((uint32_t)batch.cpu.gpr[4][0]==4u);
 assert((uint32_t)batch.cpu.gpr[5][0]==0x80080000u);
 assert((uint32_t)batch.cpu.gpr[6][0]==0x43fcc0u);
 assert(batch.cpu.retired-start==9);
 assert(batch.search_requests==1);
 for(unsigned i=0;i<40;i++){
    AE3_PostFindResult status=ae3_boot_post_find_slice(&one,bus,1);
    if(status==AE3_POST_FIND_REQUEST)break;
    assert(status==AE3_POST_YIELDED);
 }
 assert(!memcmp(&one,&batch,sizeof one));
 /* Actual original registered MIPS handler, not a synthetic returned value. */
 write32(bus->ram+4,0x43fcc0);
 AE3_GuestFindAddress guest;
 assert(ae3_boot_post_find_begin_guest(&batch,&guest)==AE3_POST_YIELDED);
 assert(ae3_guest_find_slice_bus(&guest,bus,100)==AE3_FIND_RETURNED);
 assert(guest.matched_guest_address==4u);
 assert(ae3_boot_post_find_finish_guest(&batch,&guest)==AE3_POST_YIELDED);
 assert(batch.cpu.pc==0x43fd08u);
 assert((uint32_t)batch.cpu.gpr[2][0]==4u);
 /* Now run the real return, backward branch, and another nested search. */
 AE3_PostFindResult resume=ae3_boot_post_find_slice(&batch,bus,100);
 assert(resume==AE3_POST_FIND_REQUEST);
 assert(batch.cpu.pc==AE3_POST_FIND_SYSCALL);
 assert(batch.search_requests==2);
 assert((uint32_t)batch.cpu.gpr[4][0]==8u);
 assert((uint32_t)batch.cpu.gpr[6][0]==0x43fcc0u);
 /* No silent guessed result when the third guest handler wasn't executed. */
 AE3_BootPostFind saved=batch;
 assert(ae3_boot_post_find_slice(&batch,bus,10)==AE3_POST_FIND_REQUEST);
 assert(!memcmp(&saved,&batch,sizeof saved));
}
static void fault_case(AE3_EEMemoryBus *bus){
 AE3_BootHookInit hook=seed(bus,1);
 AE3_BootPostFind task;
 assert(ae3_boot_post_find_enter(&task,&hook)==AE3_POST_YIELDED);
 /* Reach the original 64-bit LD instruction. */
 for(unsigned i=0;i<3;i++)assert(ae3_boot_post_find_slice(&task,bus,1)==AE3_POST_YIELDED);
 assert(task.cpu.pc==0x43fe1cu);
 /* Unaligned stack fault: faulting guest state and RAM remain untouched. */
 task.cpu.gpr[29][0]++;
 AE3_BootPostFind old=task;
 uint32_t first_word=read32(bus->ram+0x006a1e90);
 assert(ae3_boot_post_find_slice(&task,bus,1)==AE3_POST_MEMORY_FAULT);
 assert(!memcmp(&old,&task,sizeof task));
 assert(read32(bus->ram+0x006a1e90)==first_word);
 task.cpu.gpr[29][0]--;
 assert(ae3_boot_post_find_slice(&task,bus,1)==AE3_POST_YIELDED);
}
int main(int argc,char**argv){
 assert(argc==2);
 size_t file_size=0;uint8_t *elf=load(argv[1],&file_size);
 uint8_t *ram=calloc(N,1);assert(ram);
 AE3_ElfImage img;assert(ae3_load_ee_elf32(elf,file_size,ram,N,&img)==AE3_ELF_IMAGE_OK);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,N);
 assert(read32(ram+AE3_POST_FIND_ENTRY)==0x12300014u); /* original BEQ */
 equality_case(&bus);search_case(&bus);fault_case(&bus);
 puts("PASS actual AE3 post-FindAddress BEQ exit, loop + third original guest handler, memory faults, and instruction-slice parity");
 free(ram);free(elf);return 0;
}