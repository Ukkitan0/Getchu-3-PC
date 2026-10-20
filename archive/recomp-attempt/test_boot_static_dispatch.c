/* Standalone, pinned-opcode original guest program fixture. No ISO or BIOS. */
#include "ae3_boot_static_dispatch.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void install(AE3_EEMemoryBus *bus){
    FILE *f=fopen("retail_static_dispatch_words.tsv","r");assert(f);
    unsigned address,op;int scanned;
    while((scanned=fscanf(f,"%x %x",&address,&op))==2){
      assert(ae3_ee_bus_write32(bus,address,op)==AE3_EE_BUS_OK);
    }
    assert(scanned==EOF);fclose(f);
}
static AE3_BootMainResume seed(void){
    AE3_BootMainResume s={0};s.active=1;s.flush_requests=1;
    s.cpu.pc=AE3_STATIC_INIT_ENTRY;
    s.cpu.gpr[16][0]=UINT64_C(0x00628210);
    s.cpu.gpr[29][0]=UINT64_C(0x01fed000);
    s.cpu.gpr[31][0]=UINT64_C(0x0010032c);
    s.cpu.retired=329097;
    return s;
}
static void run_fixture(uint32_t ctor_word){
    uint8_t *ram1=calloc(1,AE3_EE_RAM_BYTES),*ram2=calloc(1,AE3_EE_RAM_BYTES);
    assert(ram1&&ram2);
    AE3_EEMemoryBus a,b;
    ae3_ee_bus_init(&a,ram1,AE3_EE_RAM_BYTES);
    ae3_ee_bus_init(&b,ram2,AE3_EE_RAM_BYTES);
    install(&a);install(&b);
    assert(ae3_ee_bus_write32(&a,AE3_STATIC_CONSTRUCTOR_TABLE,ctor_word)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_write32(&b,AE3_STATIC_CONSTRUCTOR_TABLE,ctor_word)==AE3_EE_BUS_OK);
    AE3_BootMainResume before=seed();
    AE3_BootStaticDispatch first,second;
    assert(ae3_boot_static_enter(&first,&before)==AE3_STATIC_YIELDED);
    assert(ae3_boot_static_enter(&second,&before)==AE3_STATIC_YIELDED);
    before.cpu.gpr[16][0]=0;
    assert(ae3_boot_static_enter(&second,&before)==AE3_STATIC_INVALID);
    before.cpu.gpr[16][0]=UINT64_C(0x00628210);
    assert(ae3_boot_static_enter(&second,&before)==AE3_STATIC_YIELDED);
    AE3_BootStaticDispatch saved=first;
    assert(ae3_ee_bus_write32(&a,first.cpu.pc,0)==AE3_EE_BUS_OK);
    assert(ae3_boot_static_slice(&first,&a,1)==AE3_STATIC_UNSUPPORTED);
    assert(!memcmp(&first,&saved,sizeof first));
    assert(ae3_ee_bus_write32(&a,first.cpu.pc,UINT32_C(0x0200f809))==AE3_EE_BUS_OK);
    a.ram_size=first.cpu.pc;
    assert(ae3_boot_static_slice(&first,&a,1)==AE3_STATIC_MEMORY_FAULT);
    assert(!memcmp(&first,&saved,sizeof first));
    a.ram_size=AE3_EE_RAM_BYTES;
    AE3_StaticInitResult result=AE3_STATIC_YIELDED;
    for(unsigned step=0;step<200;step++){
      result=ae3_boot_static_slice(&first,&a,1);
      if(result!=AE3_STATIC_YIELDED)break;
    }
    const AE3_StaticInitResult wanted=ctor_word==UINT32_C(0xffffffff)?
        AE3_STATIC_NEXT_CALL:AE3_STATIC_CONSTRUCTOR_PENDING;
    assert(result==wanted);
    assert(ae3_boot_static_slice(&second,&b,300)==wanted);
    assert(!memcmp(&first,&second,sizeof first));
    assert(!memcmp(ram1,ram2,AE3_EE_RAM_BYTES));
    if(wanted==AE3_STATIC_NEXT_CALL){
      assert(first.cpu.pc==AE3_STATIC_NEXT_JAL);
      assert(first.cpu.gpr[4][0]==UINT64_C(0x00628234));
      assert(first.cpu.gpr[29][0]==before.cpu.gpr[29][0]);
      assert(first.cpu.gpr[16][0]==UINT64_C(0x00628210));
      assert(first.cpu.gpr[31][0]==UINT64_C(0x00100344));
      assert(first.registered_zero_path && first.empty_ctor_path);
      printf("PASS retail static initializer: %llu guest instructions, next JAL %08X to %08X\n",
         (unsigned long long)(first.cpu.retired-before.cpu.retired),first.cpu.pc,AE3_STATIC_NEXT_CALLEE);
    }else{
      assert(first.cpu.pc==UINT32_C(0x00428ae8));
      assert(first.guest_ctor_word==ctor_word);
      assert(!first.empty_ctor_path);
      printf("PASS original guest constructor callback pending at %08X, pointer %08X (NOT invoked)\n",
        first.cpu.pc,first.guest_ctor_word);
    }
    free(ram1);free(ram2);
}
static void check_registration_branch(void){
    uint8_t *ram=calloc(1,AE3_EE_RAM_BYTES);assert(ram);
    AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);install(&bus);
    assert(ae3_ee_bus_write32(&bus,UINT32_C(0x006b7f90),UINT32_C(0x00100000))==AE3_EE_BUS_OK);
    AE3_BootMainResume pre=seed();AE3_BootStaticDispatch x;
    assert(ae3_boot_static_enter(&x,&pre)==AE3_STATIC_YIELDED);
    AE3_StaticInitResult evt=AE3_STATIC_YIELDED;
    for(int i=0;i<100;i++){
       evt=ae3_boot_static_slice(&x,&bus,1);
       if(evt!=AE3_STATIC_YIELDED)break;
    }
    assert(evt==AE3_STATIC_DYNAMIC_REGISTRATION_PENDING);
    assert(x.cpu.pc==UINT32_C(0x00427090));
    assert(x.registered_zero_path==0);
    free(ram);
}
static void check_constructor_table(void){
 uint8_t *ram=calloc(1,AE3_EE_RAM_BYTES);assert(ram);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);
 FILE *f=fopen("retail_constructor_addresses.tsv","r");assert(f);
 unsigned a,v;unsigned count=0;
 while(fscanf(f,"%x %x",&a,&v)==2){
    assert(ae3_ee_bus_write32(&bus,a,v)==AE3_EE_BUS_OK);count++;
 }
 fclose(f);assert(count==144);
 AE3_BootConstructorList list={0};
 assert(ae3_boot_static_scan_constructors(&list,&bus)==AE3_STATIC_YIELDED);
 assert(list.count==143u);
 assert(list.entries[0]==UINT32_C(0x0041a390));
 assert(list.entries[142]!=0u);
 for(unsigned i=0;i<list.count;i++){
    assert((list.entries[i]&3u)==0&&list.entries[i]>=0x00100000u);
 }
 AE3_BootConstructorList previous=list;
 /* Unmapped table read and invalid pointer must not alter output. */
 bus.ram_size=0x62e89cu;
 assert(ae3_boot_static_scan_constructors(&list,&bus)==AE3_STATIC_MEMORY_FAULT);
 assert(!memcmp(&list,&previous,sizeof list));
 bus.ram_size=AE3_EE_RAM_BYTES;
 assert(ae3_ee_bus_write32(&bus,AE3_STATIC_CONSTRUCTOR_TABLE,3)==AE3_EE_BUS_OK);
 assert(ae3_boot_static_scan_constructors(&list,&bus)==AE3_STATIC_UNSUPPORTED);
 assert(!memcmp(&list,&previous,sizeof list));
 printf("PASS recovered constructor table: %u retail callbacks, correct terminal sentinel, atomic scan\n",previous.count);
 free(ram);
}
int main(void){
 run_fixture(UINT32_C(0xffffffff));
 run_fixture(UINT32_C(0x0041a390)); /* First recovered real constructor. */
 check_registration_branch();
 check_constructor_table();
 puts("PASS bounded original JALR/static startup; no implicit callback or BIOS state");
 return 0;
}