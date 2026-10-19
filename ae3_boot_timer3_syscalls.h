#ifndef AE3_BOOT_TIMER3_SYSCALLS_H
#define AE3_BOOT_TIMER3_SYSCALLS_H
#include "ae3_boot_timer3_resume.h"
#include "ae3_ee_syscall_kernel.h"
#include "ae3_guest_kernel_copy.h"
/* Guest-only instruction runner for the two actual wrappers at 0x4402D0 and
 * 0x4402E0. SetSyscall and 0x5A handler dispatch remain EXPLICIT compatibility
 * kernel services, not Sony BIOS execution or exception-vector emulation. */
typedef enum AE3_TimerSysResult {
 AE3_TS_YIELDED=1,AE3_TS_SET_SYSCALL=2,AE3_TS_COPY_SYSCALL=3,
 AE3_TS_RETURNED=4,AE3_TS_MEMORY_FAULT=-1,AE3_TS_UNSUPPORTED=-2,
 AE3_TS_INVALID=-3
} AE3_TimerSysResult;
typedef struct AE3_Timer3Syscalls {
 AE3_BootPrefix cpu;
 AE3_GuestKernelCopy copy;
 uint8_t copy_active;
 uint8_t done;
} AE3_Timer3Syscalls;
AE3_TimerSysResult ae3_timer3_syscalls_enter(AE3_Timer3Syscalls*out,
 const AE3_BootTimer3Resume *timer);
AE3_TimerSysResult ae3_timer3_syscalls_slice(AE3_Timer3Syscalls*s,
 AE3_EEMemoryBus*bus,uint32_t budget);
AE3_TimerSysResult ae3_timer3_syscalls_dispatch(AE3_Timer3Syscalls*s,
 AE3_EEMemoryBus*bus,AE3_EESyscallKernel*kernel);
#endif