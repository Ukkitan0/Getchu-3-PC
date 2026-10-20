#ifndef AE3_BOOT_TIMER2_CONTINUE_H
#define AE3_BOOT_TIMER2_CONTINUE_H
#include "ae3_boot_intc_critical.h"
#include "ae3_ee_timer2.h"
/* Execute ONLY the retail 0x00440590..0x004405AC instruction prefix,
 * preserving branch/delay-slot semantics. Halt BEFORE the next guest JAL.
 * The MMIO read must be supplied by a mapped EE Timer 2 device, never by an
 * invented return value. */
#define AE3_T2_GUEST_COUNT_SETUP UINT32_C(0x004405B0)
#define AE3_T2_GUEST_MODE_SETUP UINT32_C(0x004405C0)
typedef enum AE3_BootTimer2Result {
 AE3_BT2_YIELDED=1,
 AE3_BT2_TIMER_SETUP_CALL=2,
 AE3_BT2_MODE_SETUP_CALL=3,
 AE3_BT2_MEMORY_FAULT=-1,
 AE3_BT2_UNSUPPORTED=-2,
 AE3_BT2_INVALID=-3
} AE3_BootTimer2Result;
typedef struct AE3_BootTimer2Continue {AE3_BootPrefix cpu;uint32_t cop0_status;uint8_t entered;} AE3_BootTimer2Continue;
AE3_BootTimer2Result ae3_boot_timer2_enter(AE3_BootTimer2Continue *out,
 const AE3_BootIntcCritical *prior);
AE3_BootTimer2Result ae3_boot_timer2_slice(AE3_BootTimer2Continue *state,
 const AE3_EEMemoryBus *bus,uint32_t budget);
#endif