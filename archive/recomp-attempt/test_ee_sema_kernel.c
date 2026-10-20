#include "ae3_ee_sema_kernel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 AE3_EESemaKernel k;ae3_ee_sema_kernel_init(&k);
 AE3_GuestSemaRequest q={0};q.pc=AE3_SEMA_SYSCALL_PC;
 q.guest_address=0x1000;q.syscall_number=0x40;
 q.values[1]=1;q.values[2]=1;q.values[5]=0x0072b940;
 assert(ae3_ee_sema_kernel_create(&k,&q)==1);
 q.values[5]=0x0072b950;
 assert(ae3_ee_sema_kernel_create(&k,&q)==2);
 uint32_t cnt=0,max=0;
 assert(!ae3_ee_sema_kernel_get(&k,1,&cnt,&max)&&cnt==1&&max==1);
 assert(ae3_ee_sema_kernel_signal(&k,1)==-1);
 assert(ae3_ee_sema_kernel_poll(&k,1)==0);
 assert(ae3_ee_sema_kernel_poll(&k,1)==-1);
 assert(ae3_ee_sema_kernel_signal(&k,1)==0);
 assert(ae3_ee_sema_kernel_get(&k,2,&cnt,&max)==0&&cnt==1);
 assert(ae3_ee_sema_kernel_get(&k,300,&cnt,&max)==-1);
 AE3_EESemaKernel before=k;
 q.values[2]=2;
 assert(ae3_ee_sema_kernel_create(&k,&q)==-1);
 assert(!memcmp(&k,&before,sizeof k));
 q.values[2]=0;
 for(int i=3;i<=256;i++)assert(ae3_ee_sema_kernel_create(&k,&q)==i);
 before=k;
 assert(ae3_ee_sema_kernel_create(&k,&q)==-1);
 assert(!memcmp(&k,&before,sizeof k));
 printf("PASS opt-in EE semaphore compatibility model: 256 IDs, create validation, count, poll, signal, and capacity\n");
 return 0;
}