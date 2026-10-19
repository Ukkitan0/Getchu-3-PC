#ifndef AE3_GUEST_GET_ENTRY_H
#define AE3_GUEST_GET_ENTRY_H
#include "ae3_boot_prefix.h"
#include "ae3_ee_memory_bus.h"
/* Execute the REAL copied game kernel handler at KSEG0 0x80076000, originally
 * stored in .data at 0x006A2678. Uses its six guest table entries in
 * 0x80076710..0x8007673F. Guest memory is read, never synthesized. */
#define AE3_GET_ENTRY_GUEST_PC UINT32_C(0x80076000)
#define AE3_GET_ENTRY_GUEST_END UINT32_C(0x80076038)
#define AE3_GET_ENTRY_GUEST_TABLE UINT32_C(0x80076710)
typedef enum AE3_GetEntryResult {
 AE3_GET_ENTRY_YIELDED=1, AE3_GET_ENTRY_RETURNED=2,
 AE3_GET_ENTRY_MEMORY_FAULT=-1,AE3_GET_ENTRY_UNSUPPORTED=-2,
 AE3_GET_ENTRY_INVALID=-3
} AE3_GetEntryResult;
typedef struct AE3_GuestGetEntry {
 AE3_BootPrefix cpu;
 uint32_t return_pc;
 uint32_t table_reads;
 uint8_t done;
} AE3_GuestGetEntry;
AE3_GetEntryResult ae3_guest_get_entry_init(AE3_GuestGetEntry*out,
 const AE3_BootPrefix*syscall_caller,uint32_t return_pc);
AE3_GetEntryResult ae3_guest_get_entry_slice(AE3_GuestGetEntry *s,
 const AE3_EEMemoryBus *bus,uint32_t instruction_budget);
#endif