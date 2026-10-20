#ifndef AE3_EE_MEMORY_BUS_H
#define AE3_EE_MEMORY_BUS_H
#include <stddef.h>
#include <stdint.h>

/* EE physical address aliases. This is a bounded kernel-mode RAM/ROM bus,
   NOT a full COP0 TLB or PS2 hardware emulator. Unknown addresses fault.
   Kernel bytes are aliases of the SAME physical EE RAM, never a private copy.
   ROM and scratchpad remain unmapped unless explicitly supplied. */
#define AE3_EE_RAM_BYTES UINT32_C(0x02000000)
#define AE3_EE_ROM_BYTES UINT32_C(0x00400000)
#define AE3_EE_SCRATCH_BYTES UINT32_C(0x00004000)

typedef enum AE3_EEBusStatus {
    AE3_EE_BUS_OK=0,
    AE3_EE_BUS_UNMAPPED=-1,
    AE3_EE_BUS_MISALIGNED=-2,
    AE3_EE_BUS_READ_ONLY=-3,
    AE3_EE_BUS_BAD_ARGUMENT=-4
} AE3_EEBusStatus;

/* An optional, explicit hardware decoder. With no decoder attached, MMIO
 * remains unmapped, preserving the conservative original boot diagnostic. */
typedef AE3_EEBusStatus (*AE3_EEMmioRead32)(void *,uint32_t,uint32_t *);
typedef AE3_EEBusStatus (*AE3_EEMmioWrite32)(void *,uint32_t,uint32_t);

typedef struct AE3_EEMemoryBus {
    AE3_EEMmioRead32 mmio_read32;
    AE3_EEMmioWrite32 mmio_write32;
    void *mmio_context;
    uint8_t *ram;
    size_t ram_size;
    const uint8_t *rom; /* Optional, never generated/faked by this module */
    size_t rom_size;
    uint8_t *scratch;
    size_t scratch_size;
} AE3_EEMemoryBus;

void ae3_ee_bus_attach_mmio(AE3_EEMemoryBus *bus,
    AE3_EEMmioRead32 read32,AE3_EEMmioWrite32 write32,void *context);
void ae3_ee_bus_init(AE3_EEMemoryBus *bus, uint8_t *ram, size_t ram_size);
AE3_EEBusStatus ae3_ee_bus_read32(const AE3_EEMemoryBus *bus,
                                  uint32_t address, uint32_t *out);
AE3_EEBusStatus ae3_ee_bus_write32(AE3_EEMemoryBus *bus,
                                   uint32_t address, uint32_t value);
AE3_EEBusStatus ae3_ee_bus_read(const AE3_EEMemoryBus *bus,
                                uint32_t address, void *out, size_t length);
AE3_EEBusStatus ae3_ee_bus_write(AE3_EEMemoryBus *bus,
                                 uint32_t address, const void *src, size_t length);
#endif