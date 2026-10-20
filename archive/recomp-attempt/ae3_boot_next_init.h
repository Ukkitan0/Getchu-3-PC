#ifndef AE3_BOOT_NEXT_INIT_H
#define AE3_BOOT_NEXT_INIT_H
#include "ae3_boot_post_find.h"
/* Retail call at 0x43FE70 -> 0x440338, stops at first EE MMIO read.
   No fake hardware register value is supplied. */
#define AE3_NEXT_INIT_JAL UINT32_C(0x0043FE70)
#define AE3_NEXT_INIT_CALLEE UINT32_C(0x00440338)
#define AE3_NEXT_INIT_MMIO_PC UINT32_C(0x00440354)
#define AE3_NEXT_INIT_MMIO_ADDRESS UINT32_C(0x10001810)
typedef enum AE3_NextInitResult {
    AE3_NEXT_YIELDED=1, AE3_NEXT_MMIO_PENDING=2,
    AE3_NEXT_MEMORY_FAULT=-1, AE3_NEXT_UNSUPPORTED=-2, AE3_NEXT_INVALID=-3
} AE3_NextInitResult;
typedef struct AE3_BootNextInit {
    AE3_BootPrefix cpu;
    uint8_t waiting_mmio;
} AE3_BootNextInit;
AE3_NextInitResult ae3_boot_next_init_enter(AE3_BootNextInit *out,
    const AE3_BootPostFind *post);
AE3_NextInitResult ae3_boot_next_init_slice(AE3_BootNextInit *state,
    AE3_EEMemoryBus *bus, uint32_t instruction_budget);
#endif