/* Original retail Timer 2 MODE path, including true guest LW, branch
   delay-slot and KSEG1 MMIO alias tests. Execute against the user's own ELF. */
#include "ae3_boot_timer2_setup.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t *load(const char *name,size_t *sz){
 FILE *f=fopen(name,"rb");assert(f);assert(fseek(f,0,SEEK_END)==0);
 long n=ftell(f);assert(n>0&&fseek(f,0,SEEK_SET)==0);
 uint8_t *buf=malloc((size_t)n);assert(buf);
 assert(fread(buf,1,(size_t)n,f)==(size_t)n);fclose(f);*sz=(size_t)n;return buf;
}
static AE3_BootIntcCritical seed(void){
 AE3_BootIntcCritical c={0};c.enabled=1;c.cpu.pc=AE3_T2_MODE_GUEST_LW;
 c.cpu.gpr[3][0]=AE3_T2_MODE_REGISTER;c.cpu.gpr[2][0]=UINT64_C(0xffffffffffff0000);
 c.cpu.gpr[20][0]=0;c.cpu.gpr[29][0]=UINT64_C(0x01fe0000);
 c.cpu.retired=UINT64_C(323264);
 return c;
}
static void check_mode(const uint8_t *elf,size_t n,uint32_t mode,
     AE3_BootTimer2Result expected,uint32_t expected_pc){
 uint8_t *ra=calloc(1,AE3_EE_RAM_BYTES),*rb=calloc(1,AE3_EE_RAM_BYTES);
 assert(ra&&rb);AE3_ElfImage info;
 assert(ae3_load_ee_elf32(elf,n,ra,AE3_EE_RAM_BYTES,&info)==AE3_ELF_IMAGE_OK);
 memcpy(rb,ra,AE3_EE_RAM_BYTES);
 AE3_EEMemoryBus ba,bb;ae3_ee_bus_init(&ba,ra,AE3_EE_RAM_BYTES);
 ae3_ee_bus_init(&bb,rb,AE3_EE_RAM_BYTES);
 AE3_EETimer2 ta,tb;ae3_ee_timer2_reset(&ta);ae3_ee_timer2_reset(&tb);
 ae3_ee_bus_attach_mmio(&ba,ae3_ee_timer2_read,ae3_ee_timer2_write,&ta);
 ae3_ee_bus_attach_mmio(&bb,ae3_ee_timer2_read,ae3_ee_timer2_write,&tb);
 assert(ae3_ee_bus_write32(&ba,AE3_T2_MODE,mode)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_write32(&bb,AE3_T2_MODE,mode)==AE3_EE_BUS_OK);
 AE3_BootIntcCritical prior=seed();AE3_BootTimer2Continue a,b;
 assert(ae3_boot_timer2_enter(&a,&prior)==AE3_BT2_YIELDED);
 assert(ae3_boot_timer2_enter(&b,&prior)==AE3_BT2_YIELDED);
 /* Missing hardware must fail atomically before reading any made-up MODE. */
 ba.mmio_read32=NULL;AE3_BootTimer2Continue save=a;
 assert(ae3_boot_timer2_slice(&a,&ba,1)==AE3_BT2_MEMORY_FAULT);
 assert(!memcmp(&save,&a,sizeof a));
 ba.mmio_read32=ae3_ee_timer2_read;
 /* Step one vs entire bounded block, comparing actual guest register state. */
 AE3_BootTimer2Result r=AE3_BT2_YIELDED;
 for(int i=0;i<16;i++){
   r=ae3_boot_timer2_slice(&a,&ba,1);
   if(r!=AE3_BT2_YIELDED)break;
 }
 assert(r==expected);
 assert(ae3_boot_timer2_slice(&b,&bb,50)==expected);
 assert(!memcmp(&a,&b,sizeof a));
 assert(a.cpu.pc==expected_pc&&a.cpu.retired==prior.cpu.retired+8);
 assert((a.cpu.gpr[16][0]&UINT64_C(0x300))==UINT64_C(0x300));
 assert(a.cpu.gpr[4][0]==0&&a.cpu.in_delay_slot==0);
 assert(memcmp(ra,rb,AE3_EE_RAM_BYTES)==0);
 assert(!memcmp(&ta,&tb,sizeof ta));
 /* Execute the ORIGINAL JAL wrappers and common COP0 ERET register-writer.
    A MODE with CUE clear must write COUNT, COMP, MODE; CUE already set
    skips the first two writes, but still writes MODE. */
 AE3_BootTimer2Setup sa,sb;
 assert(ae3_boot_timer2_setup_enter(&sa,&a)==AE3_BT2S_YIELDED);
 assert(ae3_boot_timer2_setup_enter(&sb,&b)==AE3_BT2S_YIELDED);
 AE3_BootTimer2SetupResult sr=AE3_BT2S_YIELDED;
 unsigned mmio_faults=0;
 for(int i=0;i<512;i++){
   if(sa.cpu.pc==0x00440468u && mmio_faults==0){
      AE3_BootTimer2Setup prior_fault=sa;
      AE3_EETimer2 timer_before=ta;
      ba.mmio_write32=NULL;
      assert(ae3_boot_timer2_setup_slice(&sa,&ba,1)==AE3_BT2S_MEMORY_FAULT);
      assert(!memcmp(&sa,&prior_fault,sizeof sa));
      assert(!memcmp(&ta,&timer_before,sizeof ta));
      ba.mmio_write32=ae3_ee_timer2_write;
      mmio_faults++;
   }
   sr=ae3_boot_timer2_setup_slice(&sa,&ba,1);
   if(sr!=AE3_BT2S_YIELDED)break;
 }
 assert(mmio_faults==1&&sr==AE3_BT2S_ENABLE_INTC_PENDING);
 assert(ae3_boot_timer2_setup_slice(&sb,&bb,1000)==AE3_BT2S_ENABLE_INTC_PENDING);
 assert(!memcmp(&sa,&sb,sizeof sa)&&!memcmp(&ta,&tb,sizeof ta));
 assert(memcmp(ra,rb,AE3_EE_RAM_BYTES)==0);
 assert(sa.cpu.pc==AE3_T2_GUEST_NEXT_ENABLE_INTC&&sa.cop0_status==0);
 assert(sa.cpu.in_delay_slot==0&&sa.io_stores==((mode&0x80u)?1u:3u));
 assert(ta.mode&0x80u);
 if(!(mode&0x80u)){
    assert(ta.count==0&&ta.compare==UINT16_C(0xffff));
 }else assert(ta.compare==0);
 printf("PASS retail Timer2 MODE=%04X: original setup %u register writes, %llu guest instructions, next EnableIntc %08X\n",
  mode,sa.io_stores,(unsigned long long)(sa.cpu.retired-a.cpu.retired),sa.cpu.pc);
 /* Any modification of real guest code aborts before committing registers. */
 assert(ae3_ee_bus_write32(&ba,0x00440590,0)==AE3_EE_BUS_OK);
 AE3_BootTimer2Continue bad;
 assert(ae3_boot_timer2_enter(&bad,&prior)==AE3_BT2_YIELDED);
 save=bad;assert(ae3_boot_timer2_slice(&bad,&ba,1)==AE3_BT2_UNSUPPORTED);
 assert(!memcmp(&save,&bad,sizeof bad));
 free(ra);free(rb);
}
int main(int argc,char **argv){
 assert(argc==2);size_t n=0;uint8_t *elf=load(argv[1],&n);
 /* All three aliases must access ONE timer register bank. */
 AE3_EETimer2 t;ae3_ee_timer2_reset(&t);
 AE3_EEMemoryBus b;uint8_t ram[128]={0};ae3_ee_bus_init(&b,ram,sizeof ram);
 uint32_t x=0xbada55;
 assert(ae3_ee_bus_read32(&b,0xb0001010,&x)==AE3_EE_BUS_UNMAPPED&&x==0xbada55);
 ae3_ee_bus_attach_mmio(&b,ae3_ee_timer2_read,ae3_ee_timer2_write,&t);
 assert(ae3_ee_bus_write32(&b,AE3_T2_COUNT,123)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read32(&b,0xb0001000,&x)==AE3_EE_BUS_OK&&x==123);
 assert(ae3_ee_bus_write32(&b,0xb0001020,300)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read32(&b,AE3_T2_COMP,&x)==AE3_EE_BUS_OK&&x==300);
 assert(ae3_ee_bus_write32(&b,0xb2001010,0)==AE3_EE_BUS_UNMAPPED);
 assert(ae3_ee_bus_write32(&b,0xb0001002,0)==AE3_EE_BUS_MISALIGNED);
 assert(ae3_ee_bus_write32(&b,AE3_T2_MODE,0x0380)==AE3_EE_BUS_OK);
 assert(ae3_ee_timer2_advance(&t,4)==AE3_EE_BUS_OK&&t.count==125);
 assert(ae3_ee_bus_write32(&b,AE3_T2_MODE,0x0004|0x0080)==AE3_EE_BUS_OK);
 AE3_EETimer2 snapshot=t;
 assert(ae3_ee_timer2_advance(&t,100)==AE3_EE_BUS_UNMAPPED);
 assert(!memcmp(&snapshot,&t,sizeof t));
 check_mode(elf,n,0,AE3_BT2_TIMER_SETUP_CALL,AE3_T2_GUEST_COUNT_SETUP);
 check_mode(elf,n,0x80u,AE3_BT2_MODE_SETUP_CALL,AE3_T2_GUEST_MODE_SETUP);
 check_mode(elf,n,0x0003u,AE3_BT2_TIMER_SETUP_CALL,AE3_T2_GUEST_COUNT_SETUP);
 free(elf);
 puts("PASS: original Timer2 MODE LW, native time/status bus, KSEG1 aliases, 3 retail branch cases, single-step parity, atomic faults");
 return 0;
}