#ifndef AE3_BOOT_THREAD_NEXT_H
#define AE3_BOOT_THREAD_NEXT_H
#include "ae3_boot_thread_continue.h"
/* Retail 0x436C20 callee, including nested MFC0 Status inquiry at 0x4365B0.
 * Status is an EXPLICIT fixture input, not inferred Sony BIOS state.
 * Status bit0 clear takes the short original return to GetThreadId 0x2F. */
#define AE3_TNEXT_GET_THREAD_ID UINT32_C(0x004359D4)
#define AE3_TNEXT_CRITICAL_CALL UINT32_C(0x00436C48)
typedef enum AE3_ThreadNextResult {
 AE3_TNEXT_YIELDED=1,AE3_TNEXT_GETID_PENDING=2,
 AE3_TNEXT_CRITICAL_PENDING=3,
 AE3_TNEXT_MEMORY_FAULT=-1,AE3_TNEXT_UNSUPPORTED=-2,AE3_TNEXT_INVALID=-3
} AE3_ThreadNextResult;
typedef struct AE3_BootThreadNext {
 AE3_BootPrefix cpu;
 uint32_t cop0_status;
 uint8_t active;
} AE3_BootThreadNext;
AE3_ThreadNextResult ae3_boot_thread_next_enter(AE3_BootThreadNext *s,
 const AE3_BootThreadContinue *prior,uint32_t cop0_status);
AE3_ThreadNextResult ae3_boot_thread_next_slice(AE3_BootThreadNext *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif