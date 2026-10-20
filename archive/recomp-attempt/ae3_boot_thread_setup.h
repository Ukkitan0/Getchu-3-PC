#ifndef AE3_BOOT_THREAD_SETUP_H
#define AE3_BOOT_THREAD_SETUP_H
#include "ae3_boot_memory_fill.h"

/* Original guest startup after return from 0x001036D8. It pauses BEFORE the
 * kernel syscall; no guessed return value or fabricated stack pointer. */
#define AE3_SETUP_THREAD_SYSCALL_PC UINT32_C(0x001002FC)
#define AE3_SETUP_THREAD_SYSCALL_NUMBER UINT32_C(0x3C)

typedef enum { AE3_THREAD_YIELDED=1, AE3_THREAD_SYSCALL=2,
               AE3_THREAD_BAD_STATE=-1, AE3_THREAD_UNSUPPORTED=-2,
               AE3_THREAD_MEMORY_FAULT=-3 } AE3_ThreadStatus;
typedef struct { AE3_BootPrefix cpu; uint8_t finished; } AE3_BootThreadSetup;
AE3_ThreadStatus ae3_boot_thread_enter(AE3_BootThreadSetup *thread,
                                        const AE3_BootMemoryFill *fill);
AE3_ThreadStatus ae3_boot_thread_slice(AE3_BootThreadSetup *thread,
                                        const uint8_t *ram, size_t ram_size,
                                        uint32_t budget);
#endif