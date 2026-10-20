#include "ae3_guest_get_entry.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t*load(const char*path,size_t*n){FILE*f=fopen(path,"rb");assert(f);
 assert(!fseek(f,0,SEEK_END));long size=ftell(f);assert(size>0);
 assert(!fseek(f,0,SEEK_SET));uint8_t*b=malloc((size_t)size);assert(b);
 assert(fread(b,1,(size_t)size,f)==(size_t)size);fclose(f);*n=(size_t)size;return b;}
static AE3_GuestGetEntry make(uint32_t query){
 AE3_BootPrefix caller={0};caller.pc=0x0044032C;
 caller.gpr[3][0]=0x5b;caller.gpr[4][0]=query;caller.gpr[31][0]=0x004403D8;
 AE3_GuestGetEntry result;
 assert(ae3_guest_get_entry_init(&result,&caller,0x00440330)==AE3_GET_ENTRY_YIELDED);
 return result;
}
static void run(AE3_GuestGetEntry*s,AE3_EEMemoryBus*bus,uint32_t budget){
 AE3_GetEntryResult status=AE3_GET_ENTRY_YIELDED;
 for(unsigned i=0;i<1000;i++){
    status=ae3_guest_get_entry_slice(s,bus,budget);
    if(status==AE3_GET_ENTRY_RETURNED)break;
    assert(status==AE3_GET_ENTRY_YIELDED);
 }
 assert(status==AE3_GET_ENTRY_RETURNED&&s->cpu.pc==0x440330&&s->done);
}
int main(int argc,char**argv){
 assert(argc==2);size_t bytes=0;uint8_t*elf=load(argv[1],&bytes);
 uint8_t*ram=calloc(AE3_EE_RAM_BYTES,1);assert(ram);
 AE3_ElfImage image;
 assert(ae3_load_ee_elf32(elf,bytes,ram,AE3_EE_RAM_BYTES,&image)==AE3_ELF_IMAGE_OK);
 memcpy(ram+0x76000,ram+0x6a2678,0x740); /* retail bytes, synthetic install for isolated unit test */
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);
 static const uint32_t queries[]={0xfc,0xfe,0xfd,0xff,0x12c,0x08,0x999};
 static const uint32_t answers[]={0x80076440u,0x80076440u,0x800762a0u,
                 0x800762a0u,0x80076488u,0x800766c0u,0};
 for(unsigned i=0;i<sizeof queries/sizeof queries[0];i++){
    AE3_GuestGetEntry single=make(queries[i]),batch=make(queries[i]);
    run(&single,&bus,1);run(&batch,&bus,100);
    assert(!memcmp(&single,&batch,sizeof single));
    assert((uint32_t)single.cpu.gpr[2][0]==answers[i]);
    assert(single.table_reads>0 && single.table_reads<=8);
 }
 /* Memory faults leave the current instruction, CPU and counters untouched. */
 AE3_GuestGetEntry fault=make(0x12c);
 size_t full=bus.ram_size;bus.ram_size=0x76710; /* handler code mapped, table absent */
 assert(ae3_guest_get_entry_slice(&fault,&bus,6)==AE3_GET_ENTRY_MEMORY_FAULT);
 AE3_GuestGetEntry before=fault;
 assert(ae3_guest_get_entry_slice(&fault,&bus,1)==AE3_GET_ENTRY_MEMORY_FAULT);
 assert(!memcmp(&before,&fault,sizeof fault));
 bus.ram_size=full;run(&fault,&bus,1);assert((uint32_t)fault.cpu.gpr[2][0]==0x80076488u);
 /* Reject changed MIPS instruction without partially changing guest state. */
 AE3_GuestGetEntry corrupt=make(0xfc);
 uint32_t word=0;assert(ae3_ee_bus_read32(&bus,0x80076000,&word)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(&bus,0x80076000,0xffffffffu)==AE3_EE_BUS_OK);
 before=corrupt;
 assert(ae3_guest_get_entry_slice(&corrupt,&bus,1)==AE3_GET_ENTRY_UNSUPPORTED);
 assert(!memcmp(&before,&corrupt,sizeof corrupt));
 assert(ae3_ee_bus_write32(&bus,0x80076000,word)==AE3_EE_BUS_OK);
 puts("PASS seven real copied MIPS GetEntryAddress queries (6 hits + miss), per-instruction parity, retryable KSEG0 memory fault, modified-opcode atomic rejection");
 free(ram);free(elf);return 0;
}