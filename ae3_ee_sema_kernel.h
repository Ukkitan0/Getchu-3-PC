#ifndef AE3_EE_SEMA_KERNEL_H
#define AE3_EE_SEMA_KERNEL_H
#include "ae3_boot_semaphore_init.h"
/* Small native EE semaphore compatibility model (NOT a BIOS-identical kernel).
 * The guest sema descriptor is captured by its original 0x40 syscall wrapper.
 * IDs are owned by this kernel, never fabricated by the instruction runner.
 * No scheduling, blocking wait queues, interrupts, priority inheritance, or
 * ABI-specific BIOS error numbers have been implemented yet. */
#define AE3_SEMA_CAPACITY 256u
typedef struct AE3_EEKernelSemaphore {
    uint32_t max_count, count, attr, option;
    uint8_t allocated;
} AE3_EEKernelSemaphore;
typedef struct AE3_EESemaKernel {
    AE3_EEKernelSemaphore entries[AE3_SEMA_CAPACITY];
} AE3_EESemaKernel;
void ae3_ee_sema_kernel_init(AE3_EESemaKernel *kernel);
/* Returns positive guest ID or negative generic failure. */
int32_t ae3_ee_sema_kernel_create(AE3_EESemaKernel *kernel,
                                   const AE3_GuestSemaRequest *request);
/* Return zero on success or -1 on unavailable/invalid resource. */
int ae3_ee_sema_kernel_signal(AE3_EESemaKernel *kernel, int32_t id);
int ae3_ee_sema_kernel_delete(AE3_EESemaKernel *kernel,int32_t id);
int ae3_ee_sema_kernel_poll(AE3_EESemaKernel *kernel, int32_t id);
int ae3_ee_sema_kernel_get(const AE3_EESemaKernel *kernel, int32_t id,
                           uint32_t *count, uint32_t *maximum);
/* Adapter responding to a pending 0x40 call using this kernel's object pool. */
AE3_SemaBootResult ae3_ee_sema_boot_dispatch(AE3_EESemaKernel *kernel,
                                             AE3_BootSemaphoreInit *boot);
#endif