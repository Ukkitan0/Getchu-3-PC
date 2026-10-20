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
int ae3_ee_syscall_register_bus(AE3_EESyscallKernel *kernel,
    uint32_t number,uint32_t guest_pc,const AE3_EEMemoryBus *bus){
    uint32_t original_word=0;
    if(!kernel||!bus||!bus->ram||number>=AE3_EE_SYSCALL_SLOTS||
       (guest_pc&3u)||ae3_ee_bus_read32(bus,guest_pc,&original_word)!=AE3_EE_BUS_OK)
       return -1;
    /* Keep the exact guest virtual address; aliases are translated by the bus. */
    kernel->handler[number]=guest_pc;
    kernel->installed[number]=1;
    kernel->generation++;
    (void)original_word;
    return 0;
}
int ae3_ee_syscall_lookup(const AE3_EESyscallKernel *kernel,
    uint32_t number, uint32_t *pc) {
    if (!kernel || !pc || number>=AE3_EE_SYSCALL_SLOTS ||
        !kernel->installed[number]) return -1;
    *pc=kernel->handler[number];
    return 0;
}