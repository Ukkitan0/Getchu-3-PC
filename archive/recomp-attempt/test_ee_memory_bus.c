#include "ae3_ee_memory_bus.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N (32u*1024u*1024u)
int main(void){
 uint8_t *ram=calloc(N,1);assert(ram);
 AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,N);
 uint32_t n=99;
 const uint32_t alias[]={0x00008000,0x20008000,0x80008000,0xA0008000};
 assert(ae3_ee_bus_write32(&bus,alias[0],0x89ABCDEF)==AE3_EE_BUS_OK);
 for(unsigned i=0;i<4;i++){
    assert(ae3_ee_bus_read32(&bus,alias[i],&n)==AE3_EE_BUS_OK&&n==0x89ABCDEF);
 }
 assert(ae3_ee_bus_write32(&bus,0xA0008000,0x10203040)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read32(&bus,0x80008000,&n)==AE3_EE_BUS_OK&&n==0x10203040);
 assert(ae3_ee_bus_write32(&bus,0x30100000,0x55667788)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read32(&bus,0x00100000,&n)==AE3_EE_BUS_OK&&n==0x55667788);
 for(unsigned i=0;i<4;i++){
   static const uint32_t bad[]={0x82000000,0x10000000,0xC0000000,0x70000000};
   n=123; assert(ae3_ee_bus_read32(&bus,bad[i],&n)==AE3_EE_BUS_UNMAPPED);
   assert(n==123);
 }
 assert(ae3_ee_bus_read32(&bus,0x80000001,&n)==AE3_EE_BUS_MISALIGNED);
 assert(ae3_ee_bus_write32(&bus,0x81FFFFFE,1)==AE3_EE_BUS_MISALIGNED);
 assert(ae3_ee_bus_read32(&bus,0x81FFFFFC,&n)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read32(&bus,0x9FC00000,&n)==AE3_EE_BUS_UNMAPPED);
 assert(ae3_ee_bus_write32(&bus,0xBFC00000,1)==AE3_EE_BUS_READ_ONLY);
 assert(ae3_ee_bus_read32(&bus,0xA1FFFFFC,&n)==AE3_EE_BUS_OK);
 uint32_t sentinel=0xABCDEF99;
 assert(ae3_ee_bus_read32(&bus,0xA2000000,&sentinel)==AE3_EE_BUS_UNMAPPED);
 assert(sentinel==0xABCDEF99);
 uint8_t before[4];memcpy(before,ram+N-4,4);
 const uint8_t spanning[8]={9,9,9,9,9,9,9,9};
 assert(ae3_ee_bus_write(&bus,0x81FFFFFC,spanning,8)==AE3_EE_BUS_UNMAPPED);
 assert(!memcmp(before,ram+N-4,4));
 uint8_t *rom=calloc(AE3_EE_ROM_BYTES,1);assert(rom);rom[4]=0xAB;
 bus.rom=rom;bus.rom_size=AE3_EE_ROM_BYTES;
 for(unsigned i=0;i<3;i++){
   static const uint32_t rom_alias[]={0x1FC00004,0x9FC00004,0xBFC00004};
   n=0;assert(ae3_ee_bus_read32(&bus,rom_alias[i],&n)==AE3_EE_BUS_OK && n==0xAB);
   assert(ae3_ee_bus_write32(&bus,rom_alias[i],0)==AE3_EE_BUS_READ_ONLY);
 }
 uint8_t *scratch=calloc(AE3_EE_SCRATCH_BYTES,1);assert(scratch);
 bus.scratch=scratch;bus.scratch_size=AE3_EE_SCRATCH_BYTES;
 assert(ae3_ee_bus_write32(&bus,0x70000010,0x44332211)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read32(&bus,0x70000010,&n)==AE3_EE_BUS_OK&&n==0x44332211);
 assert(ae3_ee_bus_read32(&bus,0x70004000,&n)==AE3_EE_BUS_UNMAPPED);
 free(scratch);free(rom);free(ram);
 puts("PASS EE alias bus KSEG0/KSEG1, uncached mirrors, optional BIOS and scratchpad, atomic faults");
 return 0;
}