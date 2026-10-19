#ifndef AE3_BOOT_HOOK_INIT_H
#define AE3_BOOT_HOOK_INIT_H
#include "ae3_boot_semaphore_init.h"
/* First syscall patching initialization, reached AFTER the two retail
 * CreateSema calls have actually returned through their own MIPS wrappers.
 * Never silently registers a kernel handler: stops at SetSyscall 0x74. */
#define AE3_HOOK_CALL_PC UINT32_C(0x0043FE68)
#define AE3_HOOK_ROUTINE_PC UINT32_C(0x0043FD48)
#define AE3_HOOK_WRAPPER_PC UINT32_C(0x0043FE48)
#define AE3_HOOK_SYSCALL_PC UINT32_C(0x0043FE4C)

typedef enum AE3_HookResult {
    AE3_HOOK_YIELDED=1, AE3_HOOK_SET_SYSCALL=2,
    AE3_HOOK_MEMORY_FAULT=-1, AE3_HOOK_UNSUPPORTED=-2,
    AE3_HOOK_INVALID=-3
} AE3_HookResult;
typedef struct AE3_BootHookInit {
    AE3_BootPrefix cpu;
    uint32_t syscall_number, guest_handler_number, guest_handler_address;
    uint8_t pending;
} AE3_BootHookInit;
AE3_HookResult ae3_boot_hook_enter(AE3_BootHookInit *out,
                                  const AE3_BootSemaphoreInit *sema);
AE3_HookResult ae3_boot_hook_slice(AE3_BootHookInit *s,
                                  uint8_t *ram, size_t bytes, uint32_t budget);
#endif