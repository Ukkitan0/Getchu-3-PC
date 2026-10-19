#ifndef AE3_BOOT_TIMER2_RECHECK_H
#define AE3_BOOT_TIMER2_RECHECK_H
#include "ae3_boot_enable_intc.h"
/* Original parent 0x0043FE80 JAL -> 0x004406F0: run retail nested
 * DIntr/CP0-disabled branch until a real Timer2 MODE LW at 0x00440710.
 * Only EIE-clear Status is implemented. No synthetic MODE value. */
#define AE3_T2_RECHECK_CALL UINT32_C(0x0043FE80)
#define AE3_T2_RECHECK_MMIO_LW UINT32_C(0x00440710)
typedef enum AE3_T2RecheckResult {
 AE3_T2RC_YIELDED=1,AE3_T2RC_MMIO_PENDING=2,
 AE3_T2RC_MEMORY_FAULT=-1,AE3_T2RC_UNSUPPORTED=-2,AE3_T2RC_INVALID=-3
} AE3_T2RecheckResult;
typedef struct AE3_BootTimer2Recheck {
 AE3_BootPrefix cpu;
 uint32_t cop0_status;
 uint8_t active;
} AE3_BootTimer2Recheck;
AE3_T2RecheckResult ae3_boot_timer2_recheck_enter(AE3_BootTimer2Recheck *out,
 const AE3_BootEnableIntc *prior);
AE3_T2RecheckResult ae3_boot_timer2_recheck_slice(AE3_BootTimer2Recheck *state,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif
