#ifndef AE3_BOOT_SECOND_NUMERIC_H
#define AE3_BOOT_SECOND_NUMERIC_H
#include "ae3_boot_second_constructor.h"
/* Enter original numeric helper at 0x41EFD0 with the original IEEE-754 double
   bits from the constructor; do not impersonate the 0x423E20 callee. */
#define AE3_SECOND_MATH_ENTRY UINT32_C(0x0041EFD0)
#define AE3_SECOND_MATH_CALLEE_JAL UINT32_C(0x0041F118)
#define AE3_SECOND_MATH_CALLEE UINT32_C(0x00423E20)
typedef enum AE3_SecondNumericResult {
 AE3_SN_YIELDED=1,AE3_SN_MATH_CALL_PENDING=2,
 AE3_SN_MEMORY_FAULT=-1,AE3_SN_UNSUPPORTED=-2,AE3_SN_INVALID=-3
} AE3_SecondNumericResult;
typedef struct AE3_BootSecondNumeric {
 AE3_BootPrefix cpu;
 uint32_t double_high_word;
 uint32_t math_branches_taken;
 uint8_t active;
} AE3_BootSecondNumeric;
AE3_SecondNumericResult ae3_boot_second_numeric_enter(AE3_BootSecondNumeric *out,
 const AE3_BootSecondConstructor *prior);
AE3_SecondNumericResult ae3_boot_second_numeric_slice(AE3_BootSecondNumeric *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif