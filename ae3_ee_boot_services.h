#ifndef AE3_EE_BOOT_SERVICES_H
#define AE3_EE_BOOT_SERVICES_H
#include "ae3_boot_kernel_bridge.h"
#include "ae3_ee_memory_bus.h"
/*
 * Experimental EE startup kernel model. This is a documented compatibility
 * policy, NOT a reconstruction of the Sony BIOS. All addresses and sizes are
 * derived from the retail guest's actual syscall arguments. Only the amount
 * of stack frame space reserved at startup is a policy parameter.
 */
typedef enum AE3_BootServiceResult {
    AE3_BS_APPLIED=1, AE3_BS_BAD_ARGUMENT=-1,
    AE3_BS_UNMAPPED=-2, AE3_BS_OVERLAP=-3,
    AE3_BS_WRONG_SYSCALL=-4
} AE3_BootServiceResult;
typedef struct AE3_EEBootServices {
    uint32_t gp, stack_base, stack_size, stack_pointer;
    uint32_t args_pointer, exit_function;
    uint32_t heap_base, heap_size;
    uint32_t reserved_top_bytes;
    uint32_t generation;
    uint8_t thread_created, heap_created;
} AE3_EEBootServices;

void ae3_ee_boot_services_init(AE3_EEBootServices *model, uint32_t stack_top_reserve);
/* Services commit both guest state and kernel model atomically. The supplied
 * bus must expose the same physical RAM used to execute original guest code. */
AE3_BootServiceResult ae3_ee_boot_services_dispatch(
    AE3_EEBootServices *model, AE3_BootKernelBridge *bridge,
    const AE3_EEMemoryBus *bus);
#endif