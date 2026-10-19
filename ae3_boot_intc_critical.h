#ifndef AE3_BOOT_INTC_CRITICAL_H
#define AE3_BOOT_INTC_CRITICAL_H
#include "ae3_boot_post_intc.h"
/* Original 0x440578 JAL to CP0 Status inspection at 0x43FBA0. Requires an
 * explicitly supplied COP0 Status value; NO assumed BIOS or CPU state.
 * Implements only the path where EIE bit 16 is clear. Stops before the next
 * unresolved device register read at EE Timer2 MODE 0x10001010. */
#define AE3_T2_MODE_REGISTER UINT32_C(0x10001010)
#define AE3_T2_MODE_GUEST_LW UINT32_C(0x00440590)
typedef enum AE3_IntcCriticalResult {
 AE3_INTC_CRITICAL_YIELDED=1,AE3_INTC_CRITICAL_T2_MODE_PENDING=2,
 AE3_INTC_CRITICAL_MEMORY_FAULT=-1,AE3_INTC_CRITICAL_UNSUPPORTED=-2,
 AE3_INTC_CRITICAL_INVALID=-3
} AE3_IntcCriticalResult;
typedef struct AE3_BootIntcCritical {
 AE3_BootPrefix cpu;
 uint32_t cop0_status;
 uint8_t enabled;
} AE3_BootIntcCritical;
AE3_IntcCriticalResult ae3_boot_intc_critical_enter(AE3_BootIntcCritical *out,
  const AE3_BootPostIntc *prior,uint32_t explicit_cop0_status);
AE3_IntcCriticalResult ae3_boot_intc_critical_slice(AE3_BootIntcCritical *s,
  AE3_EEMemoryBus *bus,uint32_t budget);
#endif