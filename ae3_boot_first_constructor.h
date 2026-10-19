#ifndef AE3_BOOT_FIRST_CONSTRUCTOR_H
#define AE3_BOOT_FIRST_CONSTRUCTOR_H
#include "ae3_boot_static_dispatch.h"
/* Execute the real first retail static constructor (0x0041A390), including
   its tail call to 0x0041A088 and the original 0x004173B0 return stub.
   This remains bounded guest-code execution, not a general MIPS emulator. */
#define AE3_FIRST_CTOR_PC UINT32_C(0x0041A390)
#define AE3_FIRST_CTOR_BODY UINT32_C(0x0041A088)
#define AE3_FIRST_CTOR_STUB UINT32_C(0x004173B0)
#define AE3_SECOND_CTOR_PC UINT32_C(0x00417C58)
typedef enum AE3_FirstCtorResult {
    AE3_FCTOR_YIELDED=1,
    AE3_FCTOR_SECOND_PENDING=2,
    AE3_FCTOR_DISPATCH_DONE=3,
    AE3_FCTOR_MEMORY_FAULT=-1,
    AE3_FCTOR_UNSUPPORTED=-2,
    AE3_FCTOR_INVALID=-3
} AE3_FirstCtorResult;
typedef struct AE3_BootFirstConstructor {
    AE3_BootPrefix cpu;
    uint32_t reference_count_stores;
    uint32_t stub_calls;
    uint32_t next_constructor;
    uint8_t active;
} AE3_BootFirstConstructor;
AE3_FirstCtorResult ae3_boot_first_ctor_enter(AE3_BootFirstConstructor *out,
    const AE3_BootStaticDispatch *prior);
AE3_FirstCtorResult ae3_boot_first_ctor_slice(AE3_BootFirstConstructor *s,
    AE3_EEMemoryBus *bus,uint32_t budget);
#endif