#ifndef AE3_BOOT_ALLOC_INIT_H
#define AE3_BOOT_ALLOC_INIT_H
#include "ae3_boot_post_patch.h"
#include "ae3_ee_memory_bus.h"
/* Original EE code after successful 0x440338 kernel initialization:
 * 0x43FE78 -> 0x4404B0, then (on retail's -1 initial state)
 * a native-C translation of the known zero-fill helper at 0x42B320,
 * a 128-entry pointer chain, and the original 64-node list initializer
 * at 0x441780. Stop BEFORE the next unknown nested callee 0x4357D0.
 * This is not a BIOS replacement or a completed allocator. */
#define AE3_ALLOC_PARENT_JAL UINT32_C(0x0043FE78)
#define AE3_ALLOC_ENTRY UINT32_C(0x004404B0)
#define AE3_ALLOC_MEMSET_ENTRY UINT32_C(0x0042B320)
#define AE3_ALLOC_FREELIST_ENTRY UINT32_C(0x00441780)
#define AE3_ALLOC_NEXT_CALL UINT32_C(0x00440558)
#define AE3_ALLOC_INTC_WRAPPER UINT32_C(0x004357D0)
#define AE3_ALLOC_INTC_SYSCALL_PC UINT32_C(0x004357D4)
#define AE3_ALLOC_PARENT_RETURN UINT32_C(0x0043FE80)
#define AE3_ALLOC_AREA UINT32_C(0x0076EE80)
#define AE3_ALLOC_CLEAR_BYTES UINT32_C(0x00002000)

typedef enum AE3_AllocResult {
    AE3_ALLOC_YIELDED=1,
    AE3_ALLOC_ZERO_FILL_REQUEST=2,
    AE3_ALLOC_NEXT_CALL_PENDING=3,
    AE3_ALLOC_RETURNED=4,
    AE3_ALLOC_INTC_SYSCALL=5,
    AE3_ALLOC_MEMORY_FAULT=-1,
    AE3_ALLOC_UNSUPPORTED=-2,
    AE3_ALLOC_INVALID=-3
} AE3_AllocResult;
typedef struct AE3_BootAllocInit {
    AE3_BootPrefix cpu;
    uint32_t pointer_stores;
    uint32_t freelist_stores;
    uint32_t translated_zero_fills;
    uint8_t initialized_path;
    uint8_t done;
} AE3_BootAllocInit;
AE3_AllocResult ae3_boot_alloc_enter(AE3_BootAllocInit *out,
                                      const AE3_BootPostPatch *previous);
AE3_AllocResult ae3_boot_alloc_slice(AE3_BootAllocInit *state,
    AE3_EEMemoryBus *bus,uint32_t instruction_budget);
/* Native semantic translation of the *specific* recognized retail memset
 * call (zero, aligned 8192 bytes). Invoked only by an explicit host action.
 * Does not falsely count the original MMI instructions as retired. */
AE3_AllocResult ae3_boot_alloc_dispatch_zero(AE3_BootAllocInit *state,
    AE3_EEMemoryBus *bus);
/* Follow the already identified AddIntcHandler (EE syscall 0x10) wrapper,
 * executing its guest JAL/delay-slot and ADDIU $v1 instruction, but NEVER
 * fabricate an interrupt registration or a return value. */
AE3_AllocResult ae3_boot_alloc_follow_intc(AE3_BootAllocInit *state,
    AE3_EEMemoryBus *bus);
#endif