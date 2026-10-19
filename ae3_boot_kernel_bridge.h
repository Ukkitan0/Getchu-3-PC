#ifndef AE3_BOOT_KERNEL_BRIDGE_H
#define AE3_BOOT_KERNEL_BRIDGE_H

#include "ae3_boot_thread_setup.h"

/* Continue the ORIGINAL guest opcodes from SetupThread (0x3c) to the first
 * post-kernel game initialization JAL. Kernel responses MUST be supplied by
 * the caller (e.g. a verified EE BIOS implementation or a reference trace).
 * A synthetic test response is not evidence of an authentic PS2 kernel boot.
 */
#define AE3_KERNEL_THREAD_PC UINT32_C(0x001002FC)
#define AE3_KERNEL_HEAP_PC UINT32_C(0x00100318)
#define AE3_KERNEL_NEXT_CALL_PC UINT32_C(0x0010031C)
#define AE3_KERNEL_NEXT_CALL_TARGET UINT32_C(0x0043FE58)

typedef enum AE3_KernelBootResult {
    AE3_KBOOT_YIELDED = 1,
    AE3_KBOOT_SETUP_THREAD = 2,
    AE3_KBOOT_SETUP_HEAP = 3,
    AE3_KBOOT_NEXT_CALL = 4,
    AE3_KBOOT_MEMORY_FAULT = -1,
    AE3_KBOOT_UNSUPPORTED = -2,
    AE3_KBOOT_INVALID = -3,
    AE3_KBOOT_REJECTED = -4
} AE3_KernelBootResult;

typedef struct AE3_BootKernelRequest {
    uint32_t number;
    uint32_t arguments[5];  /* a0-a3,t0; copied from real guest registers */
    uint32_t pc;
} AE3_BootKernelRequest;

typedef struct AE3_BootKernelBridge {
    AE3_BootPrefix cpu;
    AE3_BootKernelRequest request;
    uint32_t initial_stack_pointer;
    uint32_t heap_start;
    uint32_t heap_size;
    uint8_t completed_thread;
    uint8_t completed_heap;
    uint8_t pending_syscall;
    uint8_t finished;
} AE3_BootKernelBridge;

AE3_KernelBootResult ae3_boot_kernel_enter(AE3_BootKernelBridge *out,
                                            const AE3_BootThreadSetup *thread);
/* Stops at a real kernel service, never silently supplies a return value. */
AE3_KernelBootResult ae3_boot_kernel_slice(AE3_BootKernelBridge *s,
    const uint8_t *ram, size_t ram_size, uint32_t instruction_budget);
/* Thread return value MUST be sourced from an EE BIOS/kernel implementation.
 * We check alignment and consistency with the guest-specified stack range.
 * This function cannot establish BIOS accuracy of the supplied pointer. */
AE3_KernelBootResult ae3_boot_kernel_return_thread(AE3_BootKernelBridge *s,
                                                    uint32_t actual_stack_pointer);
/* SetupHeap is void. The owner must execute/record the kernel side effect
 * before acknowledging; we record the guest-requested memory limits only. */
AE3_KernelBootResult ae3_boot_kernel_ack_heap(AE3_BootKernelBridge *s);
#endif