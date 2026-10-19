#include "ae3_native_dispatch.h"
#include "ae3_control_leaves.h"
#include "ae3_loop_leaves.h"

int ae3_run_native_leaf(AE3_ScalarContext *ctx, uint32_t pc) {
    if (!ctx) return -1;
    int status = ae3_run_scalar_leaf(ctx, pc);
    if (status != -2) return status;
    status = ae3_run_control_leaf(ctx, pc);
    if (status != -2) return status;
    return ae3_run_loop_leaf(ctx, pc);
}
size_t ae3_native_leaf_count(void) {
    return ae3_scalar_leaf_count() + ae3_control_leaf_count() + ae3_loop_leaf_count();
}
uint32_t ae3_native_leaf_address(size_t index) {
    size_t n0=ae3_scalar_leaf_count(), n1=ae3_control_leaf_count();
    if (index < n0) return ae3_scalar_leaf_address(index);
    if (index < n0+n1) return ae3_control_leaf_address(index-n0);
    return ae3_loop_leaf_address(index-n0-n1);
}

void ae3_native_frame_init(AE3_ResumeFrame *frame, uint32_t entry) {
    ae3_resume_init(frame, entry);
}
int ae3_run_native_slice(AE3_ScalarContext *ctx, AE3_ResumeFrame *frame,
                        uint32_t instruction_budget) {
    return ae3_run_resume_slice(ctx, frame, instruction_budget);
}

void ae3_native_caller_init(AE3_CallFrame *frame,uint32_t entry) {
    ae3_call_init(frame,entry);
}
int ae3_run_native_caller_slice(AE3_ScalarContext *ctx,AE3_CallFrame *frame,
                                uint32_t instruction_budget) {
    return ae3_run_call_slice(ctx,frame,instruction_budget);
}
size_t ae3_native_caller_count(void) {return ae3_call_count();}
uint32_t ae3_native_caller_address(size_t index) {return ae3_call_address(index);}

void ae3_native_stack_init(AE3_StackFrame *frame,uint32_t entry){ae3_stack_init(frame,entry);}
int ae3_run_native_stack_slice(AE3_ScalarContext *ctx,AE3_StackFrame *frame,uint32_t budget){
    return ae3_run_stack_slice(ctx,frame,budget);
}
size_t ae3_native_stack_count(void){return ae3_stack_count();}
uint32_t ae3_native_stack_address(size_t index){return ae3_stack_address(index);}