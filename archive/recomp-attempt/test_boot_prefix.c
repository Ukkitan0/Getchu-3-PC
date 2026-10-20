#include "ae3_boot_prefix.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAM_SIZE (32u * 1024u * 1024u)
#define FULL_QUAD_STORES ((AE3_BOOT_CLEAR_END - AE3_BOOT_CLEAR_START) / 16u)

static uint8_t *read_elf(const char *path, size_t *size) {
    FILE *f=fopen(path,"rb");
    assert(f);
    assert(fseek(f,0,SEEK_END)==0);
    long len=ftell(f);
    assert(len>0 && (unsigned long)len < RAM_SIZE);
    rewind(f);
    uint8_t *bytes=(uint8_t*)malloc((size_t)len);
    assert(bytes && fread(bytes,1,(size_t)len,f)==(size_t)len);
    assert(fclose(f)==0);
    *size=(size_t)len;
    return bytes;
}
static uint8_t *new_guest(const uint8_t *elf,size_t n) {
    uint8_t *ram=(uint8_t*)calloc(RAM_SIZE,1);
    assert(ram);
    AE3_ElfImage report;
    assert(ae3_load_ee_elf32(elf,n,ram,RAM_SIZE,&report)==AE3_ELF_IMAGE_OK);
    assert(report.entry==AE3_BOOT_ENTRY && report.segment_count==1);
    assert(report.copied_bytes==0x62c5d0 && report.zeroed_bss_bytes==0x46340);
    ram[AE3_BOOT_CLEAR_START-1]=0x6e;
    ram[AE3_BOOT_CLEAR_END]=0x3b;
    memset(ram+AE3_BOOT_CLEAR_START,0xa5,AE3_BOOT_CLEAR_END-AE3_BOOT_CLEAR_START);
    return ram;
}
static void seed(AE3_BootPrefix *s) {
    ae3_boot_prefix_init(s);
    for(int r=1;r<32;r++){
        s->gpr[r][0]=UINT64_C(0xfedcba9876543210) ^ (unsigned)r;
        s->gpr[r][1]=UINT64_C(0xf123456789abcdef) ^ (unsigned)r;
    }
    for(int f=0;f<32;f++)s->fpr[f]=0x3f800000;
    s->fpu_acc=0x3f800000;
    s->fcr31=UINT32_C(0xffffffff);
    for(int b=0;b<2;b++)for(int j=0;j<2;j++)s->hilo[b][j]=UINT64_C(0x123456789abcdeff);
}
static void verify(const AE3_BootPrefix *s,const uint8_t *ram) {
    assert(s->done && s->pc==AE3_BOOT_FIRST_CALL && !s->in_delay_slot);
    assert(s->branch_target==0 && s->quad_stores==FULL_QUAD_STORES);
    assert(s->byte_stores==0 && s->fcr31==0 && s->fpu_acc==0);
    assert(s->gpr[0][0]==0 && s->gpr[0][1]==0);
    for(int i=0;i<32;i++)assert(s->fpr[i]==0);
    for(int b=0;b<2;b++)for(int i=0;i<2;i++)assert(s->hilo[b][i]==0);
    assert(s->gpr[31][0]==0x1001c0);
    assert(s->gpr[26][0]==(UINT64_C(0xfedcba9876543210)^26u));
    assert(s->gpr[27][1]==(UINT64_C(0xf123456789abcdef)^27u));
    assert(s->gpr[2][0]==AE3_BOOT_CLEAR_END);
    assert(ram[AE3_BOOT_CLEAR_START-1]==0x6e && ram[AE3_BOOT_CLEAR_END]==0x3b);
    for(unsigned p=AE3_BOOT_CLEAR_START;p<AE3_BOOT_CLEAR_END;p++)assert(ram[p]==0);
}
static int same(const AE3_BootPrefix *a,const AE3_BootPrefix *b) {
    return memcmp(a,b,sizeof(*a))==0;
}
int main(int argc,char **argv) {
    assert(argc==2);
    size_t elf_size;
    uint8_t *elf=read_elf(argv[1],&elf_size);
    uint8_t *one_ram=new_guest(elf,elf_size);
    uint8_t *sliced_ram=new_guest(elf,elf_size);
    AE3_BootPrefix one,sliced;
    seed(&one);seed(&sliced);
    assert(ae3_boot_prefix_slice(&one,one_ram,RAM_SIZE,0)==AE3_BOOT_YIELDED);
    assert(one.pc==AE3_BOOT_ENTRY && one.retired==0);
    assert(ae3_boot_prefix_slice(&one,one_ram,RAM_SIZE,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    verify(&one,one_ram);
    uint32_t jal = (uint32_t)one_ram[AE3_BOOT_FIRST_CALL] |
        ((uint32_t)one_ram[AE3_BOOT_FIRST_CALL+1]<<8) |
        ((uint32_t)one_ram[AE3_BOOT_FIRST_CALL+2]<<16) |
        ((uint32_t)one_ram[AE3_BOOT_FIRST_CALL+3]<<24);
    assert((jal>>26)==3 && ((jal&0x03ffffffu)<<2)==0x001036d8);
    /* Multiple yields, including *between branch and its delay slot*. */
    int saw_pending=0, calls=0;
    while(!sliced.done) {
        AE3_BootResult r=ae3_boot_prefix_slice(&sliced,sliced_ram,RAM_SIZE,(unsigned)((calls%13)+1));
        assert(r==AE3_BOOT_YIELDED||r==AE3_BOOT_FIRST_CALL_REACHED);
        saw_pending |= sliced.in_delay_slot;
        calls++;
        assert(calls<200000);
    }
    assert(saw_pending);
    verify(&sliced,sliced_ram);
    assert(same(&one,&sliced));
    assert(memcmp(one_ram,sliced_ram,RAM_SIZE)==0);
    printf("PASS retail boot prefix: %llu instructions, %u SQ stores, %d execution slices; arrived at original JAL %08X\n",
           (unsigned long long)one.retired,one.quad_stores,calls,one.pc);

    /* Fault retry at *first actual 128-bit store*; instruction is transactional. */
    uint8_t *fault_ram=new_guest(elf,elf_size);
    AE3_BootPrefix fault;
    seed(&fault);
    int steps=0;
    while(fault.pc!=UINT32_C(0x0010028c)) {
        assert(ae3_boot_prefix_slice(&fault,fault_ram,RAM_SIZE,1)==AE3_BOOT_YIELDED);
        assert(++steps<130);
    }
    AE3_BootPrefix before=fault;
    uint8_t before_byte=fault_ram[AE3_BOOT_CLEAR_START];
    assert(ae3_boot_prefix_slice(&fault,fault_ram,AE3_BOOT_CLEAR_START+8,1)==AE3_BOOT_MEMORY_FAULT);
    assert(same(&before,&fault) && fault_ram[AE3_BOOT_CLEAR_START]==before_byte);
    assert(ae3_boot_prefix_slice(&fault,fault_ram,RAM_SIZE,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    verify(&fault,fault_ram);
    assert(same(&one,&fault));

    /* Unknown original-code change must not be accepted as a NOP. */
    uint8_t *corrupt_ram=new_guest(elf,elf_size);
    AE3_BootPrefix corrupted;seed(&corrupted);
    for(int i=0;corrupted.pc!=0x100248;i++){
        assert(i<100);
        assert(ae3_boot_prefix_slice(&corrupted,corrupt_ram,RAM_SIZE,1)==AE3_BOOT_YIELDED);
    }
    AE3_BootPrefix saved=corrupted;
    uint8_t original[4];memcpy(original,corrupt_ram+0x100248,4);
    memset(corrupt_ram+0x100248,0xff,4);
    assert(ae3_boot_prefix_slice(&corrupted,corrupt_ram,RAM_SIZE,1)==AE3_BOOT_UNSUPPORTED_INSTRUCTION);
    assert(same(&saved,&corrupted));
    memcpy(corrupt_ram+0x100248,original,4);
    assert(ae3_boot_prefix_slice(&corrupted,corrupt_ram,RAM_SIZE,400000)==AE3_BOOT_FIRST_CALL_REACHED);
    verify(&corrupted,corrupt_ram);

    free(one_ram);free(sliced_ram);free(fault_ram);free(corrupt_ram);free(elf);
    puts("PASS transactional guest memory faults, opcode rejection, full 128-bit register/FPR zeroing and delayed branch resume");
    return 0;
}