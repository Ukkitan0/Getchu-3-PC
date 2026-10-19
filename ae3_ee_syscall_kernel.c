#include "ae3_ee_syscall_kernel.h"
void ae3_ee_syscall_kernel_init(AE3_EESyscallKernel *kernel) {
    if (kernel) *kernel=(AE3_EESyscallKernel){0};
}
int ae3_ee_syscall_register(AE3_EESyscallKernel *kernel,
    uint32_t number, uint32_t pc, const uint8_t *ram, size_t size) {
    if (!kernel || !ram || number>=AE3_EE_SYSCALL_SLOTS ||
        (pc&3u) || pc<UINT32_C(0x00100000) ||
        (size_t)pc>size || size-(size_t)pc<4) return -1;
    /* Do not claim that this implements the PS2 exception vector or the
       kernel execution privilege model. Validated addresses are stored only. */
    kernel->handler[number]=pc;
    kernel->installed[number]=1;
    kernel->generation++;
    return 0;
}
int ae3_ee_syscall_lookup(const AE3_EESyscallKernel *kernel,
    uint32_t number, uint32_t *pc) {
    if (!kernel || !pc || number>=AE3_EE_SYSCALL_SLOTS ||
        !kernel->installed[number]) return -1;
    *pc=kernel->handler[number];
    return 0;
}