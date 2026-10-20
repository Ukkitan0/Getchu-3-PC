#ifndef AE3_BOOT_MAIN_RESUME_H
#define AE3_BOOT_MAIN_RESUME_H
#include "ae3_boot_post_tlb.h"
/* The *retail call site* 0x0010031C contains a unique JAL to the
 * initializer at 0x0043FE58, giving guest link address 0x00100324.
 * However the real saved $ra is READ, never inserted or invented here.
 * Returns from original memset; runs the original FlushCache syscall 0x64;
 * stops BEFORE the game's dynamic JALR to 0x00628210. */
#define AE3_MAIN_CALLER_PC UINT32_C(0x00100324)
#define AE3_MAIN_NEXT_JALR UINT32_C(0x0010033C)
#define AE3_MAIN_DYNAMIC_TARGET UINT32_C(0x00628210)
#define AE3_MAIN_FLUSH_SYSCALL UINT32_C(0x00435D64)
typedef enum AE3_MainResumeResult {
 AE3_MAIN_YIELDED=1, AE3_MAIN_FLUSH_PENDING=2,
 AE3_MAIN_NEXT_CALL=3, AE3_MAIN_MEMORY_FAULT=-1,
 AE3_MAIN_UNSUPPORTED=-2,AE3_MAIN_INVALID=-3
} AE3_MainResumeResult;
typedef struct AE3_BootMainResume {
 AE3_BootPrefix cpu;
 uint32_t flush_requests;
 uint32_t flush_mode;
 uint8_t active;
} AE3_BootMainResume;
AE3_MainResumeResult ae3_boot_main_resume_enter(AE3_BootMainResume *out,
                                                 const AE3_BootPostTlb *prior);
AE3_MainResumeResult ae3_boot_main_resume_slice(AE3_BootMainResume *state,
                                                 AE3_EEMemoryBus *bus,
                                                 uint32_t budget);
AE3_MainResumeResult ae3_boot_main_resume_flush(AE3_BootMainResume *state,
                                                 const AE3_EEMemoryBus *bus);
#endif