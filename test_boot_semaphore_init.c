#include "ae3_elf_image.h"
#include "ae3_boot_semaphore_init.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAM_BYTES (32u*1024u*1024u)

static uint32_t rd32(const uint8_t*p){return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;}
static uint64_t rd64(const uint8_t*p){return (uint64_t)rd32(p)|((uint64_t)rd32(p+4)<<32);}
static uint8_t *loadelf(const char *name,size_t *n) {
    FILE *f=fopen(name,"rb");assert(f);assert(fseek(f,0,SEEK_END)==0);
    long length=ftell(f);assert(length>0);assert(fseek(f,0,SEEK_SET)==0);
    uint8_t *buf=malloc((size_t)length);assert(buf);
    assert(fread(buf,1,(size_t)length,f)==(size_t)length);fclose(f);*n=(size_t)length;return buf;
}
static AE3_BootInitPrefix ready(uint8_t *ram) {
    AE3_BootPrefix p;ae3_boot_prefix_init(&p);
    assert(ae3_boot_prefix_slice(&p,ram,RAM_BYTES,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    AE3_BootMemoryFill fill;assert(ae3_boot_fill_enter(&fill,&p)==AE3_FILL_YIELDED);
    assert(ae3_boot_fill_slice(&fill,ram,RAM_BYTES,200000)==AE3_FILL_RETURNED);
    AE3_BootThreadSetup t;assert(ae3_boot_thread_enter(&t,&fill)==AE3_THREAD_YIELDED);
    assert(ae3_boot_thread_slice(&t,ram,RAM_BYTES,30)==AE3_THREAD_SYSCALL);
    AE3_BootKernelBridge k;assert(ae3_boot_kernel_enter(&k,&t)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_BYTES,1)==AE3_KBOOT_SETUP_THREAD);
    /* Synthetic kernel value used ONLY to test subsequent original guest code. */
    assert(ae3_boot_kernel_return_thread(&k,0x01fffff0)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_BYTES,30)==AE3_KBOOT_SETUP_HEAP);
    assert(ae3_boot_kernel_ack_heap(&k)==AE3_KBOOT_YIELDED);
    assert(ae3_boot_kernel_slice(&k,ram,RAM_BYTES,30)==AE3_KBOOT_NEXT_CALL);
    AE3_BootInitPrefix init;assert(ae3_boot_init_enter(&init,&k)==AE3_INIT_YIELDED);
    assert(ae3_boot_init_slice(&init,ram,RAM_BYTES,10)==AE3_INIT_SUBCALL);
    return init;
}
static void validate_request(const AE3_BootSemaphoreInit *s, int index) {
    assert(s->pending && s->request.syscall_number==0x40 && s->request.pc==AE3_SEMA_SYSCALL_PC);
    assert(s->request.guest_address==(index==0?0x01ffff90u:0x01ffffb0u));
    /* Guest only initializes selected descriptor fields; other words are stack scratch. */
    assert(s->request.values[1]==1);
    assert(s->request.values[2]==1);
    assert(s->request.values[5]==(index==0?0x0072b940u:0x0072b950u));
}
static int do_one(AE3_BootSemaphoreInit*s,uint8_t*ram,int sliced,int n) {
    int calls=0;
    while(1) {
        AE3_SemaBootResult r=ae3_boot_sema_slice(s,ram,RAM_BYTES,sliced?1u:100u);
        if(r==AE3_SEMA_NEXT_CALL)break;
        if(r==AE3_SEMA_CREATE_REQUEST) {
            assert(calls<2);
            validate_request(s,calls);
            assert(s->completed==calls);
            assert(ae3_boot_sema_slice(s,ram,RAM_BYTES,0)==AE3_SEMA_CREATE_REQUEST);
            assert(ae3_boot_sema_kernel_return(s,calls?12:11)==AE3_SEMA_YIELDED);
            calls++;
        } else assert(r==AE3_SEMA_YIELDED);
        assert(++n<300);
    }
    assert(calls==2 && s->done && s->completed==2 && s->cpu.pc==0x0043fe68);
    assert(!s->cpu.in_delay_slot);
    assert(s->cpu.gpr[31][0]==0x0043fe68); /* original outer JAL's link register restored */
    assert(s->cpu.gpr[29][0]==0x01ffffe0); /* entry stack restored */
    assert(rd64(ram+0x01ffffd0)==0x0043fe68); /* saved RA on inner stack */
    assert(rd32(ram+0x006a1ea8)==11);
    assert(rd32(ram+0x006a1eac)==12);
    assert(ae3_boot_sema_slice(s,ram,RAM_BYTES,100)==AE3_SEMA_NEXT_CALL);
    return calls;
}
int main(int argc,char**argv) {
    assert(argc==2);
    size_t n;uint8_t *elf=loadelf(argv[1],&n);
    uint8_t *ram=calloc(RAM_BYTES,1);assert(ram);
    AE3_ElfImage report;assert(ae3_load_ee_elf32(elf,n,ram,RAM_BYTES,&report)==AE3_ELF_IMAGE_OK);
    AE3_BootInitPrefix init=ready(ram);
    AE3_BootSemaphoreInit direct,sliced;
    assert(ae3_boot_sema_enter(&direct,&init)==AE3_SEMA_YIELDED);
    assert(ae3_boot_sema_enter(&sliced,&init)==AE3_SEMA_YIELDED);
    do_one(&direct,ram,0,0);
    do_one(&sliced,ram,1,0);
    assert(!memcmp(&direct,&sliced,sizeof direct));
    assert(direct.cpu.retired==init.cpu.retired+34);
    /* Memory fault on guest descriptor stores must leave register state intact. */
    AE3_BootSemaphoreInit probe;
    assert(ae3_boot_sema_enter(&probe,&init)==AE3_SEMA_YIELDED);
    AE3_SemaBootResult r=AE3_SEMA_YIELDED;
    for(int j=0;j<100&&probe.cpu.pc!=0x0043fc20;j++) {
        r=ae3_boot_sema_slice(&probe,ram,RAM_BYTES,1);
        assert(r==AE3_SEMA_YIELDED);
    }
    assert(probe.cpu.pc==0x0043fc20);
    AE3_BootSemaphoreInit before=probe;
    assert(ae3_boot_sema_slice(&probe,ram,0x01ffffd0,1)==AE3_SEMA_MEMORY_FAULT);
    assert(!memcmp(&probe,&before,sizeof probe));
    assert(ae3_boot_sema_slice(&probe,ram,RAM_BYTES,1)==AE3_SEMA_YIELDED);
    /* Code corruption must be rejected transactionally; no guessed opcode. */
    AE3_BootSemaphoreInit bad;
    assert(ae3_boot_sema_enter(&bad,&init)==AE3_SEMA_YIELDED);
    const uint32_t first_pc=AE3_SEMA_INIT_CALL_PC;
    uint8_t old_op[4];memcpy(old_op,ram+first_pc,4);
    ram[first_pc]=0;ram[first_pc+1]=0;ram[first_pc+2]=0;ram[first_pc+3]=0x10; /* BEQ opcode intentionally unsupported here */
    before=bad;
    assert(ae3_boot_sema_slice(&bad,ram,RAM_BYTES,1)==AE3_SEMA_UNSUPPORTED);
    assert(!memcmp(&before,&bad,sizeof bad));
    memcpy(ram+first_pc,old_op,4);
    assert(ae3_boot_sema_slice(&bad,ram,RAM_BYTES,1)==AE3_SEMA_YIELDED);
    printf("PASS real EE CreateSema wrapper x2, preserved 0x0043FC08 guest execution, return to 0x0043FE68, and transactional fault retry (test-only kernel IDs)\n");
    free(ram);free(elf);return 0;
}