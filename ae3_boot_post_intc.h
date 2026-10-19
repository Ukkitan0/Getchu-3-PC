#ifndef AE3_BOOT_POST_INTC_H
#define AE3_BOOT_POST_INTC_H
#include "ae3_boot_alloc_init.h"
#include "ae3_ee_intc_kernel.h"
/* Original retail AddIntcHandler syscall 0x10, original MIPS wrapper return,
 * then branch-likely in the allocator. Ends BEFORE guest JAL 0x43FBA0.
 * Only the BIOS service itself is compatibility-mode; instructions are guest.
 */
#define AE3_POST_INTC_NEXT_PC UINT32_C(0x00440578)
#define AE3_POST_INTC_FAIL_PC UINT32_C(0x004405E4)
typedef enum AE3_PostIntcResult {
    AE3_POST_INTC_YIELDED=1,
    AE3_POST_INTC_NEXT_CALL=2,
    AE3_POST_INTC_FAILED_PATH=3,
    AE3_POST_INTC_MEMORY_FAULT=-1,
    AE3_POST_INTC_UNSUPPORTED=-2,
    AE3_POST_INTC_INVALID=-3
} AE3_PostIntcResult;
typedef struct AE3_BootPostIntc {
    AE3_BootPrefix cpu;
    uint32_t handler_id;
    uint8_t serviced;
} AE3_BootPostIntc;
AE3_PostIntcResult ae3_boot_post_intc_enter(AE3_BootPostIntc *out,
    const AE3_BootAllocInit *alloc);
AE3_PostIntcResult ae3_boot_post_intc_service(AE3_BootPostIntc *state,
    AE3_EEIntcKernel *kernel,const AE3_EEMemoryBus *bus);
AE3_PostIntcResult ae3_boot_post_intc_slice(AE3_BootPostIntc *state,
    const AE3_EEMemoryBus *bus,uint32_t budget);
#endif