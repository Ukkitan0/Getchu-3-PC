#ifndef AE3_BOOT_TLB_FINALIZE_H
#define AE3_BOOT_TLB_FINALIZE_H
#include "ae3_boot_tlb_patch.h"
/* Execute the retail copied 0x5B GetEntryAddress guest MIPS search (6 calls),
 * five guest SetSyscall calls, and original function epilogue.
 * Data is fetched from the copied guest KSEG0 table, never hardcoded as
 * a BIOS response. This is strictly experimental compatibility execution. */
#define AE3_TLB_FINAL_RETURN UINT32_C(0x0043FEA0)
typedef enum AE3_TlbFinishResult {
 AE3_TLBF_YIELDED=1, AE3_TLBF_GET_ENTRY=2,
 AE3_TLBF_SET_SYSCALL=3, AE3_TLBF_RETURNED=4,
 AE3_TLBF_MEMORY_FAULT=-1, AE3_TLBF_UNSUPPORTED=-2,AE3_TLBF_INVALID=-3
} AE3_TlbFinishResult;
typedef struct AE3_BootTlbFinalize {
 AE3_BootPrefix cpu;
 uint32_t entries,sets,table_reads;
 uint8_t active,entry_active,done;
} AE3_BootTlbFinalize;
AE3_TlbFinishResult ae3_boot_tlb_finish_enter(AE3_BootTlbFinalize *out,
                                             const AE3_BootTlbPatch *prior);
AE3_TlbFinishResult ae3_boot_tlb_finish_slice(AE3_BootTlbFinalize *s,
                                             AE3_EEMemoryBus *bus,uint32_t budget);
AE3_TlbFinishResult ae3_boot_tlb_finish_service(AE3_BootTlbFinalize *s,
                                             AE3_EEMemoryBus *bus,
                                             AE3_EESyscallKernel *kernel);
#endif