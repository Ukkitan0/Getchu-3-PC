#ifndef AE3_BOOT_EVENT_CREATE_H
#define AE3_BOOT_EVENT_CREATE_H
#include "ae3_boot_timer2_finish.h"
/* Retail initializer at 0x0043FE88 -> 0x004368B8: inspect the actual
 * 0x006A1788 state before deciding whether to allocate the next OS event.
 * Stops BEFORE CreateEventFlag-style kernel syscall 0x40; no fake ID. */
#define AE3_EVENT_GUEST_SYSCALL UINT32_C(0x00435AE4)
#define AE3_EVENT_PARENT_NEXT UINT32_C(0x0043FE90)
#define AE3_EVENT_GUEST_STATE UINT32_C(0x006A1788)
typedef enum AE3_EventInitResult {
 AE3_EVT_YIELDED=1,AE3_EVT_SYSCALL_PENDING=2,AE3_EVT_RETURNED=3,
 AE3_EVT_MEMORY_FAULT=-1,AE3_EVT_UNSUPPORTED=-2,AE3_EVT_INVALID=-3
} AE3_EventInitResult;
typedef struct AE3_BootEventInit {
 AE3_BootPrefix cpu;
 uint32_t observed_global;
 uint8_t entered;
 uint8_t global_read;
} AE3_BootEventInit;
AE3_EventInitResult ae3_boot_event_enter(AE3_BootEventInit *out,
 const AE3_BootTimer2Finish *prior);
AE3_EventInitResult ae3_boot_event_slice(AE3_BootEventInit *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif
