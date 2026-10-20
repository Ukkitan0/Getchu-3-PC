#ifndef AE3_EE_TIMER3_H
#define AE3_EE_TIMER3_H
#include "ae3_ee_memory_bus.h"
/* Experimental subset of the EE Timer 3 device. Bus clocks are explicitly
 * supplied; no invented wall-clock time or BIOS state. HBLANK and gated modes
 * are rejected instead of being approximated. MODE bit 8 is compare IRQ enable. */
#define AE3_T3_COUNT UINT32_C(0x10001800)
#define AE3_T3_MODE  UINT32_C(0x10001810)
#define AE3_T3_COMP  UINT32_C(0x10001820)
#define AE3_T3_CMPE UINT32_C(0x0100)
#define AE3_T3_OVFE UINT32_C(0x0200)
#define AE3_T3_EQUF UINT32_C(0x0400)
#define AE3_T3_OVFF UINT32_C(0x0800)
typedef struct AE3_EETimer3 {
    uint16_t count, mode, compare;
    uint16_t subcycles;
} AE3_EETimer3;
void ae3_ee_timer3_reset(AE3_EETimer3 *t);
AE3_EEBusStatus ae3_ee_timer3_read(void *ctx, uint32_t addr, uint32_t *out);
AE3_EEBusStatus ae3_ee_timer3_write(void *ctx, uint32_t addr, uint32_t value);
/* Advance exactly the caller-provided EE CPU clock cycles. This subset does
 * not report interrupts into COP0/INTC; mode flags are maintained internally.
 * Returns UNMAPPED without mutation for gated or HBLANK-sourced settings. */
AE3_EEBusStatus ae3_ee_timer3_advance(AE3_EETimer3 *t, uint32_t ee_cycles);
#endif