#include "ae3_guest_kernel_copy.h"
static uint64_t rr(const AE3_BootPrefix*s,unsigned reg){return reg?s->gpr[reg][0]:0;}
static void wr(AE3_BootPrefix*s,unsigned reg,uint64_t v){if(reg)s->gpr[reg][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
AE3_CopyResult ae3_guest_kernel_copy_init(AE3_GuestKernelCopy*out,
 uint32_t dst,uint32_t src,uint32_t size,uint32_t ra){
 if(!out||!ra||(ra&3u)||(dst&3u)||(src&3u)||(size&3u))return AE3_COPY_INVALID;
 *out=(AE3_GuestKernelCopy){0};out->cpu.pc=AE3_GUEST_COPY_ENTRY;
 wr(&out->cpu,4,sx(dst));wr(&out->cpu,5,sx(src));wr(&out->cpu,6,size);
 wr(&out->cpu,31,ra);out->return_pc=ra;return AE3_COPY_YIELDED;
}
AE3_CopyResult ae3_guest_kernel_copy_slice(AE3_GuestKernelCopy*s,
 AE3_EEMemoryBus*b,uint32_t budget){
 if(!s||!b||!b->ram||s->cpu.in_delay_slot>1)return AE3_COPY_INVALID;
 if(s->done)return s->cpu.pc==s->return_pc?AE3_COPY_RETURNED:AE3_COPY_INVALID;
 if(s->cpu.pc<AE3_GUEST_COPY_ENTRY||s->cpu.pc>AE3_GUEST_COPY_END|| (s->cpu.pc&3u))
   return AE3_COPY_INVALID;
 for(uint32_t i=0;i<budget;i++){
   uint32_t pc=s->cpu.pc,w=0;
   if(ae3_ee_bus_read32(b,pc,&w)!=AE3_EE_BUS_OK)return AE3_COPY_MEMORY_FAULT;
   AE3_BootPrefix t=s->cpu;
   uint32_t nextpc=pc+4;
   int delay=t.in_delay_slot;
   if(delay){nextpc=t.branch_target;t.in_delay_slot=0;t.branch_target=0;}
   uint32_t addr=0,store=0,value=0;
   switch(pc){
   case 0x4402f0:
      if(w!=0x00063082u)return AE3_COPY_UNSUPPORTED;
      wr(&t,6,(uint32_t)rr(&t,6)>>2);break;
   case 0x4402f4:
      if(w!=0x10c0000au||delay)return AE3_COPY_UNSUPPORTED;
      t.in_delay_slot=1;t.branch_target=rr(&t,6)?0x4402fcu:0x440320u;break;
   case 0x4402f8:
      if(w!=0x0000382du)return AE3_COPY_UNSUPPORTED;
      wr(&t,7,0);break;
   case 0x4402fc:case 0x44031c:
      if(w!=0)return AE3_COPY_UNSUPPORTED;
      break;
   case 0x440300:
      if(w!=0x8ca30000u)return AE3_COPY_UNSUPPORTED;
      {
        uint32_t p=(uint32_t)rr(&t,5);
        if(ae3_ee_bus_read32(b,p,&value)!=AE3_EE_BUS_OK)
            return AE3_COPY_MEMORY_FAULT;
        wr(&t,3,sx(value));
      }
      break;
   case 0x440304:
      if(w!=0x24e70001u)return AE3_COPY_UNSUPPORTED;
      wr(&t,7,sx((uint32_t)rr(&t,7)+1u));break;
   case 0x440308:
      if(w!=0x24a50004u)return AE3_COPY_UNSUPPORTED;
      wr(&t,5,sx((uint32_t)rr(&t,5)+4u));break;
   case 0x44030c:
      if(w!=0x00e6102bu)return AE3_COPY_UNSUPPORTED;
      wr(&t,2,rr(&t,7)<rr(&t,6));break;
   case 0x440310:
      if(w!=0xac830000u)return AE3_COPY_UNSUPPORTED;
      addr=(uint32_t)rr(&t,4);store=1;value=(uint32_t)rr(&t,3);break;
   case 0x440314:
      if(w!=0x24840004u)return AE3_COPY_UNSUPPORTED;
      wr(&t,4,sx((uint32_t)rr(&t,4)+4u));break;
   case 0x440318:
      if(w!=0x1440fff9u||delay)return AE3_COPY_UNSUPPORTED;
      t.in_delay_slot=1;t.branch_target=rr(&t,2)?0x440300u:0x440320u;break;
   case 0x440320:
      if(w!=0x03e00008u||delay||(uint32_t)rr(&t,31)!=s->return_pc)
          return AE3_COPY_UNSUPPORTED;
      t.in_delay_slot=1;t.branch_target=s->return_pc;break;
   case 0x440324:
      if(w!=0x0000102du||!delay)return AE3_COPY_UNSUPPORTED;
      wr(&t,2,0);break;
   default:return AE3_COPY_UNSUPPORTED;
   }
   if(nextpc!=s->return_pc &&
       (nextpc<AE3_GUEST_COPY_ENTRY||nextpc>AE3_GUEST_COPY_END||(nextpc&3u)))
       return AE3_COPY_UNSUPPORTED;
   if(t.in_delay_slot && t.branch_target!=s->return_pc &&
       (t.branch_target<AE3_GUEST_COPY_ENTRY||t.branch_target>AE3_GUEST_COPY_END))
       return AE3_COPY_UNSUPPORTED;
   if(store && ae3_ee_bus_write32(b,addr,value)!=AE3_EE_BUS_OK)
       return AE3_COPY_MEMORY_FAULT;
   t.pc=nextpc;t.retired++;s->cpu=t;
   if(nextpc==s->return_pc){s->done=1;return AE3_COPY_RETURNED;}
 }
 return AE3_COPY_YIELDED;
}