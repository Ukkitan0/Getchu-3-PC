#ifndef AE3_BOOT_TLB_PATCH_H
#define AE3_BOOT_TLB_PATCH_H
#include "ae3_boot_osd_roundtrip.h"
#include "ae3_ee_syscall_kernel.h"
/* Executes retail EE InitTLBFunctions code through the FIRST GetEntryAddress
 * syscall at 0x0043F454, including the 204-iteration original 0x5A Copy
 * guest handler. Device state, BIOS context and cache effects are NOT modeled.
 * This is a bounded, instruction-verified experimental compatibility path. */
#define AE3_TLB_GUEST_ENTRY UINT32_C(0x0043FE98)
#define AE3_TLB_GUEST_GET_ENTRY UINT32_C(0x0043F454)
#define AE3_TLB_COPY_SRC UINT32_C(0x006A1840)
#define AE3_TLB_COPY_DST UINT32_C(0x80075000)
#define AE3_TLB_COPY_BYTES UINT32_C(0x330)
typedef enum AE3_TlbResult {
 AE3_TLB_YIELDED=1,AE3_TLB_SET_PENDING=2,AE3_TLB_COPY_PENDING=3,
 AE3_TLB_FLUSH_PENDING=4,AE3_TLB_GET_ENTRY_PENDING=5,
 AE3_TLB_MEMORY_FAULT=-1,AE3_TLB_UNSUPPORTED=-2,AE3_TLB_INVALID=-3
} AE3_TlbResult;
typedef struct AE3_BootTlbPatch {
 AE3_BootPrefix cpu;
 uint32_t copy_words;
 uint8_t entered,copy_active,copy_done,flushes,registrations;
 uint32_t flush_modes[2];
} AE3_BootTlbPatch;
AE3_TlbResult ae3_boot_tlb_enter(AE3_BootTlbPatch *dst,
                                 const AE3_BootOsdRoundtrip *prior);
AE3_TlbResult ae3_boot_tlb_slice(AE3_BootTlbPatch *state,
                                 AE3_EEMemoryBus *bus,uint32_t budget);
AE3_TlbResult ae3_boot_tlb_service(AE3_BootTlbPatch *state,
                                   AE3_EEMemoryBus *bus,
                                   AE3_EESyscallKernel *kernel);
#endif