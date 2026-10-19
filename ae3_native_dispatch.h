#ifndef AE3_NATIVE_DISPATCH_H
#define AE3_NATIVE_DISPATCH_H
#include "ae3_resumable_leaves.h"
/* Unified native code dispatch for recovered routines; DOES NOT implement PS2 hardware. */
int ae3_run_native_leaf(AE3_ScalarContext *ctx, uint32_t pc);
/* Instruction-slice API; caller owns resumable execution state. */
void ae3_native_frame_init(AE3_ResumeFrame *frame, uint32_t entry);
int ae3_run_native_slice(AE3_ScalarContext *ctx, AE3_ResumeFrame *frame,
                        uint32_t instruction_budget);
size_t ae3_native_leaf_count(void);
uint32_t ae3_native_leaf_address(size_t index);
#endif