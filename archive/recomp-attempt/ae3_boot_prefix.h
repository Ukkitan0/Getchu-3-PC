#ifndef AE3_BOOT_PREFIX_H
#define AE3_BOOT_PREFIX_H
#include <stddef.h>
#include <stdint.h>

/* A deliberately bounded interpreter for the *retail game's* first EE startup
 * sequence.  The original ELF must first be mapped with ae3_load_ee_elf32.
 * This does NOT run the callee at 0x001036D8, initialize PS2 devices, or boot
 * the game. The two 64-bit words form a full EE 128-bit GPR, not just a
 * scalar approximation. Host pointers are never stored in guest state. */
#define AE3_BOOT_ENTRY UINT32_C(0x00100134)
#define AE3_BOOT_FIRST_CALL UINT32_C(0x001002C4)
#define AE3_BOOT_CLEAR_START UINT32_C(0x0072C600)
#define AE3_BOOT_CLEAR_END UINT32_C(0x00772910)

typedef enum AE3_BootResult {
    AE3_BOOT_YIELDED = 1,
    AE3_BOOT_FIRST_CALL_REACHED = 2,
    AE3_BOOT_MEMORY_FAULT = -1,
    AE3_BOOT_UNSUPPORTED_INSTRUCTION = -2,
    AE3_BOOT_INVALID_STATE = -3
} AE3_BootResult;

typedef struct AE3_BootPrefix {
    uint64_t gpr[32][2];
    uint64_t hilo[2][2];        /* [bank 0|1][HI|LO] */
    uint32_t fpr[32];
    uint32_t fpu_acc;
    uint32_t fcr31;
    uint32_t pc;
    uint32_t branch_target;
    uint64_t retired;
    uint32_t byte_stores;
    uint32_t quad_stores;
    uint8_t in_delay_slot;
    uint8_t done;
} AE3_BootPrefix;

/* Initial state is all zero; callers may seed individual registers for tests.
 * Guest memory is caller-owned; this function never allocates or persists RAM. */
void ae3_boot_prefix_init(AE3_BootPrefix *state);

/* Runs no more than budget guest instructions.  An unknown opcode or unmapped
 * memory leaves the faulting instruction's PC and architectural state intact;
 * after fixing the mapping, call again.  Returns when the next instruction is
 * the game's first JAL; never executes the JAL itself. */
AE3_BootResult ae3_boot_prefix_slice(AE3_BootPrefix *state,
    uint8_t *ram, size_t ram_size, uint32_t budget);

#endif