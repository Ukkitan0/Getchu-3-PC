#ifndef AE3_EE_TIMER2_H
#define AE3_EE_TIMER2_H
#include "ae3_ee_memory_bus.h"
/* Explicit, standalone EE Timer 2 model. This is not a full EE counter/INTC
 * implementation: video-derived clocks/gates and automatic IRQ delivery are
 * deliberately unsupported. Time advances only by caller-supplied EE cycles. */
#define AE3_T2_COUNT UINT32_C(0x10001000)
#define AE3_T2_MODE  UINT32_C(0x10001010)
#define AE3_T2_COMP  UINT32_C(0x10001020)
#define AE3_T2_CMPE UINT16_C(0x0100)
#define AE3_T2_OVFE UINT16_C(0x0200)
#define AE3_T2_EQUF UINT16_C(0x0400)
#define AE3_T2_OVFF UINT16_C(0x0800)
typedef struct AE3_EETimer2 {
    uint16_t count, mode, compare, subcycles;
} AE3_EETimer2;
void ae3_ee_timer2_reset(AE3_EETimer2 *timer);
AE3_EEBusStatus ae3_ee_timer2_read(void *context,uint32_t address,uint32_t *out);
AE3_EEBusStatus ae3_ee_timer2_write(void *context,uint32_t address,uint32_t value);
AE3_EEBusStatus ae3_ee_timer2_advance(AE3_EETimer2 *timer,uint32_t ee_cycles);
#endif