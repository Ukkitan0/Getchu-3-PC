#include "ae3_native_dispatch.h"
#include "ae3_scalar_leaves.h"
#include "ae3_control_leaves.h"
#include "ae3_loop_leaves.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
    /* Check legacy one-shot dispatch against the new slice interface. */
    uint8_t ram_a[65536] = {0};
    uint8_t ram_b[65536] = {0};
    AE3_ScalarContext one = {0}, sliced = {0};
    one.ram = ram_a; one.ram_size = sizeof(ram_a);
    sliced.ram = ram_b; sliced.ram_size = sizeof(ram_b);
    one.gpr[29]=0x8000; sliced.gpr[29]=0x8000;
    const uint32_t entry=ae3_scalar_leaf_address(0);
    int legacy=ae3_run_native_leaf(&one,entry);
    AE3_ResumeFrame f = {0};
    ae3_native_frame_init(&f,entry);
    int resumed=AE3_RESUME_YIELDED;
    for(unsigned n=0;n<128 && resumed==AE3_RESUME_YIELDED;n++)
        resumed=ae3_run_native_slice(&sliced,&f,1);
    assert(legacy==resumed);
    assert(memcmp(one.gpr,sliced.gpr,sizeof(one.gpr))==0);
    assert(memcmp(ram_a,ram_b,sizeof(ram_a))==0);
    assert(f.retired>0);
    assert(ae3_resume_count()==count);
    printf("PASS: %zu native candidate routines dispatched; resumable and legacy entry agree\n", count);
    return 0;
}