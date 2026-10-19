#ifndef AE3_BOOT_THREAD_PRIORITY_H
#define AE3_BOOT_THREAD_PRIORITY_H
#include "ae3_boot_thread_next.h"

/* Executes verified original GetThreadId / ChangeThreadPriority wrappers,
 * their delay slots and the return to the parent initializer. The current
 * boot thread is an explicit TEST fixture, never inferred from a Sony BIOS. */
#define AE3_TPR_GETID_SYSCALL UINT32_C(0x004359D4)
#define AE3_TPR_PRIORITY_SYSCALL UINT32_C(0x00435974)
#define AE3_TPR_PARENT_NEXT UINT32_C(0x0043FE90)
typedef enum AE3_ThreadPriorityResult {
 AE3_TPR_YIELDED=1,AE3_TPR_GETID_PENDING=2,
 AE3_TPR_PRIORITY_PENDING=3,AE3_TPR_PARENT_RETURNED=4,
 AE3_TPR_CONTEXT_MISSING=-1,AE3_TPR_MEMORY_FAULT=-2,
 AE3_TPR_UNSUPPORTED=-3,AE3_TPR_INVALID=-4
} AE3_ThreadPriorityResult;
typedef struct AE3_BootThreadPriority {
 AE3_BootPrefix cpu;
 int32_t returned_id,priority_result;
 uint8_t active,getid_serviced,priority_serviced;
} AE3_BootThreadPriority;
AE3_ThreadPriorityResult ae3_boot_thread_priority_enter(
 AE3_BootThreadPriority *out,const AE3_BootThreadNext *prior);
AE3_ThreadPriorityResult ae3_boot_thread_priority_getid(
 AE3_BootThreadPriority *state,const AE3_EEMemoryBus *bus,
 const AE3_EEThreadKernel *kernel);
AE3_ThreadPriorityResult ae3_boot_thread_priority_change(
 AE3_BootThreadPriority *state,const AE3_EEMemoryBus *bus,
 AE3_EEThreadKernel *kernel);
AE3_ThreadPriorityResult ae3_boot_thread_priority_slice(
 AE3_BootThreadPriority *state,const AE3_EEMemoryBus *bus,uint32_t budget);
#endif