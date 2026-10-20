#include "ae3_ee_sema_kernel.h"
void ae3_ee_sema_kernel_init(AE3_EESemaKernel *kernel){
 if(kernel)*kernel=(AE3_EESemaKernel){0};
}
static AE3_EEKernelSemaphore *entry(AE3_EESemaKernel *kernel,int32_t id){
 if(!kernel||id<=0||id>(int32_t)AE3_SEMA_CAPACITY)return NULL;
 AE3_EEKernelSemaphore *s=&kernel->entries[(uint32_t)id-1];
 return s->allocated?s:NULL;
}
int32_t ae3_ee_sema_kernel_create(AE3_EESemaKernel *kernel,
                                   const AE3_GuestSemaRequest *request){
 if(!kernel||!request||request->syscall_number!=0x40u||
    !request->guest_address||request->pc!=AE3_SEMA_SYSCALL_PC)return -1;
 const uint32_t max=request->values[1],initial=request->values[2];
 if(!max||initial>max)return -1;
 for(unsigned i=0;i<AE3_SEMA_CAPACITY;i++) {
    if(!kernel->entries[i].allocated) {
       AE3_EEKernelSemaphore *s=&kernel->entries[i];
       s->max_count=max;s->count=initial;s->attr=request->values[4];
       s->option=request->values[5];s->allocated=1;
       return (int32_t)(i+1);
    }
 }
 return -1;
}
int ae3_ee_sema_kernel_signal(AE3_EESemaKernel *kernel,int32_t id){
 AE3_EEKernelSemaphore *s=entry(kernel,id);
 if(!s||s->count>=s->max_count)return -1;
 s->count++;return 0;
}
int ae3_ee_sema_kernel_delete(AE3_EESemaKernel *kernel,int32_t id){
 AE3_EEKernelSemaphore *s=entry(kernel,id);
 if(!s)return -1;
 *s=(AE3_EEKernelSemaphore){0};return 0;
}
int ae3_ee_sema_kernel_poll(AE3_EESemaKernel *kernel,int32_t id){
 AE3_EEKernelSemaphore *s=entry(kernel,id);
 if(!s||!s->count)return -1;
 s->count--;return 0;
}
int ae3_ee_sema_kernel_get(const AE3_EESemaKernel *kernel,int32_t id,
                           uint32_t *count,uint32_t *maximum){
 if(!kernel||id<=0||id>(int32_t)AE3_SEMA_CAPACITY||
    !kernel->entries[(uint32_t)id-1].allocated)return -1;
 const AE3_EEKernelSemaphore *s=&kernel->entries[(uint32_t)id-1];
 if(count)*count=s->count;
 if(maximum)*maximum=s->max_count;
 return 0;
}
AE3_SemaBootResult ae3_ee_sema_boot_dispatch(AE3_EESemaKernel *kernel,
                                             AE3_BootSemaphoreInit *boot){
 if(!kernel||!boot||!boot->pending)return AE3_SEMA_INVALID;
 /* Snapshot is validated by the original syscall interpreter; this adapter
    receives no host pointers into guest RAM and cannot mutate it directly. */
 AE3_EESemaKernel prior=*kernel;
 int32_t id=ae3_ee_sema_kernel_create(kernel,&boot->request);
 AE3_SemaBootResult result=ae3_boot_sema_kernel_return(boot,id);
 if(result!=AE3_SEMA_YIELDED)*kernel=prior;
 return result;
}