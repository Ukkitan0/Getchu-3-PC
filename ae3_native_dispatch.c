#include "ae3_native_dispatch.h"
#include "ae3_control_leaves.h"

int ae3_run_native_leaf(AE3_ScalarContext *ctx, uint32_t pc) {
    if (!ctx) return -1;
    int status = ae3_run_scalar_leaf(ctx, pc);
    if (status != -2) return status;
    return ae3_run_control_leaf(ctx, pc);
}
size_t ae3_native_leaf_count(void) {
    return ae3_scalar_leaf_count() + ae3_control_leaf_count();
}
uint32_t ae3_native_leaf_address(size_t index) {
    size_t n = ae3_scalar_leaf_count();
    return index < n ? ae3_scalar_leaf_address(index)
                     : ae3_control_leaf_address(index - n);
}
