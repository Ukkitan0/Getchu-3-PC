#ifndef AE3_BOOT_TIMER2_FINISH_H
#define AE3_BOOT_TIMER2_FINISH_H
#include "ae3_boot_timer2_recheck.h"
/* Genuine retail Timer2 second initialization, entered at original LW
 * 0x00440710. Known Timer2 MMIO must be mapped: no invented register bits.
 * EIE-active guest paths and later hardware services are explicit boundaries.
 */
#define AE3_T2_FINISH_PARENT_NEXT UINT32_C(0x0043FE88)
#define AE3_T2_FINISH_EINTR_CALL UINT32_C(0x00440728)
#define AE3_T2_FINISH_MODE_CALL UINT32_C(0x00440744)
#define AE3_T2_FINISH_NEXT_SERVICE UINT32_C(0x0044074C)
typedef enum AE3_T2FinishResult {
 AE3_T2F_YIELDED=1,
 AE3_T2F_RETURNED=2,
 AE3_T2F_EINTR_PENDING=3,
 AE3_T2F_SET_MODE_PENDING=4,
 AE3_T2F_NEXT_SERVICE_PENDING=5,
 AE3_T2F_MEMORY_FAULT=-1,
 AE3_T2F_UNSUPPORTED=-2,
 AE3_T2F_INVALID=-3
} AE3_T2FinishResult;
typedef struct AE3_BootTimer2Finish {
 AE3_BootPrefix cpu;
 uint32_t cop0_status;
 uint32_t observed_mode;
 uint8_t entered;
 uint8_t read_performed;
} AE3_BootTimer2Finish;
AE3_T2FinishResult ae3_boot_timer2_finish_enter(AE3_BootTimer2Finish *out,
 const AE3_BootTimer2Recheck *prior);
AE3_T2FinishResult ae3_boot_timer2_finish_slice(AE3_BootTimer2Finish *state,
 const AE3_EEMemoryBus *bus,uint32_t budget);
/* The JAL at 0x440744 is run by the shared real SetT2_MODE wrapper.
 * Commit its result only after verified original ERET reaches 0x44074C. */
AE3_T2FinishResult ae3_boot_timer2_finish_resume_mode(AE3_BootTimer2Finish *state,
 const AE3_BootTimer2Setup *completed);
#endif
