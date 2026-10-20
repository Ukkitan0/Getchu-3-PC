#include "ae3_ee_timer3.h"
void ae3_ee_timer3_reset(AE3_EETimer3 *t){if(t)*t=(AE3_EETimer3){0};}
AE3_EEBusStatus ae3_ee_timer3_read(void *ctx,uint32_t addr,uint32_t *out){
    AE3_EETimer3 *t=(AE3_EETimer3*)ctx;
    if(!t||!out)return AE3_EE_BUS_BAD_ARGUMENT;
    switch(addr){
    case AE3_T3_COUNT:*out=t->count;return AE3_EE_BUS_OK;
    case AE3_T3_MODE:*out=t->mode;return AE3_EE_BUS_OK;
    case AE3_T3_COMP:*out=t->compare;return AE3_EE_BUS_OK;
    default:return AE3_EE_BUS_UNMAPPED;
    }
}
AE3_EEBusStatus ae3_ee_timer3_write(void *ctx,uint32_t addr,uint32_t value){
    AE3_EETimer3 *t=(AE3_EETimer3*)ctx;
    if(!t)return AE3_EE_BUS_BAD_ARGUMENT;
    switch(addr){
    case AE3_T3_COUNT:t->count=(uint16_t)value;return AE3_EE_BUS_OK;
    case AE3_T3_COMP:t->compare=(uint16_t)value;return AE3_EE_BUS_OK;
    case AE3_T3_MODE:
        /* Writing a 1 to either event flag clears it; other control bits are
         * replaced. PCSX2's EE counter implementation follows this behavior. */
        t->mode=(uint16_t)(((t->mode&0x0c00u)&~(value&0x0c00u))|(value&0x03ffu));
        t->subcycles=0;
        return AE3_EE_BUS_OK;
    default:return AE3_EE_BUS_UNMAPPED;
    }
}
AE3_EEBusStatus ae3_ee_timer3_advance(AE3_EETimer3 *t,uint32_t cycles){
    if(!t)return AE3_EE_BUS_BAD_ARGUMENT;
    if(!cycles || !(t->mode&0x80u))return AE3_EE_BUS_OK;
    /* Hardware gate/HBLANK require video timing, not provided by the guest CPU
       instruction runner. Make them a hard boundary rather than fake time. */
    if((t->mode&0x04u)||((t->mode&3u)==3u))return AE3_EE_BUS_UNMAPPED;
    const uint32_t divisor=(t->mode&3u)==0?2u:(t->mode&3u)==1?32u:512u;
    uint64_t total=(uint64_t)t->subcycles+cycles;
    uint32_t ticks=(uint32_t)(total/divisor);
    uint16_t residue=(uint16_t)(total%divisor);
    /* Keep fast forward bounded: at most 65,536 ticks are needed to establish
     * a periodic flag. For very large advances, use cycles in the caller. */
    if(ticks>65536u)return AE3_EE_BUS_BAD_ARGUMENT;
    AE3_EETimer3 next=*t;next.subcycles=residue;
    while(ticks--){
        uint16_t count=(uint16_t)(next.count+1u);
        if(count==0u && (next.mode&AE3_T3_OVFE))next.mode|=AE3_T3_OVFF;
        if(count==next.compare){
            if(next.mode&AE3_T3_CMPE)next.mode|=AE3_T3_EQUF;
            if(next.mode&0x40u)count=0;
        }
        next.count=count;
    }
    *t=next;return AE3_EE_BUS_OK;
}