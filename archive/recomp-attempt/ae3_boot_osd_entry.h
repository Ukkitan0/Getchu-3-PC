#ifndef AE3_BOOT_OSD_ENTRY_H
#define AE3_BOOT_OSD_ENTRY_H
#include "ae3_boot_thread_priority.h"
/* Retail parent initializer 0x43FE90 -> 0x43FF80 -> 0x43FF18,
 * stopping at GetOsdConfigParam (0x4B). No BIOS OSD data is fabricated. */
#define AE3_OSD_SYSCALL_PC UINT32_C(0x00435B94)
typedef enum AE3_OsdEntryResult {
 AE3_OSD_YIELDED=1,AE3_OSD_SYSCALL_PENDING=2,
 AE3_OSD_MEMORY_FAULT=-1,AE3_OSD_UNSUPPORTED=-2,AE3_OSD_INVALID=-3
} AE3_OsdEntryResult;
typedef struct AE3_BootOsdEntry {AE3_BootPrefix cpu;uint8_t active;} AE3_BootOsdEntry;
AE3_OsdEntryResult ae3_boot_osd_enter(AE3_BootOsdEntry *out,
 const AE3_BootThreadPriority *prior);
AE3_OsdEntryResult ae3_boot_osd_slice(AE3_BootOsdEntry *s,
 AE3_EEMemoryBus *bus,uint32_t budget);
#endif