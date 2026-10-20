#include "ae3_ee_thread_kernel.h"
void ae3_ee_thread_kernel_init(AE3_EEThreadKernel *k){
 if(k)*k=(AE3_EEThreadKernel){0};
}
static int readable(const AE3_EEMemoryBus *bus,uint32_t addr,size_t len){
 uint8_t test=0;
 if(!len||!bus||addr>UINT32_MAX-(uint32_t)(len-1))return 0;
 return ae3_ee_bus_read(bus,addr,&test,1)==AE3_EE_BUS_OK &&
        ae3_ee_bus_read(bus,addr+(uint32_t)len-1u,&test,1)==AE3_EE_BUS_OK;
}
int32_t ae3_ee_thread_kernel_create(AE3_EEThreadKernel *k,
 const AE3_EEMemoryBus *bus,uint32_t descriptor,
 const uint32_t v[AE3_EE_THREAD_DESC_WORDS]){
 if(!k||!bus||!v||!descriptor||(descriptor&3u)||
    !readable(bus,descriptor,AE3_EE_THREAD_DESC_WORDS*4u))return -1;
 const uint32_t func=v[1],stack=v[2],size=v[3],gp=v[4];
 if(!func||(func&3u)||!stack||(stack&15u)||!size||(size&15u)||
    size>AE3_EE_RAM_BYTES || (gp&3u)||v[5]>127u||
    !readable(bus,func,4)||!readable(bus,stack,size)||
    (gp && !readable(bus,gp,4)))return -1;
 for(unsigned i=0;i<AE3_EE_THREAD_CAPACITY;i++){
  if(!k->entries[i].allocated &&
     (!k->current_present || k->current_thread_id!=i+1u)){
   AE3_EEThread record={0};
   record.guest_descriptor=descriptor;
   for(unsigned j=0;j<AE3_EE_THREAD_DESC_WORDS;j++)record.fields[j]=v[j];
   record.allocated=1;k->entries[i]=record;k->generation++;
   return (int32_t)(i+1);
  }
 }
 return -1;
}
int ae3_ee_thread_kernel_lookup(const AE3_EEThreadKernel *k,
 uint32_t id,AE3_EEThread *out){
 if(!k||!out||!id||id>AE3_EE_THREAD_CAPACITY||
    !k->entries[id-1u].allocated)return -1;
 *out=k->entries[id-1u];return 0;
}

int ae3_ee_thread_kernel_bind_current(AE3_EEThreadKernel *k,
    uint32_t id,uint32_t priority){
 if(!k||k->current_present||!id||id>AE3_EE_THREAD_CAPACITY||
    priority>=128u||k->entries[id-1u].allocated)return -1;
 k->current_thread_id=id;k->current_priority=priority;
 k->current_present=1;k->generation++;
 return 0;
}
int32_t ae3_ee_thread_kernel_get_current(const AE3_EEThreadKernel *k){
 if(!k||!k->current_present)return -1;
 return (int32_t)k->current_thread_id;
}
int32_t ae3_ee_thread_kernel_change_current_priority(AE3_EEThreadKernel *k,
    uint32_t id,uint32_t priority){
 if(!k||!k->current_present||priority>=128u||
    (id!=k->current_thread_id && id!=0))return -1;
 k->current_priority=priority;k->generation++;
 return 0;
}