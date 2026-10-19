#include "ae3_ee_timer3.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){
    AE3_EETimer3 timer;ae3_ee_timer3_reset(&timer);
    uint8_t *ram=calloc(AE3_EE_RAM_BYTES,1);assert(ram);
    AE3_EEMemoryBus bus;ae3_ee_bus_init(&bus,ram,AE3_EE_RAM_BYTES);
    uint32_t value=0xbeef;
    assert(ae3_ee_bus_read32(&bus,AE3_T3_MODE,&value)==AE3_EE_BUS_UNMAPPED);
    assert(value==0xbeef);
    ae3_ee_bus_attach_mmio(&bus,ae3_ee_timer3_read,ae3_ee_timer3_write,&timer);
    assert(ae3_ee_bus_read32(&bus,AE3_T3_MODE,&value)==AE3_EE_BUS_OK&&value==0);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_COMP,3)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_MODE,0x181)==AE3_EE_BUS_OK);
    assert(ae3_ee_timer3_advance(&timer,32*3)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_read32(&bus,AE3_T3_COUNT,&value)==AE3_EE_BUS_OK&&value==3);
    assert(ae3_ee_bus_read32(&bus,AE3_T3_MODE,&value)==AE3_EE_BUS_OK && (value&0x581)==0x581);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_MODE,0x581)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_read32(&bus,AE3_T3_MODE,&value)==AE3_EE_BUS_OK&&value==0x181);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_COUNT,0xfffe)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_MODE,0x280)==AE3_EE_BUS_OK);
    assert(ae3_ee_timer3_advance(&timer,4)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_read32(&bus,AE3_T3_COUNT,&value)==AE3_EE_BUS_OK&&value==0);
    assert(ae3_ee_bus_read32(&bus,AE3_T3_MODE,&value)==AE3_EE_BUS_OK&&(value&0x800));
    assert(ae3_ee_bus_write32(&bus,AE3_T3_MODE,0x884)==AE3_EE_BUS_OK);
    AE3_EETimer3 old=timer;
    assert(ae3_ee_timer3_advance(&timer,128)==AE3_EE_BUS_UNMAPPED);
    assert(!memcmp(&old,&timer,sizeof old));
    assert(ae3_ee_bus_write32(&bus,0x10001830,0)==AE3_EE_BUS_UNMAPPED);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_MODE,0x180)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_write32(&bus,AE3_T3_COUNT,0)==AE3_EE_BUS_OK);
    assert(ae3_ee_timer3_advance(&timer,200000)==AE3_EE_BUS_BAD_ARGUMENT);
    assert(timer.count==0);
    /* EE physical RAM and both KSEG aliases remain the same backing storage. */
    assert(ae3_ee_bus_write32(&bus,0x80001000,0x12345678)==AE3_EE_BUS_OK);
    assert(ae3_ee_bus_read32(&bus,0xA0001000,&value)==AE3_EE_BUS_OK && value==0x12345678);
    free(ram);
    puts("PASS EE T3 COUNT/MODE/COMP, divisor timing, flags W1C, unmapped MMIO, atomic unsupported clocks and KSEG aliases");
}