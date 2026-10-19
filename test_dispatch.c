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
    /* Real retail JAL wrapper calls a recovered leaf, including delay slots. */
    assert(ae3_native_caller_count() > 0);
    for(size_t i=0;i<ae3_native_caller_count();i++) {
        uint32_t pc=ae3_native_caller_address(i);
        assert(pc);
        for(size_t j=i+1;j<ae3_native_caller_count();j++)
            assert(pc!=ae3_native_caller_address(j));
    }
    assert(ae3_native_caller_address(ae3_native_caller_count())==0);
    static uint8_t caller_ram[8*1024*1024]={0};
    AE3_ScalarContext caller={0};
    caller.ram=caller_ram;caller.ram_size=sizeof(caller_ram);
    caller.gpr[29]=0xD000;caller.gpr[2]=0x2000;caller.gpr[31]=0x12345678;
    AE3_CallFrame cf;
    ae3_native_caller_init(&cf,UINT32_C(0x00112CC4));
    int rc=AE3_RESUME_YIELDED;
    for(unsigned i=0;i<512 && rc==AE3_RESUME_YIELDED;i++)
        rc=ae3_run_native_caller_slice(&caller,&cf,1);
    assert(rc==AE3_RESUME_DONE);
    assert(cf.done && cf.retired>0);
    assert(caller.gpr[31]==UINT32_C(0x12345678));
    /* New stack executor includes legacy callers and newly recovered JALR. */
    assert(ae3_native_stack_count()==176);
    for(size_t i=0;i<ae3_native_stack_count();++i){
        assert(ae3_native_stack_address(i));
        for(size_t j=i+1;j<ae3_native_stack_count();j++)
            assert(ae3_native_stack_address(i)!=ae3_native_stack_address(j));
    }
    assert(ae3_native_stack_address(ae3_native_stack_count())==0);
    AE3_StackFrame stacked;
    ae3_native_stack_init(&stacked,UINT32_C(0x00112CC4));
    static uint8_t stack_ram[8*1024*1024]={0};
    AE3_ScalarContext newcaller={0};
    newcaller.ram=stack_ram;newcaller.ram_size=sizeof(stack_ram);
    newcaller.gpr[29]=0xD000;newcaller.gpr[2]=0x2000;
    newcaller.gpr[31]=0x12345678;
    rc=AE3_RESUME_YIELDED;
    for(unsigned i=0;i<512&&rc==AE3_RESUME_YIELDED;i++)
        rc=ae3_run_native_stack_slice(&newcaller,&stacked,1);
    assert(rc==AE3_RESUME_DONE && stacked.done && stacked.retired>0);
    assert(memcmp(newcaller.gpr,caller.gpr,sizeof(caller.gpr))==0);
    assert(memcmp(stack_ram,caller_ram,sizeof(stack_ram))==0);
    printf("PASS: %zu native candidate routines dispatched; resumable and legacy entry agree\n", count);
    return 0;
}