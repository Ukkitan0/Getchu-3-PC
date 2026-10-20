#ifndef AE3_BOOT_INIT_PREFIX_H
#define AE3_BOOT_INIT_PREFIX_H
#include "ae3_boot_kernel_bridge.h"
#define AE3_INIT_ENTRY_JAL UINT32_C(0x0010031C)
#define AE3_INIT_FIRST_SUBCALL_PC UINT32_C(0x0043FE60)
#define AE3_INIT_FIRST_SUBCALL_TARGET UINT32_C(0x0043FC08)
typedef enum AE3_InitResult {
    AE3_INIT_YIELDED=1, AE3_INIT_SUBCALL=2,
    AE3_INIT_INVALID=-1, AE3_INIT_MEMORY_FAULT=-2, AE3_INIT_UNSUPPORTED=-3
} AE3_InitResult;
typedef struct AE3_BootInitPrefix { AE3_BootPrefix cpu; uint8_t done; } AE3_BootInitPrefix;
AE3_InitResult ae3_boot_init_enter(AE3_BootInitPrefix *state,const AE3_BootKernelBridge *kernel);
AE3_InitResult ae3_boot_init_slice(AE3_BootInitPrefix *state,uint8_t *ram,size_t bytes,uint32_t budget);
#endif