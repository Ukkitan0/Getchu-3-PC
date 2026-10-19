#ifndef AE3_EE_THREAD_KERNEL_H
#define AE3_EE_THREAD_KERNEL_H
#include "ae3_ee_memory_bus.h"
/* Experimental ID and descriptor registry for EE CreateThread (syscall 0x20).
 * Not a PS2 BIOS scheduler. Created threads are dormant, never executed here.
 * The 9-word layout is PS2SDK ee_thread_t, not a host pointer layout. */
#define AE3_EE_THREAD_CAPACITY 256u
#define AE3_EE_THREAD_DESC_WORDS 9u
typedef struct AE3_EEThread {
 uint32_t guest_descriptor;
 uint32_t fields[AE3_EE_THREAD_DESC_WORDS];
 uint8_t allocated;
} AE3_EEThread;
typedef struct AE3_EEThreadKernel {
 AE3_EEThread entries[AE3_EE_THREAD_CAPACITY];
 uint32_t generation;
} AE3_EEThreadKernel;
void ae3_ee_thread_kernel_init(AE3_EEThreadKernel *k);
/* Read/check the entire guest descriptor and memory spans before allocating.
 * Returns positive compatibility ID, -1 on invalid/exhaustion; no mutation on
 * rejection. Unreadable descriptor (as distinct from invalid contents) is
 * reported separately by the guest syscall adapter. */
int32_t ae3_ee_thread_kernel_create(AE3_EEThreadKernel *k,
 const AE3_EEMemoryBus *bus,uint32_t descriptor,
 const uint32_t values[AE3_EE_THREAD_DESC_WORDS]);
int ae3_ee_thread_kernel_lookup(const AE3_EEThreadKernel *k,
 uint32_t id,AE3_EEThread *out);
#endif