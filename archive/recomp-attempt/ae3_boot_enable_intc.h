#ifndef AE3_BOOT_ENABLE_INTC_H
#define AE3_BOOT_ENABLE_INTC_H
#include "ae3_boot_timer2_setup.h"
#include "ae3_ee_intc_kernel.h"
/* Original retail JAL 0x004405C8 -> wrapper 0x00436628 -> syscall 0x14.
 * Only the CP0 EIE-clear startup branch is supported. A registered cause
 * is enabled through the explicitly experimental kernel model, not real BIOS.
 * The core runs actual guest instructions, delay slots, SD/LD stack saves,
 * and the outer initializer epilogue. */
#define AE3_ENABLE_INTC_CALL UINT32_C(0x004405C8)
#define AE3_ENABLE_INTC_SYSCALL UINT32_C(0x00435824)
#define AE3_ENABLE_INTC_PARENT_RETURN UINT32_C(0x0043FE80)
#define AE3_ENABLE_INTC_EI_BOUNDARY UINT32_C(0x004405D8)
typedef enum AE3_EnableIntcResult {
    AE3_EINTC_YIELDED=1,
    AE3_EINTC_SYSCALL_PENDING=2,
    AE3_EINTC_PARENT_RETURNED=3,
    AE3_EINTC_EI_PENDING=4,
    AE3_EINTC_MEMORY_FAULT=-1,
    AE3_EINTC_UNSUPPORTED=-2,
    AE3_EINTC_INVALID=-3
} AE3_EnableIntcResult;
typedef struct AE3_BootEnableIntc {
    AE3_BootPrefix cpu;
    uint32_t cop0_status;
    uint32_t serviced_count;
    int32_t last_return;
    uint8_t active;
} AE3_BootEnableIntc;
AE3_EnableIntcResult ae3_boot_enable_intc_enter(AE3_BootEnableIntc *out,
    const AE3_BootTimer2Setup *prior);
AE3_EnableIntcResult ae3_boot_enable_intc_slice(AE3_BootEnableIntc *state,
    AE3_EEMemoryBus *bus,uint32_t budget);
AE3_EnableIntcResult ae3_boot_enable_intc_service(AE3_BootEnableIntc *state,
    AE3_EEMemoryBus *bus,AE3_EEIntcKernel *kernel);
#endif
