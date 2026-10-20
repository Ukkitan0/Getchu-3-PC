#include "ae3_ee_timer2.h"
void ae3_ee_timer2_reset(AE3_EETimer2 *t){if(t)*t=(AE3_EETimer2){0};}
AE3_EEBusStatus ae3_ee_timer2_read(void *context,uint32_t address,uint32_t *out){
 AE3_EETimer2 *t=(AE3_EETimer2*)context;
 if(!t||!out)return AE3_EE_BUS_BAD_ARGUMENT;
 switch(address){
 case AE3_T2_COUNT:*out=t->count;return AE3_EE_BUS_OK;
 case AE3_T2_MODE:*out=t->mode;return AE3_EE_BUS_OK;
 case AE3_T2_COMP:*out=t->compare;return AE3_EE_BUS_OK;
 default:return AE3_EE_BUS_UNMAPPED;
 }
}
AE3_EEBusStatus ae3_ee_timer2_write(void *context,uint32_t address,uint32_t value){
 AE3_EETimer2 *t=(AE3_EETimer2*)context;
 if(!t)return AE3_EE_BUS_BAD_ARGUMENT;
 switch(address){
 case AE3_T2_COUNT:t->count=(uint16_t)value;return AE3_EE_BUS_OK;
 case AE3_T2_COMP:t->compare=(uint16_t)value;return AE3_EE_BUS_OK;
 case AE3_T2_MODE:
   t->mode=(uint16_t)(((t->mode&UINT16_C(0x0c00))&~(value&UINT32_C(0x0c00)))|
      (value&UINT32_C(0x03ff)));
   t->subcycles=0;return AE3_EE_BUS_OK;
 default:return AE3_EE_BUS_UNMAPPED;
 }
}
AE3_EEBusStatus ae3_ee_timer2_advance(AE3_EETimer2 *t,uint32_t ee_cycles){
 if(!t)return AE3_EE_BUS_BAD_ARGUMENT;
 if(!ee_cycles||!(t->mode&UINT16_C(0x80)))return AE3_EE_BUS_OK;
 /* Gate and external HBLANK timing require a video/GS clock source. */
 if((t->mode&UINT16_C(0x04))||(t->mode&3u)==3u)return AE3_EE_BUS_UNMAPPED;
 const uint32_t divisor=(t->mode&3u)==0?2u:(t->mode&3u)==1?32u:512u;
 uint64_t accumulated=(uint64_t)t->subcycles+ee_cycles;
 uint64_t ticks=accumulated/divisor;
 if(ticks>65536u)return AE3_EE_BUS_BAD_ARGUMENT;
 AE3_EETimer2 next=*t;
 next.subcycles=(uint16_t)(accumulated%divisor);
 while(ticks--){
   uint16_t count=(uint16_t)(next.count+1u);
   if(count==0&&(next.mode&AE3_T2_OVFE))next.mode|=AE3_T2_OVFF;
   if(count==next.compare){
     if(next.mode&AE3_T2_CMPE)next.mode|=AE3_T2_EQUF;
     if(next.mode&UINT16_C(0x40))count=0;
   }
   next.count=count;
 }
 *t=next;return AE3_EE_BUS_OK;
}