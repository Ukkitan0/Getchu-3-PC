#ifndef AE3_BOOT_TIMER2_SETUP_H
#define AE3_BOOT_TIMER2_SETUP_H
#include "ae3_boot_timer2_continue.h"
/* Run the REAL retail SetT2_COUNT/COMP/MODE wrappers and their shared guest
   COP0/ERET MMIO store routine. Narrowly supports explicit EIE=0 guest COP0
   status. Does not claim to emulate arbitrary CP0 interrupts or exceptions. */
#define AE3_T2_GUEST_NEXT_ENABLE_INTC UINT32_C(0x004405C8)
typedef enum AE3_BootTimer2SetupResult {
 AE3_BT2S_YIELDED=1,AE3_BT2S_ENABLE_INTC_PENDING=2,
 AE3_BT2S_MEMORY_FAULT=-1,AE3_BT2S_UNSUPPORTED=-2,AE3_BT2S_INVALID=-3
} AE3_BootTimer2SetupResult;
typedef struct AE3_BootTimer2Setup {
 AE3_BootPrefix cpu;
 uint32_t cop0_status,error_epc;
 uint32_t io_stores;
 uint8_t active;
} AE3_BootTimer2Setup;
AE3_BootTimer2SetupResult ae3_boot_timer2_setup_enter(AE3_BootTimer2Setup *out,
 const AE3_BootTimer2Continue *prior);
AE3_BootTimer2SetupResult ae3_boot_timer2_setup_slice(
 AE3_BootTimer2Setup *state,AE3_EEMemoryBus *bus,uint32_t budget);
#endif