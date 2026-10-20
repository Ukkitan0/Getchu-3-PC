#ifndef AE3_BOOT_POST_TLB_H
#define AE3_BOOT_POST_TLB_H
#include "ae3_boot_tlb_finalize.h"
/* Execute the retail tail-call at 0x43FEA0 and its UNALIGNED 512-byte
 * memset at 0x42B320, including 512 authentic SB instructions and all
 * branch delay slots. Stop BEFORE the JR to an unknown boot-time $ra.
 * This is a bounded original-instruction path, not Sony BIOS execution. */
#define AE3_POST_TLB_ENTRY UINT32_C(0x0043FEA0)
#define AE3_POST_TLB_ZERO_START UINT32_C(0x00771288)
#define AE3_POST_TLB_ZERO_BYTES UINT32_C(512)
#define AE3_POST_TLB_RETURN_PC UINT32_C(0x0042B3D8)

typedef enum AE3_PostTlbResult {
 AE3_PTL_YIELDED=1, AE3_PTL_RETURN_PENDING=2,
 AE3_PTL_MEMORY_FAULT=-1, AE3_PTL_UNSUPPORTED=-2, AE3_PTL_INVALID=-3
} AE3_PostTlbResult;
typedef struct AE3_BootPostTlb {
 AE3_BootPrefix cpu;
 uint32_t zero_stores;
 uint32_t global_stores;
 uint8_t active;
} AE3_BootPostTlb;
AE3_PostTlbResult ae3_boot_post_tlb_enter(AE3_BootPostTlb *out,
                                          const AE3_BootTlbFinalize *prior);
AE3_PostTlbResult ae3_boot_post_tlb_slice(AE3_BootPostTlb *state,
                                          AE3_EEMemoryBus *bus,
                                          uint32_t budget);
#endif