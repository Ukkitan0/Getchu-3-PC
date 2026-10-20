#include "ae3_guest_execution.h"

static int is_known_leaf(uint32_t pc) {
    for (size_t i = 0, n = ae3_resume_count(); i < n; ++i)
        if (ae3_resume_address(i) == pc) return 1;
    return 0;
}
static int is_known_stack(uint32_t pc) {
    for (size_t i = 0, n = ae3_native_stack_count(); i < n; ++i)
        if (ae3_native_stack_address(i) == pc) return 1;
    return 0;
}

int ae3_guest_open(AE3_GuestExecution *exec, uint32_t entry) {
    if (!exec) return AE3_RESUME_INVALID;
    *exec = (AE3_GuestExecution){0};
    if (!entry) return AE3_RESUME_UNKNOWN;
    if (is_known_stack(entry)) {
        exec->kind = AE3_EXEC_STACK;
        exec->entry = entry;
        ae3_native_stack_init(&exec->state.stack, entry);
        return AE3_RESUME_DONE;
    }
    if (is_known_leaf(entry)) {
        exec->kind = AE3_EXEC_LEAF;
        exec->entry = entry;
        ae3_native_frame_init(&exec->state.leaf, entry);
        return AE3_RESUME_DONE;
    }
    return AE3_RESUME_UNKNOWN;
}

int ae3_guest_slice(AE3_GuestExecution *exec, AE3_ScalarContext *ctx,
                    uint32_t instruction_budget) {
    if (!exec || !ctx) return AE3_RESUME_INVALID;
    switch(exec->kind) {
        case AE3_EXEC_LEAF:
            if (exec->entry != exec->state.leaf.entry) return AE3_RESUME_INVALID;
            return ae3_run_native_slice(ctx, &exec->state.leaf, instruction_budget);
        case AE3_EXEC_STACK:
            if (exec->entry != exec->state.stack.entry) return AE3_RESUME_INVALID;
            return ae3_run_native_stack_slice(ctx, &exec->state.stack, instruction_budget);
        case AE3_EXEC_NONE: return AE3_RESUME_UNKNOWN;
        default: return AE3_RESUME_INVALID;
    }
}

uint32_t ae3_guest_pc(const AE3_GuestExecution *exec) {
    if (!exec) return 0;
    if (exec->kind == AE3_EXEC_LEAF) {
        const AE3_ResumeFrame *f = &exec->state.leaf;
        return !f->done && f->entry == exec->entry ? f->pc : 0;
    }
    if (exec->kind == AE3_EXEC_STACK) {
        const AE3_StackFrame *f = &exec->state.stack;
        if (f->done || !f->depth || f->depth > AE3_STACK_MAX_DEPTH ||
            f->entry != exec->entry) return 0;
        const AE3_StackActivation *a = &f->frames[f->depth-1u];
        return a->leaf_active ? a->leaf.pc : a->pc;
    }
    return 0;
}
uint64_t ae3_guest_retired(const AE3_GuestExecution *exec) {
    if (!exec) return 0;
    if (exec->kind == AE3_EXEC_LEAF) return exec->state.leaf.retired;
    if (exec->kind == AE3_EXEC_STACK) return exec->state.stack.retired;
    return 0;
}
int ae3_guest_done(const AE3_GuestExecution *exec) {
    if (!exec) return 0;
    if (exec->kind == AE3_EXEC_LEAF) return exec->state.leaf.done == 1;
    if (exec->kind == AE3_EXEC_STACK) return exec->state.stack.done == 1;
    return 0;
}

size_t ae3_guest_entry_count(void) {
    return ae3_resume_count() + ae3_native_stack_count();
}
uint32_t ae3_guest_entry_at(size_t index) {
    size_t leaves = ae3_resume_count();
    if (index < leaves) return ae3_resume_address(index);
    index -= leaves;
    return index < ae3_native_stack_count() ? ae3_native_stack_address(index) : 0;
}