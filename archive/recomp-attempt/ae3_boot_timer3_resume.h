#ifndef AE3_BOOT_TIMER3_RESUME_H
#define AE3_BOOT_TIMER3_RESUME_H
#include "ae3_boot_next_init.h"
/* The original retail function 0x00440338 branches based on T3.MODE bit 8.
 * This runner consumes a REAL MMIO decoder and retires source MIPS words only.
 * It pauses before call to 0x004402D0 (normal path) or after return at
 * 0x0043FE78 (compare-interrupt-enabled path). */
#define AE3_TIMER3_NEXT_CALL UINT32_C(0x00440378)
#define AE3_TIMER3_CALL_TARGET UINT32_C(0x004402D0)
#define AE3_TIMER3_RETURN_PC UINT32_C(0x0043FE78)
typedef enum AE3_TimerBootResult {
 AE3_TIMER_BOOT_YIELDED=1,
 AE3_TIMER_BOOT_NEXT_CALL=2,
 AE3_TIMER_BOOT_RETURNED=3,
 AE3_TIMER_BOOT_UNMAPPED=-1,
 AE3_TIMER_BOOT_UNSUPPORTED=-2,
 AE3_TIMER_BOOT_INVALID=-3
} AE3_TimerBootResult;
typedef struct AE3_BootTimer3Resume {
 AE3_BootPrefix cpu;
 uint8_t complete;
} AE3_BootTimer3Resume;
AE3_TimerBootResult ae3_boot_timer3_enter(AE3_BootTimer3Resume *out,
    const AE3_BootNextInit *prior);
AE3_TimerBootResult ae3_boot_timer3_slice(AE3_BootTimer3Resume *s,
    AE3_EEMemoryBus *bus,uint32_t instruction_budget);
#endif