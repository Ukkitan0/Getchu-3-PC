#include "ae3_ee_memory_bus.h"
#include <string.h>

void ae3_ee_bus_init(AE3_EEMemoryBus *b, uint8_t *ram, size_t n) {
    if (b) {
        *b=(AE3_EEMemoryBus){0};
        b->ram=ram; b->ram_size=n;
    }
}

void ae3_ee_bus_attach_mmio(AE3_EEMemoryBus *b,
    AE3_EEMmioRead32 rd,AE3_EEMmioWrite32 wr,void *context){
    if(b){b->mmio_read32=rd;b->mmio_write32=wr;b->mmio_context=context;}
}

static int valid_span(uint32_t a, size_t n, uint32_t base, size_t max) {
    if(a<base) return 0;
    uint64_t off=(uint64_t)a-(uint64_t)base;
    return off <= max && n <= max-(size_t)off;
}

/* Resolve an entire access atomically: a span must not cross memory regions.
   The 31-MiB accelerated alias starts at physical address 0x00100000. */
static AE3_EEBusStatus resolve(const AE3_EEMemoryBus *b,
    uint32_t addr, size_t size, int write, const uint8_t **pointer) {
    if(!b || !pointer || (!b->ram && b->ram_size) ||
       b->ram_size>AE3_EE_RAM_BYTES || b->rom_size>AE3_EE_ROM_BYTES ||
       b->scratch_size>AE3_EE_SCRATCH_BYTES) return AE3_EE_BUS_BAD_ARGUMENT;
    *pointer=NULL;
    struct Span {uint32_t vbase, pbase; size_t bytes;} ram[] = {
        {UINT32_C(0x00000000),0,AE3_EE_RAM_BYTES},
        {UINT32_C(0x20000000),0,AE3_EE_RAM_BYTES},
        {UINT32_C(0x30100000),UINT32_C(0x00100000),UINT32_C(0x01F00000)},
        {UINT32_C(0x80000000),0,AE3_EE_RAM_BYTES},
        {UINT32_C(0xA0000000),0,AE3_EE_RAM_BYTES},
    };
    for(size_t i=0;i<sizeof(ram)/sizeof(ram[0]);++i) {
        if(valid_span(addr,size,ram[i].vbase,ram[i].bytes)) {
            const uint64_t physical=(uint64_t)addr-ram[i].vbase+ram[i].pbase;
            if(!b->ram || physical>b->ram_size || size>b->ram_size-(size_t)physical)
                return AE3_EE_BUS_UNMAPPED;
            *pointer=b->ram+(size_t)physical;
            return AE3_EE_BUS_OK;
        }
        /* Reject an access crossing this alias's end, not remap its tail. */
        if(addr>=ram[i].vbase && (uint64_t)addr<(uint64_t)ram[i].vbase+ram[i].bytes)
            return AE3_EE_BUS_UNMAPPED;
    }
    struct Alias {uint32_t base;} rom[] = {{UINT32_C(0x1FC00000)},
        {UINT32_C(0x9FC00000)},{UINT32_C(0xBFC00000)}};
    for(size_t i=0;i<sizeof(rom)/sizeof(rom[0]);++i) {
        if(addr>=rom[i].base && (uint64_t)addr<(uint64_t)rom[i].base+AE3_EE_ROM_BYTES) {
            if(write) return AE3_EE_BUS_READ_ONLY;
            if(!valid_span(addr,size,rom[i].base,b->rom_size) || !b->rom)
                return AE3_EE_BUS_UNMAPPED;
            *pointer=b->rom+(size_t)(addr-rom[i].base);
            return AE3_EE_BUS_OK;
        }
    }
    if(addr>=UINT32_C(0x70000000) && addr<UINT32_C(0x70004000)) {
        if(!valid_span(addr,size,UINT32_C(0x70000000),b->scratch_size) || !b->scratch)
            return AE3_EE_BUS_UNMAPPED;
        *pointer=b->scratch+(size_t)(addr-UINT32_C(0x70000000));
        return AE3_EE_BUS_OK;
    }
    return AE3_EE_BUS_UNMAPPED;
}
AE3_EEBusStatus ae3_ee_bus_read(const AE3_EEMemoryBus *b,
    uint32_t addr, void *out, size_t len) {
    if(!out || !len) return AE3_EE_BUS_BAD_ARGUMENT;
    const uint8_t *src=NULL;
    AE3_EEBusStatus st=resolve(b,addr,len,0,&src);
    if(st==AE3_EE_BUS_OK)memcpy(out,src,len);
    return st;
}
AE3_EEBusStatus ae3_ee_bus_write(AE3_EEMemoryBus *b,
    uint32_t addr, const void *src, size_t len) {
    if(!src || !len) return AE3_EE_BUS_BAD_ARGUMENT;
    const uint8_t *dst=NULL;
    AE3_EEBusStatus st=resolve(b,addr,len,1,&dst);
    if(st==AE3_EE_BUS_OK)memcpy((uint8_t*)dst,src,len);
    return st;
}
/* KSEG1 MMIO is the same physical register bank as its 0x10000000 alias.
   Only 32-bit MMIO accesses with an explicitly attached device are
   translated. Other addresses remain rejected by the ordinary bus. */
static uint32_t mmio_physical(uint32_t addr){
    if(addr>=UINT32_C(0xb0000000) && addr<UINT32_C(0xb2000000))
        return addr-UINT32_C(0xa0000000);
    return addr;
}
AE3_EEBusStatus ae3_ee_bus_read32(const AE3_EEMemoryBus *b,
    uint32_t addr, uint32_t *out) {
    if(!out)return AE3_EE_BUS_BAD_ARGUMENT;
    if(addr&3)return AE3_EE_BUS_MISALIGNED;
    uint32_t physical=mmio_physical(addr);
    if(b && b->mmio_read32 && (physical&UINT32_C(0xf0000000))==UINT32_C(0x10000000))
        return b->mmio_read32(b->mmio_context,physical,out);
    uint8_t raw[4];AE3_EEBusStatus st=ae3_ee_bus_read(b,addr,raw,4);
    if(st==AE3_EE_BUS_OK)*out=(uint32_t)raw[0]|((uint32_t)raw[1]<<8)|
        ((uint32_t)raw[2]<<16)|((uint32_t)raw[3]<<24);
    return st;
}
AE3_EEBusStatus ae3_ee_bus_write32(AE3_EEMemoryBus *b,
    uint32_t addr, uint32_t value) {
    if(addr&3)return AE3_EE_BUS_MISALIGNED;
    uint32_t physical=mmio_physical(addr);
    if(b && b->mmio_write32 && (physical&UINT32_C(0xf0000000))==UINT32_C(0x10000000))
        return b->mmio_write32(b->mmio_context,physical,value);
    uint8_t raw[4]={(uint8_t)value,(uint8_t)(value>>8),
                    (uint8_t)(value>>16),(uint8_t)(value>>24)};
    return ae3_ee_bus_write(b,addr,raw,4);
}