#ifndef AE3_BOOT_POST_FIND_H
#define AE3_BOOT_POST_FIND_H
#include "ae3_guest_find_address.h"
#include "ae3_ee_memory_bus.h"
/* Original retail address-reconciliation loop at 0x0043FDC4. This is
 * execution of guest MIPS code, NOT an assumed successful BIOS lookup. */
#define AE3_POST_FIND_ENTRY UINT32_C(0x0043FDC4)
#define AE3_POST_FIND_NEXT_CALL UINT32_C(0x0043FE70)
#define AE3_POST_FIND_SYSCALL UINT32_C(0x0043FD04)

typedef enum AE3_PostFindResult {
    AE3_POST_YIELDED=1,
    AE3_POST_FIND_REQUEST=2,
    AE3_POST_NEXT_INIT=3,
    AE3_POST_MEMORY_FAULT=-1,
    AE3_POST_UNSUPPORTED=-2,
    AE3_POST_INVALID=-3
} AE3_PostFindResult;

typedef struct AE3_BootPostFind {
    AE3_BootPrefix cpu;
    uint32_t search_requests;
    uint8_t done;
} AE3_BootPostFind;

AE3_PostFindResult ae3_boot_post_find_enter(AE3_BootPostFind *out,
    const AE3_BootHookInit *hook);
AE3_PostFindResult ae3_boot_post_find_slice(AE3_BootPostFind *state,
    AE3_EEMemoryBus *bus, uint32_t instruction_budget);
/* A pending syscall can be handed to the REAL registered MIPS FindAddress
   implementation. This is a state adapter, NOT a synthetic syscall answer. */
AE3_PostFindResult ae3_boot_post_find_begin_guest(
    const AE3_BootPostFind *state, AE3_GuestFindAddress *guest);
AE3_PostFindResult ae3_boot_post_find_finish_guest(
    AE3_BootPostFind *state, const AE3_GuestFindAddress *guest);
#endif