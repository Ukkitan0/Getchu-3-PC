#ifndef AE3_GUEST_FIND_ADDRESS_H
#define AE3_GUEST_FIND_ADDRESS_H
#include "ae3_boot_hook_init.h"
#include "ae3_ee_memory_bus.h"

/* Execute the ORIGINAL retail guest code at 0x0043FCC0 (installed as EE
   syscall 0x83), using caller-supplied kernel memory. No synthetic BIOS
   contents are created by this module. The host must supply an authentic
   kernel-memory mapping for meaningful game behavior. */
#define AE3_FIND_ENTRY UINT32_C(0x0043FCC0)
#define AE3_FIND_END UINT32_C(0x0043FD00)
typedef enum AE3_FindResult {
    AE3_FIND_YIELDED=1,
    AE3_FIND_RETURNED=2,
    AE3_FIND_KERNEL_UNMAPPED=-1,
    AE3_FIND_UNSUPPORTED=-2,
    AE3_FIND_INVALID=-3
} AE3_FindResult;
typedef struct AE3_GuestFindAddress {
    AE3_BootPrefix cpu;
    uint32_t return_pc;
    uint32_t matched_guest_address;
    uint32_t loads;
    uint8_t done;
} AE3_GuestFindAddress;
AE3_FindResult ae3_guest_find_begin(AE3_GuestFindAddress *out,
    const AE3_BootHookInit *hook);
/* Kernel bytes are interpreted as a guest KSEG0 linear window starting at
   guest_base. Passing NULL or an incomplete window produces an atomic fault. */
AE3_FindResult ae3_guest_find_slice(AE3_GuestFindAddress *state,
    const uint8_t *original_ee_ram, size_t ee_bytes,
    const uint8_t *kernel_bytes, uint32_t guest_base,
    size_t kernel_bytes_count, uint32_t instruction_budget);
/* Read the registered handler from the SAME physical EE RAM image used for
   executing guest code, with kernel segment aliasing. This exposes unmapped
   boot-time kernel contents instead of populating invented bytes. */
AE3_FindResult ae3_guest_find_slice_bus(AE3_GuestFindAddress *state,
    const AE3_EEMemoryBus *bus, uint32_t instruction_budget);
/* Retire the interrupted syscall only after the guest handler really returns.
   Its result is the guest's $v0, copied verbatim with no guessed kernel value. */
AE3_FindResult ae3_guest_find_commit(AE3_BootHookInit *hook,
    const AE3_GuestFindAddress *completed);
#endif