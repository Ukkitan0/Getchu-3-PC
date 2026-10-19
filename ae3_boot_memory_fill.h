#ifndef AE3_BOOT_MEMORY_FILL_H
#define AE3_BOOT_MEMORY_FILL_H
#include "ae3_boot_prefix.h"
/* Real retail 0x001036D8 callee: fills upper 384 KiB of the *normal* 32MiB
   EE RAM using the 128-bit pattern loaded at 0x006B8120. This region ends
   exactly at 0x02000000, NOT at a memory-mapped peripheral. */
#define AE3_FILL_BASE UINT32_C(0x01FA0000)
#define AE3_FILL_BYTES UINT32_C(0x00060000)
#define AE3_BOOT_CALLEE UINT32_C(0x001036D8)
#define AE3_BOOT_RETURN UINT32_C(0x001002CC)
typedef enum {
    AE3_FILL_YIELDED = 1, AE3_FILL_RETURNED = 2,
    AE3_FILL_MEMORY_FAULT = -1, AE3_FILL_UNSUPPORTED = -2,
    AE3_FILL_BAD_STATE = -3
} AE3_FillResult;
typedef struct {
    AE3_BootPrefix cpu;
    uint32_t fill_stores;
    uint8_t finished;
} AE3_BootMemoryFill;
AE3_FillResult ae3_boot_fill_enter(AE3_BootMemoryFill *stage,
                                  const AE3_BootPrefix *prefix);
AE3_FillResult ae3_boot_fill_slice(AE3_BootMemoryFill *stage,
                                  uint8_t *ram, size_t ram_size,
                                  uint32_t budget);
#endif