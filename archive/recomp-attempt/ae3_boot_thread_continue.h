#ifndef AE3_BOOT_THREAD_CONTINUE_H
#define AE3_BOOT_THREAD_CONTINUE_H
#include "ae3_boot_event_create.h"
#include "ae3_ee_thread_kernel.h"
/* Real retail instructions through CreateThread JR and the caller's branch.
 * CreateThread + DeleteSema syscall effects use opt-in compatibility objects. */
#define AE3_THR_CREATE_PC UINT32_C(0x004358E4)
#define AE3_THR_DELETE_PC UINT32_C(0x00435AF4)
#define AE3_THR_NEXT_CALLEE UINT32_C(0x00436C20)
#define AE3_THR_PARENT_NEXT UINT32_C(0x0043FE90)
typedef enum AE3_ThreadBootResult {
 AE3_THR_YIELDED=1,
 AE3_THR_CREATE_PENDING=2,
 AE3_THR_DELETE_PENDING=3,
 AE3_THR_NEXT_CALL=4,
 AE3_THR_RETURNED=5,
 AE3_THR_MEMORY_FAULT=-1,
 AE3_THR_UNSUPPORTED=-2,
 AE3_THR_INVALID=-3
} AE3_ThreadBootResult;
typedef struct AE3_BootThreadContinue {
 AE3_BootPrefix cpu;
 int32_t thread_id;
 uint32_t descriptor_address;
 uint32_t descriptor[AE3_EE_THREAD_DESC_WORDS];
 uint8_t active,create_serviced,delete_serviced;
} AE3_BootThreadContinue;
AE3_ThreadBootResult ae3_boot_thread_continue_enter(AE3_BootThreadContinue *s,
 const AE3_BootEventInit *prior);
AE3_ThreadBootResult ae3_boot_thread_continue_slice(AE3_BootThreadContinue *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
AE3_ThreadBootResult ae3_boot_thread_create(AE3_BootThreadContinue *s,
 AE3_EEMemoryBus *bus,AE3_EEThreadKernel *kernel);
/* DeleteSema 0x41 failure-cleanup service uses the same semaphore registry. */
AE3_ThreadBootResult ae3_boot_thread_delete_sema(AE3_BootThreadContinue *s,
 const AE3_EEMemoryBus *bus,AE3_EESemaKernel *kernel);
#endif