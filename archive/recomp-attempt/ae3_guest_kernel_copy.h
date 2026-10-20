#ifndef AE3_GUEST_KERNEL_COPY_H
#define AE3_GUEST_KERNEL_COPY_H
#include "ae3_boot_prefix.h"
#include "ae3_ee_memory_bus.h"
/* Execute the EXACT original syscall 0x5A guest handler at 0x004402F0.
 * Runs a MIPS loop that copies aligned 32-bit words into KSEG0 EE RAM.
 * The separate syscall-kernel dispatcher must explicitly select this handler.
 * This is not evidence of a real BIOS syscall trampoline or its return ABI. */
#define AE3_GUEST_COPY_ENTRY UINT32_C(0x004402F0)
#define AE3_GUEST_COPY_END UINT32_C(0x00440324)
typedef enum AE3_CopyResult {
 AE3_COPY_YIELDED=1,AE3_COPY_RETURNED=2,AE3_COPY_MEMORY_FAULT=-1,
 AE3_COPY_UNSUPPORTED=-2,AE3_COPY_INVALID=-3
} AE3_CopyResult;
typedef struct AE3_GuestKernelCopy {
 AE3_BootPrefix cpu;
 uint32_t return_pc;
 uint8_t done;
} AE3_GuestKernelCopy;
AE3_CopyResult ae3_guest_kernel_copy_init(AE3_GuestKernelCopy *out,
 uint32_t dest,uint32_t source,uint32_t bytes,uint32_t return_pc);
AE3_CopyResult ae3_guest_kernel_copy_slice(AE3_GuestKernelCopy *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif