#ifndef AE3_BOOT_EVENT_CREATE_H
#define AE3_BOOT_EVENT_CREATE_H
#include "ae3_boot_timer2_finish.h"
#include "ae3_ee_sema_kernel.h"
/* Retail initializer 0x0043FE88 -> 0x004368B8. Syscall 0x40 is
 * CreateSema (NOT CreateEventFlag, which is 0x50). Compatibility-only.
 * Recovered new opcodes from the user-supplied PCSX2 EE RAM snapshot,
 * cross-checked against the existing retail opcode tables. */
#define AE3_EVENT_GUEST_SYSCALL UINT32_C(0x00435AE4)
#define AE3_EVENT_PARENT_NEXT UINT32_C(0x0043FE90)
#define AE3_EVENT_GUEST_STATE UINT32_C(0x006A1788)
#define AE3_EVENT_THREAD_SYSCALL UINT32_C(0x004358E4)
typedef enum AE3_EventInitResult {
 AE3_EVT_YIELDED=1,AE3_EVT_SYSCALL_PENDING=2,AE3_EVT_RETURNED=3,
 AE3_EVT_THREAD_SYSCALL_PENDING=4,
 AE3_EVT_MEMORY_FAULT=-1,AE3_EVT_UNSUPPORTED=-2,AE3_EVT_INVALID=-3
} AE3_EventInitResult;
typedef struct AE3_BootEventInit {
 AE3_BootPrefix cpu;
 uint32_t observed_global;
 int32_t sema_result;
 uint8_t entered;
 uint8_t global_read;
 uint8_t sema_serviced;
} AE3_BootEventInit;
AE3_EventInitResult ae3_boot_event_enter(AE3_BootEventInit *out,
 const AE3_BootTimer2Finish *prior);
AE3_EventInitResult ae3_boot_event_slice(AE3_BootEventInit *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
/* Reads the actual six-word guest ee_sema_t through the bus; failure
 * leaves CPU, kernel, and memory unchanged so caller can recover.
 * Positive IDs are selected by the experimental existing kernel pool,
 * never assumed to be Sony BIOS-assigned IDs. */
AE3_EventInitResult ae3_boot_event_service_sema(AE3_BootEventInit *state,
 AE3_EEMemoryBus *bus,AE3_EESemaKernel *kernel);
#endif
