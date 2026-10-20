#ifndef AE3_GUEST_EXECUTION_H
#define AE3_GUEST_EXECUTION_H

#include "ae3_native_dispatch.h"

/*
 * Runtime-facing adapter to the *limited verified translation subset*.
 * Caller owns AE3_ScalarContext, mapped RAM, and the execution structure.
 * These APIs never emulate unknown instructions or PS2 hardware.
 * No raw pointer to the original ELF is kept by this module.
 */
#define AE3_EXEC_NONE 0u
#define AE3_EXEC_LEAF 1u
#define AE3_EXEC_STACK 2u

typedef struct AE3_GuestExecution {
    uint32_t entry;
    uint32_t kind;
    union {
        AE3_ResumeFrame leaf;
        AE3_StackFrame stack;
    } state;
} AE3_GuestExecution;

/* Returns 0 for a supported entry, -2 for unknown, -4 for bad arguments.
 * A rejected entry is left uninitialized (kind=NONE, entry=0).
 * Caller may open again to start the same function from scratch. */
int ae3_guest_open(AE3_GuestExecution *exec, uint32_t entry);

/* Return AE3_RESUME_* codes: DONE, YIELDED, MEMORY_FAULT, UNKNOWN, INVALID.
 * Zero budget is a yield, even for an otherwise valid idle execution.
 * On a fault, the original guest PC remains available for retry. */
int ae3_guest_slice(AE3_GuestExecution *exec, AE3_ScalarContext *ctx,
                    uint32_t instruction_budget);

/* Deepest currently executing PC (the nested leaf if one is active), or zero
 * after completion / before initialization. */
uint32_t ae3_guest_pc(const AE3_GuestExecution *exec);
uint64_t ae3_guest_retired(const AE3_GuestExecution *exec);
int ae3_guest_done(const AE3_GuestExecution *exec);

/* Introspection covers only accepted entrypoints, never arbitrary ELF code. */
size_t ae3_guest_entry_count(void);
uint32_t ae3_guest_entry_at(size_t index);

#endif