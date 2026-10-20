#ifndef AE3_BOOT_STATIC_DISPATCH_H
#define AE3_BOOT_STATIC_DISPATCH_H
#include "ae3_boot_main_resume.h"
/* Original 0x0010033C JALR through 0x00628210 and its two nested
 * initialization calls. Narrow EE MIPS interpreter: only pinned retail
 * instructions on these exact paths are accepted. NOT a PS2 BIOS.
 * The constructor list contents come from caller guest RAM, never fabricated.
 */
#define AE3_STATIC_INIT_ENTRY UINT32_C(0x0010033C)
#define AE3_STATIC_REGISTER_ENTRY UINT32_C(0x001000A0)
#define AE3_STATIC_CONSTRUCTOR_ENTRY UINT32_C(0x00428AB8)
#define AE3_STATIC_CONSTRUCTOR_TABLE UINT32_C(0x0062E89C)
#define AE3_STATIC_MAX_CONSTRUCTORS 256u
#define AE3_STATIC_NEXT_JAL UINT32_C(0x00100354)
#define AE3_STATIC_NEXT_CALLEE UINT32_C(0x00428B80)
typedef enum AE3_StaticInitResult {
    AE3_STATIC_YIELDED=1,
    AE3_STATIC_NEXT_CALL=2,
    AE3_STATIC_CONSTRUCTOR_PENDING=3,
    AE3_STATIC_DYNAMIC_REGISTRATION_PENDING=4,
    AE3_STATIC_MEMORY_FAULT=-1,
    AE3_STATIC_UNSUPPORTED=-2,
    AE3_STATIC_INVALID=-3
} AE3_StaticInitResult;
typedef struct AE3_BootStaticDispatch {
    AE3_BootPrefix cpu;
    uint32_t guest_ctor_word;
    uint32_t registration_word;
    uint32_t code_reads;
    uint8_t active;
    uint8_t registered_zero_path;
    uint8_t empty_ctor_path;
} AE3_BootStaticDispatch;
/* Read and validate original callback pointers in reverse-invocation order
 * until the retail 0xFFFFFFFF sentinel. Does NOT call any pointer. On a
 * bad pointer/mapping, the output is unchanged and the operation fails. */
typedef struct AE3_BootConstructorList {
    uint32_t count;
    uint32_t entries[AE3_STATIC_MAX_CONSTRUCTORS];
} AE3_BootConstructorList;
AE3_StaticInitResult ae3_boot_static_scan_constructors(
     AE3_BootConstructorList *out,const AE3_EEMemoryBus *bus);
AE3_StaticInitResult ae3_boot_static_enter(AE3_BootStaticDispatch *out,
                                             const AE3_BootMainResume *prior);
AE3_StaticInitResult ae3_boot_static_slice(AE3_BootStaticDispatch *state,
                                            AE3_EEMemoryBus *bus,
                                            uint32_t budget);
#endif