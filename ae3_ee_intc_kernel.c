#include "ae3_ee_intc_kernel.h"
void ae3_ee_intc_init(AE3_EEIntcKernel *k) {
    if(k){*k=(AE3_EEIntcKernel){0};k->next_id=1u;}
}
int32_t ae3_ee_intc_add(AE3_EEIntcKernel *k,const AE3_EEMemoryBus *bus,
    uint32_t cause,uint32_t guest_handler,uint32_t next){
    if(!k||!bus||cause>=AE3_INTC_CAUSES||!guest_handler||
       (guest_handler&3u)||next>1u||k->next_id>0x7fffffffU||!k->next_id)
        return -1;
    uint32_t first=0;
    if(ae3_ee_bus_read32(bus,guest_handler,&first)!=AE3_EE_BUS_OK)return -1;
    /* A fully zero instruction is legal (NOP), so don't infer executability
       from opcode contents; only assert the entry is mapped and aligned. */
    unsigned slot=0;
    while(slot<AE3_INTC_CAPACITY&&k->handler[slot].present)slot++;
    if(slot==AE3_INTC_CAPACITY)return -1;
    const uint32_t id=k->next_id;
    k->handler[slot]=(AE3_EEIntcHandler){id,cause,guest_handler,next,1};
    k->next_id++;k->generation++;
    (void)first;
    return (int32_t)id;
}
int ae3_ee_intc_lookup(const AE3_EEIntcKernel *k,uint32_t id,AE3_EEIntcHandler *out){
    if(!k||!out||!id)return -1;
    for(unsigned i=0;i<AE3_INTC_CAPACITY;i++){
        if(k->handler[i].present&&k->handler[i].id==id){*out=k->handler[i];return 0;}
    }
    return -1;
}
int ae3_ee_intc_remove(AE3_EEIntcKernel *k,uint32_t cause,uint32_t id){
    if(!k||cause>=AE3_INTC_CAUSES||!id)return -1;
    for(unsigned i=0;i<AE3_INTC_CAPACITY;i++){
        if(k->handler[i].present&&k->handler[i].cause==cause&&k->handler[i].id==id){
            k->handler[i].present=0;k->generation++;
            int still_registered=0;
            for(unsigned j=0;j<AE3_INTC_CAPACITY;j++){
                if(k->handler[j].present&&k->handler[j].cause==cause){
                    still_registered=1;break;
                }
            }
            if(!still_registered)k->enabled_mask&=~(UINT32_C(1)<<cause);
            return 0;
        }
    }
    return -1;
}

int32_t ae3_ee_intc_enable(AE3_EEIntcKernel *k,uint32_t cause){
    if(!k||cause>=AE3_INTC_CAUSES)return -1;
    int registered=0;
    for(unsigned i=0;i<AE3_INTC_CAPACITY;i++){
        if(k->handler[i].present && k->handler[i].cause==cause){registered=1;break;}
    }
    if(!registered)return -1;
    const uint32_t bit=UINT32_C(1)<<cause;
    if(!(k->enabled_mask&bit)){
        k->enabled_mask|=bit;
        k->generation++;
    }
    return 0; /* explicit compatibility policy, NOT verified Sony BIOS */
}
int ae3_ee_intc_is_enabled(const AE3_EEIntcKernel *k,uint32_t cause){
    if(!k||cause>=AE3_INTC_CAUSES)return 0;
    return (k->enabled_mask&(UINT32_C(1)<<cause))!=0;
}
