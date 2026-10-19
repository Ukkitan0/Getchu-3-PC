#ifndef AE3_EE_INTC_KERNEL_H
#define AE3_EE_INTC_KERNEL_H
#include "ae3_ee_memory_bus.h"
/* Small experimental EE AddIntcHandler / RemoveIntcHandler compatibility model.
 * Not a BIOS; does not dispatch interrupts, prioritize handlers, or update COP0.
 * The returned IDs are policy-selected, never presented as Sony BIOS outputs. */
#define AE3_INTC_CAUSES 16u
#define AE3_INTC_CAPACITY 64u
typedef struct AE3_EEIntcHandler {
    uint32_t id, cause, entry, next;
    uint8_t present;
} AE3_EEIntcHandler;
typedef struct AE3_EEIntcKernel {
    AE3_EEIntcHandler handler[AE3_INTC_CAPACITY];
    uint32_t next_id, generation;
} AE3_EEIntcKernel;
void ae3_ee_intc_init(AE3_EEIntcKernel *k);
/* Returns a positive handler ID, or -1 without mutating anything. The guest
 * handler must be aligned and readable via physical RAM / its EE aliases.
 * 'next' is kept as guest metadata; handler ordering is not implemented. */
int32_t ae3_ee_intc_add(AE3_EEIntcKernel *k,const AE3_EEMemoryBus *bus,
    uint32_t cause,uint32_t guest_handler,uint32_t next);
int ae3_ee_intc_remove(AE3_EEIntcKernel *k,uint32_t cause,uint32_t id);
int ae3_ee_intc_lookup(const AE3_EEIntcKernel *k,uint32_t id,AE3_EEIntcHandler *out);
#endif