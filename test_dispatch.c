#include "ae3_native_dispatch.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void) {
    AE3_ScalarContext ctx = {0};
    assert(ae3_native_leaf_count() == 1331);
    assert(ae3_run_native_leaf(NULL, 0x100518) == -1);
    assert(ae3_run_native_leaf(&ctx, 0xffffffff) == -2);
    for (size_t i = 0; i < ae3_native_leaf_count(); ++i) {
        uint32_t pc = ae3_native_leaf_address(i);
        assert(pc != 0);
        for (size_t j = i + 1; j < ae3_native_leaf_count(); ++j)
            assert(pc != ae3_native_leaf_address(j));
    }
    assert(ae3_native_leaf_address(ae3_native_leaf_count()) == 0);
    puts("PASS: 1331 native C routines exposed through one address dispatcher");
    return 0;
}
