#ifndef AE3_BOOT_SECOND_CONSTRUCTOR_H
#define AE3_BOOT_SECOND_CONSTRUCTOR_H
#include "ae3_boot_first_constructor.h"
/* Original second constructor 0x417C58 -> 0x417B50, including two retail
   return stubs and the actual 0x41D6B8 numeric wrapper. Halt at the original
   0x41EFD0 target: no synthetic numerical result or callback completion. */
#define AE3_SECOND_BODY_PC UINT32_C(0x00417B50)
#define AE3_SECOND_NUMERIC_WRAPPER UINT32_C(0x0041D6B8)
#define AE3_SECOND_NUMERIC_BOUNDARY UINT32_C(0x0041EFD0)
typedef enum AE3_SecondCtorResult {
 AE3_SC_YIELDED=1,AE3_SC_NUMERIC_PENDING=2,
 AE3_SC_MEMORY_FAULT=-1,AE3_SC_UNSUPPORTED=-2,AE3_SC_INVALID=-3
} AE3_SecondCtorResult;
typedef struct AE3_BootSecondConstructor {
 AE3_BootPrefix cpu;
 uint32_t stub_calls;
 uint32_t wrapper_visits;
 uint32_t saved_first_stores;
 uint8_t active;
} AE3_BootSecondConstructor;
AE3_SecondCtorResult ae3_boot_second_ctor_enter(AE3_BootSecondConstructor *out,
 const AE3_BootFirstConstructor *first);
AE3_SecondCtorResult ae3_boot_second_ctor_slice(AE3_BootSecondConstructor *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif