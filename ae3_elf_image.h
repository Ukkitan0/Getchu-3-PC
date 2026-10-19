#ifndef AE3_ELF_IMAGE_H
#define AE3_ELF_IMAGE_H
#include <stddef.h>
#include <stdint.h>

/* ELF32 little-endian MIPS executable mapper for the EE guest RAM image.
 * It never executes code, emulates PS2 devices, or allocates memory.
 * Caller supplies an already allocated 32 MB (or larger) EE memory arena.
 */
#define AE3_ELF_IMAGE_OK 0
#define AE3_ELF_IMAGE_INVALID (-1)
#define AE3_ELF_IMAGE_UNSUPPORTED (-2)
#define AE3_ELF_IMAGE_OUT_OF_BOUNDS (-3)
typedef struct AE3_ElfImage {
    uint32_t entry;
    uint32_t segment_count;
    uint64_t copied_bytes;
    uint64_t zeroed_bss_bytes;
} AE3_ElfImage;

/* Preflight checks the full ELF before writing a single guest byte.
 * Invalid/unsupported input and address conflicts leave RAM untouched.
 * This generic ELF reader does not independently authenticate game revision;
 * retain the existing SHA-256 check of SCUS_975.01 in the build workflow.
 */
int ae3_load_ee_elf32(const uint8_t *elf, size_t elf_size,
                     uint8_t *ram, size_t ram_size, AE3_ElfImage *report);
#endif