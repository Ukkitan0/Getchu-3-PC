#include "ae3_guest_execution.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SIZE (6u*1024u*1024u)
static uint8_t test_ram[SIZE];
static uint8_t ref_ram[SIZE];
static void word(uint8_t *ram, uint32_t addr, uint32_t value) {
    for (unsigned i=0;i<4;i++) ram[addr+i]=(uint8_t)(value>>(8*i));
}
static uint32_t readword(uint8_t *ram, uint32_t addr) {
    uint32_t result=0;
    for (unsigned i=0;i<4;i++) result|=(uint32_t)ram[addr+i]<<(8*i);
    return result;
}
static void init_context(AE3_ScalarContext *c, uint8_t *ram, uint32_t count) {
    memset(c,0,sizeof(*c));
    c->ram=ram; c->ram_size=SIZE;
    c->gpr[4]=0x300000; c->gpr[5]=0x301000; c->gpr[6]=count;
    c->gpr[29]=0x5f0000; c->gpr[31]=0x710000;
}
static void check_stack_parity(uint32_t count, int start_faulted) {
    memset(test_ram,0,sizeof(test_ram));memset(ref_ram,0,sizeof(ref_ram));
    static const uint32_t values[]={0x12345678u,0x23571113u,0xdeadbeefu,0x87654321u};
    for (unsigned i=0;i<4;i++) {
        word(test_ram,0x301000+4*i,values[i]);
        word(ref_ram,0x301000+4*i,values[i]);
    }
    AE3_ScalarContext a,b;
    init_context(&a,test_ram,count); init_context(&b,ref_ram,count);
    const uint32_t entry=UINT32_C(0x00105AC4);
    AE3_GuestExecution exec;
    AE3_StackFrame direct;
    assert(ae3_guest_open(&exec,entry)==AE3_RESUME_DONE);
    assert(exec.kind==AE3_EXEC_STACK && ae3_guest_pc(&exec)==entry);
    ae3_native_stack_init(&direct,entry);
    if (start_faulted) {a.ram_size=b.ram_size=64;}
    unsigned delay_checkpoints=0, memory_faults=0;
    for (unsigned step=0;step<800;step++) {
        uint32_t budget=(step%7==0)?0u:((step%3)+1u);
        int got=ae3_guest_slice(&exec,&a,budget);
        int want=ae3_run_native_stack_slice(&b,&direct,budget);
        assert(got==want);
        assert(memcmp(&exec.state.stack,&direct,sizeof(direct))==0);
        assert(memcmp(a.gpr,b.gpr,sizeof(a.gpr))==0);
        assert(memcmp(test_ram,ref_ram,SIZE)==0);
        assert(ae3_guest_retired(&exec)==direct.retired);
        if(direct.depth) {
            AE3_StackActivation *top=&direct.frames[direct.depth-1];
            uint32_t pc=top->leaf_active?top->leaf.pc:top->pc;
            assert(ae3_guest_pc(&exec)==pc);
            if(top->pending) delay_checkpoints++;
        }
        if (got==AE3_RESUME_MEMORY_FAULT && start_faulted) {
            memory_faults++; a.ram_size=b.ram_size=SIZE;
            continue;
        }
        if (got==AE3_RESUME_DONE)break;
        assert(got==AE3_RESUME_YIELDED);
        assert(step<799);
    }
    assert(ae3_guest_done(&exec));
    assert(ae3_guest_pc(&exec)==0);
    assert((count==0 || delay_checkpoints>0) && (start_faulted? memory_faults>0 : 1));
    for(unsigned i=0;i<4;i++)
        assert(readword(test_ram,0x300000+i*4)==(i<count?values[i]:0));
    assert(a.gpr[31]==0x710000);
    assert(ae3_guest_slice(&exec,&a,1)==AE3_RESUME_DONE);
}
static void check_leaf_parity(void) {
    const uint32_t entry=ae3_resume_address(0);
    AE3_GuestExecution exec;
    assert(ae3_guest_open(&exec,entry)==0);
    assert(exec.kind==AE3_EXEC_LEAF);
    AE3_ResumeFrame direct;ae3_resume_init(&direct,entry);
    static uint8_t a_ram[65536], b_ram[65536];
    AE3_ScalarContext a={0},b={0};
    a.ram=a_ram;a.ram_size=sizeof(a_ram);a.gpr[29]=0x8000;
    b.ram=b_ram;b.ram_size=sizeof(b_ram);b.gpr[29]=0x8000;
    int got=AE3_RESUME_YIELDED;
    for(unsigned i=0;i<120;i++) {
        got=ae3_guest_slice(&exec,&a,i%3);
        int want=ae3_run_native_slice(&b,&direct,i%3);
        assert(got==want);
        assert(memcmp(&exec.state.leaf,&direct,sizeof(direct))==0);
        assert(memcmp(a.gpr,b.gpr,sizeof(a.gpr))==0);
        assert(memcmp(a_ram,b_ram,sizeof(a_ram))==0);
        assert(ae3_guest_retired(&exec)==direct.retired);
        if(got!=AE3_RESUME_YIELDED)break;
    }
    assert(got==AE3_RESUME_DONE||got==AE3_RESUME_MEMORY_FAULT);
}
int main(void) {
    assert(ae3_guest_open(NULL,0x100134)==AE3_RESUME_INVALID);
    AE3_GuestExecution exec;
    /* Retail ELF boot starts here, and is currently unsupported; do not
       report success or run an invented bootstrap. */
    assert(ae3_guest_open(&exec,UINT32_C(0x00100134))==AE3_RESUME_UNKNOWN);
    assert(exec.kind==AE3_EXEC_NONE && ae3_guest_pc(&exec)==0);
    assert(ae3_guest_slice(&exec,NULL,1)==AE3_RESUME_INVALID);
    AE3_ScalarContext ctx={0};
    assert(ae3_guest_slice(&exec,&ctx,1)==AE3_RESUME_UNKNOWN);
    assert(ae3_guest_entry_count()==ae3_resume_count()+ae3_native_stack_count());
    for(size_t i=0;i<ae3_guest_entry_count();i++) {
        uint32_t pc=ae3_guest_entry_at(i);
        assert(pc);
        assert(ae3_guest_open(&exec,pc)==AE3_RESUME_DONE);
        assert(exec.entry==pc && ae3_guest_pc(&exec)==pc);
    }
    assert(ae3_guest_entry_at(ae3_guest_entry_count())==0);
    check_leaf_parity();
    check_stack_parity(0,0);
    check_stack_parity(3,0);
    check_stack_parity(3,1);
    printf("PASS guest execution unified adapter: %zu entries, leaf/stack parity, real counted loop, fault retry; retail boot entry unsupported\n",ae3_guest_entry_count());
    return 0;
}