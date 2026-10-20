/* Verify the original retail AE3 post-kernel-copy instruction path against
 * the original ELF, not a mocked array of opcodes. */
#include "ae3_boot_post_patch.h"
#include "ae3_ee_timer3.h"
#include "ae3_elf_image.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned char *load(const char*path,size_t*n){
 FILE*f=fopen(path,"rb");assert(f);assert(!fseek(f,0,SEEK_END));
 long length=ftell(f);assert(length>0);assert(!fseek(f,0,SEEK_SET));
 unsigned char*b=malloc((size_t)length);assert(b);
 assert(fread(b,1,(size_t)length,f)==(size_t)length);fclose(f);
 *n=(size_t)length;return b;
}
static void setup(AE3_EEMemoryBus *bus,AE3_EESyscallKernel *kernel,
 AE3_Timer3Syscalls *prior,uint8_t *ram,const uint8_t *elf,size_t elf_len){
 AE3_ElfImage image;
 assert(ae3_load_ee_elf32(elf,elf_len,ram,AE3_EE_RAM_BYTES,&image)==AE3_ELF_IMAGE_OK);
 ae3_ee_bus_init(bus,ram,AE3_EE_RAM_BYTES);
 AE3_BootPostFind post={0};post.done=1;post.cpu.pc=AE3_NEXT_INIT_JAL;
 post.cpu.gpr[29][0]=0x01fe0080;
 AE3_BootNextInit first;
 assert(ae3_boot_next_init_enter(&first,&post)==AE3_NEXT_YIELDED);
 assert(ae3_boot_next_init_slice(&first,bus,40)==AE3_NEXT_MMIO_PENDING);
 AE3_EETimer3 timer;ae3_ee_timer3_reset(&timer);
 ae3_ee_bus_attach_mmio(bus,ae3_ee_timer3_read,ae3_ee_timer3_write,&timer);
 AE3_BootTimer3Resume resumed;
 assert(ae3_boot_timer3_enter(&resumed,&first)==AE3_TIMER_BOOT_YIELDED);
 assert(ae3_boot_timer3_slice(&resumed,bus,40)==AE3_TIMER_BOOT_NEXT_CALL);
 ae3_ee_syscall_kernel_init(kernel);
 assert(ae3_timer3_syscalls_enter(prior,&resumed)==AE3_TS_YIELDED);
 assert(ae3_timer3_syscalls_slice(prior,bus,60)==AE3_TS_SET_SYSCALL);
 assert(ae3_timer3_syscalls_dispatch(prior,bus,kernel)==AE3_TS_YIELDED);
 assert(ae3_timer3_syscalls_slice(prior,bus,60)==AE3_TS_COPY_SYSCALL);
 assert(ae3_timer3_syscalls_dispatch(prior,bus,kernel)==AE3_TS_YIELDED);
 AE3_TimerSysResult r=AE3_TS_YIELDED;
 for(int i=0;i<8;i++){
    r=ae3_timer3_syscalls_slice(prior,bus,4000);
    if(r==AE3_TS_RETURNED)break;
    assert(r==AE3_TS_YIELDED);
 }
 assert(r==AE3_TS_RETURNED && prior->cpu.pc==AE3_POST_PATCH_ENTRY);
 uint32_t target=0;
 assert(!ae3_ee_syscall_lookup(kernel,0x5a,&target)&&target==AE3_GUEST_COPY_ENTRY);
 ae3_ee_bus_attach_mmio(bus,NULL,NULL,NULL);
}
static void run(AE3_BootPostPatch *s,AE3_EEMemoryBus *bus,
 AE3_EESyscallKernel *kernel,unsigned budget){
 unsigned copy_req=0,flush_req=0,set_req=0,get_req=0;
 AE3_PostPatchResult result=AE3_PP_YIELDED;
 for(unsigned i=0;i<50000;i++){
    result=ae3_boot_post_patch_slice(s,bus,budget);
    if(result==AE3_PP_RETURNED)break;
    if(result==AE3_PP_COPY_REQUEST){copy_req++;assert(s->cpu.pc==0x4402e4);
       assert(ae3_boot_post_patch_dispatch(s,bus,kernel)==AE3_PP_YIELDED);}
    else if(result==AE3_PP_FLUSH_REQUEST){flush_req++;assert(s->cpu.pc==0x435d64);
       assert(ae3_boot_post_patch_dispatch(s,bus,kernel)==AE3_PP_YIELDED);}
    else if(result==AE3_PP_REGISTER_REQUEST){set_req++;assert(s->cpu.pc==0x4402d4);
       AE3_PostPatchResult dispatch_result=ae3_boot_post_patch_dispatch(s,bus,kernel);
       if(dispatch_result!=AE3_PP_YIELDED)fprintf(stderr,"registration %u failed: result=%d number=%08x handler=%08x getcount=%u registered=%u pc=%08x ra=%08x\n",set_req,dispatch_result,(uint32_t)s->cpu.gpr[4][0],(uint32_t)s->cpu.gpr[5][0],s->getentry_count,s->registered_table_count,s->cpu.pc,(uint32_t)s->cpu.gpr[31][0]);
       assert(dispatch_result==AE3_PP_YIELDED);}
    else if(result==AE3_PP_GET_ENTRY_REQUEST){get_req++;assert(s->cpu.pc==0x44032c);
       assert(ae3_boot_post_patch_dispatch(s,bus,kernel)==AE3_PP_YIELDED);}
    else assert(result==AE3_PP_YIELDED);
 }
 assert(result==AE3_PP_RETURNED);
 assert(copy_req==1&&flush_req==2&&set_req==7&&get_req==6);
 assert(s->cpu.pc==0x43fe78&&s->done==1);
 assert(s->copy_completed&&!s->copy_active&&s->registered_5b);
 assert(!s->getentry_active&&s->getentry_count==6&&s->registered_table_count==6);
 assert(s->flush_count==2&&s->flush_modes[0]==0&&s->flush_modes[1]==2);
 assert(s->cpu.gpr[29][0]==0x01fe0080);
 static const uint32_t numbers[7]={0x5b,0xfc,0xfe,0xfd,0xff,0x12c,0x08};
 static const uint32_t handlers[7]={0x80076000u,0x80076440u,0x80076440u,
                       0x800762a0u,0x800762a0u,0x80076488u,0x800766c0u};
 for(unsigned k=0;k<7;k++){
    uint32_t target=0;
    assert(!ae3_ee_syscall_lookup(kernel,numbers[k],&target));
    assert(target==handlers[k]);
 }
 unsigned char src[AE3_POST_PATCH_COPY_LENGTH],dst[AE3_POST_PATCH_COPY_LENGTH];
 assert(ae3_ee_bus_read(bus,AE3_POST_PATCH_COPY_SOURCE,src,sizeof src)==AE3_EE_BUS_OK);
 assert(ae3_ee_bus_read(bus,AE3_POST_PATCH_COPY_DEST,dst,sizeof dst)==AE3_EE_BUS_OK);
 assert(!memcmp(src,dst,sizeof src));
}

int main(int argc,char**argv){
 assert(argc==2);
 size_t n=0;unsigned char*elf=load(argv[1],&n);
 uint8_t *ram_a=calloc(AE3_EE_RAM_BYTES,1),*ram_b=calloc(AE3_EE_RAM_BYTES,1);
 assert(ram_a&&ram_b);
 AE3_EEMemoryBus bus_a,bus_b;AE3_EESyscallKernel kernel_a,kernel_b;
 AE3_Timer3Syscalls prior_a,prior_b;
 setup(&bus_a,&kernel_a,&prior_a,ram_a,elf,n);
 setup(&bus_b,&kernel_b,&prior_b,ram_b,elf,n);
 AE3_BootPostPatch single,batched;
 assert(ae3_boot_post_patch_enter(&single,&prior_a)==AE3_PP_YIELDED);
 assert(ae3_boot_post_patch_enter(&batched,&prior_b)==AE3_PP_YIELDED);
 /* Invalid kernel dispatch must not change either CPU or kernel state. */
 AE3_BootPostPatch pristine=single;
 assert(ae3_boot_post_patch_dispatch(&single,&bus_a,&kernel_a)==AE3_PP_UNSUPPORTED);
 assert(!memcmp(&single,&pristine,sizeof(single)));
 run(&single,&bus_a,&kernel_a,1);
 run(&batched,&bus_b,&kernel_b,2000);
 assert(!memcmp(&single,&batched,sizeof single));
 assert(!memcmp(ram_a,ram_b,AE3_EE_RAM_BYTES));
 assert(!memcmp(&kernel_a,&kernel_b,sizeof kernel_a));
 /* No new kernel services are implicitly dispatched after return. */
 AE3_BootPostPatch unchanged=single;
 assert(ae3_boot_post_patch_dispatch(&single,&bus_a,&kernel_a)==AE3_PP_UNSUPPORTED);
 assert(!memcmp(&single,&unchanged,sizeof single));
 printf("PASS original game second 40-byte Copy, both FlushCache calls, six actual copied GetEntryAddress calls and seven SetSyscall registrations through return 43FE78; scalar-vs-batched guest registers, RAM and kernel state identical; retired %llu\n",
    (unsigned long long)single.cpu.retired);
 free(elf);free(ram_a);free(ram_b);
 return 0;
}