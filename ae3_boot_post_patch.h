#ifndef AE3_BOOT_POST_PATCH_H
#define AE3_BOOT_POST_PATCH_H
#include "ae3_boot_timer3_syscalls.h"
#include "ae3_guest_get_entry.h"

/* Original retail code after the 0x5A kernel payload copy at 0x00440398.
 * Runs a second 40-byte guest Copy into EE RAM at 0x00082000, requests two
 * FlushCache(0x64) kernel services, registers syscall 0x5B, then executes the
 * installed guest GetEntryAddress(0x5B) routine six times, registers the
 * resulting handlers, and returns from the original initializer.
 * 
 * All syscall dispatch is explicitly compatibility-mode service handling.
 * It is NOT an EE exception-vector or hardware cache implementation. */
#define AE3_POST_PATCH_ENTRY UINT32_C(0x00440398)
#define AE3_POST_PATCH_COPY_DEST UINT32_C(0x00082000)
#define AE3_POST_PATCH_COPY_SOURCE UINT32_C(0x006A2DB8)
#define AE3_POST_PATCH_COPY_LENGTH 40u
#define AE3_POST_PATCH_GET_ENTRY_PC UINT32_C(0x0044032C)

typedef enum AE3_PostPatchResult {
 AE3_PP_YIELDED=1,
 AE3_PP_COPY_REQUEST=2,
 AE3_PP_FLUSH_REQUEST=3,
 AE3_PP_REGISTER_REQUEST=4,
 AE3_PP_GET_ENTRY_REQUEST=5,
 AE3_PP_RETURNED=6,
 AE3_PP_MEMORY_FAULT=-1,
 AE3_PP_UNSUPPORTED=-2,
 AE3_PP_INVALID=-3
} AE3_PostPatchResult;

typedef struct AE3_BootPostPatch {
 AE3_BootPrefix cpu;
 AE3_GuestKernelCopy copy;
 AE3_GuestGetEntry getentry;
 uint32_t flush_modes[2];
 uint8_t flush_count;
 uint8_t copy_active;
 uint8_t copy_completed;
 uint8_t registered_5b;
 uint8_t getentry_active;
 uint8_t getentry_count;
 uint8_t registered_table_count;
 uint8_t done;
} AE3_BootPostPatch;

AE3_PostPatchResult ae3_boot_post_patch_enter(AE3_BootPostPatch *out,
 const AE3_Timer3Syscalls *prior);
AE3_PostPatchResult ae3_boot_post_patch_slice(AE3_BootPostPatch *s,
 AE3_EEMemoryBus *bus, uint32_t instruction_budget);
/* Explicit compatibility kernel service actions; no automatic dispatch. */
AE3_PostPatchResult ae3_boot_post_patch_dispatch(AE3_BootPostPatch *s,
 AE3_EEMemoryBus *bus, AE3_EESyscallKernel *kernel);
#endif