#include "ae3_native_dispatch.h"
#include "ae3_scalar_leaves.h"
#include "ae3_control_leaves.h"
#include "ae3_loop_leaves.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void) {
    AE3_ScalarContext ctx = {0};
    size_t count=ae3_scalar_leaf_count()+ae3_control_leaf_count()+ae3_loop_leaf_count();
    assert(ae3_native_leaf_count() == count);
    assert(ae3_run_native_leaf(NULL, 0x100518) == -1);
    assert(ae3_run_native_leaf(&ctx, 0xffffffff) == -2);
    for (size_t i = 0; i < count; ++i) {
        uint32_t pc = ae3_native_leaf_address(i);
        assert(pc != 0);
        for (size_t j = i + 1; j < count; ++j)
            assert(pc != ae3_native_leaf_address(j));
    }
    assert(ae3_native_leaf_address(count) == 0);
    printf("PASS: %zu distinct native C candidate routines exposed through dispatch\n", count);
    return 0;
}