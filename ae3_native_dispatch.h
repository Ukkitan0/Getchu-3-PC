#ifndef AE3_NATIVE_DISPATCH_H
#define AE3_NATIVE_DISPATCH_H
#include "ae3_call_leaves.h"
#include "ae3_stack_leaves.h"
/* Unified native code dispatch for recovered routines; DOES NOT implement PS2 hardware. */
int ae3_run_native_leaf(AE3_ScalarContext *ctx, uint32_t pc);
/* Instruction-slice API; caller owns resumable execution state. */
void ae3_native_frame_init(AE3_ResumeFrame *frame, uint32_t entry);
int ae3_run_native_slice(AE3_ScalarContext *ctx, AE3_ResumeFrame *frame,
                        uint32_t instruction_budget);
/* Additional recovered direct-JAL callers; these are not scalar leaves. */
void ae3_native_caller_init(AE3_CallFrame *frame, uint32_t entry);
int ae3_run_native_caller_slice(AE3_ScalarContext *ctx, AE3_CallFrame *frame,
                                uint32_t instruction_budget);
size_t ae3_native_caller_count(void);
uint32_t ae3_native_caller_address(size_t index);
/* Nested JAL and whitelisted virtual JALR execution; not a full PS2 CPU. */
void ae3_native_stack_init(AE3_StackFrame *frame,uint32_t entry);
int ae3_run_native_stack_slice(AE3_ScalarContext *ctx,AE3_StackFrame *frame,uint32_t budget);
size_t ae3_native_stack_count(void);
uint32_t ae3_native_stack_address(size_t index);
size_t ae3_native_leaf_count(void);
uint32_t ae3_native_leaf_address(size_t index);
#endif