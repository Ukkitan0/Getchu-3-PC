#ifndef AE3_EE_SYSCALL_KERNEL_H
#define AE3_EE_SYSCALL_KERNEL_H
#include <stddef.h>
#include <stdint.h>

/* Minimal guest-owned SetSyscall table for EE syscall 0x74.
 * This is an experimental compatibility kernel, NOT an EE BIOS implementation.
 * A registered handler is a GUEST instruction address, never a host pointer.
 * Registrations do not execute the handler or infer kernel memory mappings. */
#define AE3_EE_SYSCALL_SLOTS 256u
typedef struct AE3_EESyscallKernel {
    uint32_t handler[AE3_EE_SYSCALL_SLOTS];
    uint8_t installed[AE3_EE_SYSCALL_SLOTS];
    uint32_t generation;
} AE3_EESyscallKernel;
void ae3_ee_syscall_kernel_init(AE3_EESyscallKernel *kernel);
/* Returns 0 on successful atomic registration, -1 on invalid input. */
int ae3_ee_syscall_register(AE3_EESyscallKernel *kernel,
    uint32_t number, uint32_t guest_handler_pc,
    const uint8_t *guest_ram, size_t guest_ram_size);
/* Distinguish unregistered from intentionally blank/invalid addresses. */
int ae3_ee_syscall_lookup(const AE3_EESyscallKernel *kernel,
    uint32_t number, uint32_t *guest_handler_pc);
#endif