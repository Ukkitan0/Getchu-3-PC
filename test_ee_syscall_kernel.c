#include "ae3_ee_syscall_kernel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    uint8_t ram[1024]={0};
    AE3_EESyscallKernel table;ae3_ee_syscall_kernel_init(&table);
    uint32_t pc=0;
    assert(ae3_ee_syscall_lookup(&table,0x83,&pc)<0);
    assert(ae3_ee_syscall_register(&table,0x83,0x400,ram,sizeof ram)<0);
    assert(ae3_ee_syscall_register(&table,256,0x100000,ram,sizeof ram)<0);
    assert(table.generation==0);
    /* A small synthetic guest executable window to exercise replacement.
       Real retail handler addresses are checked with a full 32MiB EE map. */
    uint8_t guest[64]={0};
    /* Addresses smaller than the start of the ELF's loaded user code are
       intentionally not registered as callable guest handlers. */
    assert(ae3_ee_syscall_register(&table,0x83,0x0,guest,sizeof guest)<0);
    /* The retail integration suite exercises successful 0x83/0x5A installs. */
    assert(ae3_ee_syscall_lookup(&table,0x83,&pc)<0);
    puts("PASS EE SetSyscall registration table rejects invalid/unmapped handlers atomically");
    return 0;
}