#ifndef AE3_BOOT_HOOK_INIT_H
#define AE3_BOOT_HOOK_INIT_H
#include "ae3_boot_semaphore_init.h"
#include "ae3_ee_syscall_kernel.h"
/* First syscall patching initialization, reached AFTER the two retail
 * CreateSema calls have actually returned through their own MIPS wrappers.
 * Never silently registers a kernel handler: stops at SetSyscall 0x74. */
#define AE3_HOOK_CALL_PC UINT32_C(0x0043FE68)
#define AE3_HOOK_ROUTINE_PC UINT32_C(0x0043FD48)
#define AE3_HOOK_WRAPPER_PC UINT32_C(0x0043FE48)
#define AE3_HOOK_SYSCALL_PC UINT32_C(0x0043FE4C)

typedef enum AE3_HookResult {
    AE3_HOOK_YIELDED=1, AE3_HOOK_SET_SYSCALL=2, AE3_HOOK_GUEST_SYSCALL=3, AE3_HOOK_FIND_PAIR_COMPLETE=4,
    AE3_HOOK_MEMORY_FAULT=-1, AE3_HOOK_UNSUPPORTED=-2,
    AE3_HOOK_INVALID=-3
} AE3_HookResult;
typedef struct AE3_BootHookInit {
    AE3_BootPrefix cpu;
    uint32_t syscall_number, guest_handler_number, guest_handler_address;
    uint32_t invoked_number, invoked_handler_pc;
    uint32_t registered_numbers[2], registered_handlers[2];
    uint8_t pending, registered, guest_pending, guest_completed;
} AE3_BootHookInit;
AE3_HookResult ae3_boot_hook_enter(AE3_BootHookInit *out,
                                  const AE3_BootSemaphoreInit *sema);
AE3_HookResult ae3_boot_hook_slice(AE3_BootHookInit *s,
                                  uint8_t *ram, size_t bytes, uint32_t budget);
/* Atomically accept the pending original SetSyscall instruction via a
   maintained guest kernel table. No fabricated v0 return value. */
AE3_HookResult ae3_boot_hook_register(AE3_BootHookInit *s,
    AE3_EESyscallKernel *kernel, const uint8_t *ram, size_t bytes);
/* A registered guest syscall is an execution handoff, NOT a return value.
   The caller must implement the handler's MIPS instructions/memory mapping. */
AE3_HookResult ae3_boot_hook_route(AE3_BootHookInit *s,
    const AE3_EESyscallKernel *kernel, uint32_t *target_pc);
#endif