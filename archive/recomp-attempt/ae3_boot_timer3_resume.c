#include "ae3_boot_timer3_resume.h"
static uint64_t rr(const AE3_BootPrefix*s,unsigned i){return i?s->gpr[i][0]:0;}
static void wr(AE3_BootPrefix*s,unsigned i,uint64_t v){if(i)s->gpr[i][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static int code_pc(uint32_t pc){
    return (pc>=UINT32_C(0x440354)&&pc<=UINT32_C(0x440378)&&!(pc&3)) ||
           (pc>=UINT32_C(0x4403f8)&&pc<=UINT32_C(0x440408)&&!(pc&3));
}
static int checkpoint(uint32_t pc){return pc==AE3_TIMER3_NEXT_CALL||pc==AE3_TIMER3_RETURN_PC;}
AE3_TimerBootResult ae3_boot_timer3_enter(AE3_BootTimer3Resume *out,
    const AE3_BootNextInit *prior){
    if(!out||!prior||prior->cpu.pc!=UINT32_C(0x00440354)||
       !prior->waiting_mmio||prior->cpu.in_delay_slot)return AE3_TIMER_BOOT_INVALID;
    *out=(AE3_BootTimer3Resume){0};out->cpu=prior->cpu;
    return AE3_TIMER_BOOT_YIELDED;
}
AE3_TimerBootResult ae3_boot_timer3_slice(AE3_BootTimer3Resume*s,
    AE3_EEMemoryBus*b,uint32_t budget){
    if(!s||!b||!b->ram||b->ram_size<AE3_EE_RAM_BYTES||
       s->cpu.in_delay_slot>1||(!code_pc(s->cpu.pc)&&!checkpoint(s->cpu.pc)))
       return AE3_TIMER_BOOT_INVALID;
    if(s->cpu.pc==AE3_TIMER3_NEXT_CALL)return AE3_TIMER_BOOT_NEXT_CALL;
    if(s->cpu.pc==AE3_TIMER3_RETURN_PC)return AE3_TIMER_BOOT_RETURNED;
    for(uint32_t n=0;n<budget;n++){
       const uint32_t pc=s->cpu.pc;
       uint32_t w=0;
       if(ae3_ee_bus_read32(b,pc,&w)!=AE3_EE_BUS_OK)return AE3_TIMER_BOOT_UNMAPPED;
       const uint32_t op=w>>26,rs=(w>>21)&31u,rt=(w>>16)&31u,imm=w&65535u;
       const int32_t offset=(int16_t)imm;
       uint32_t nextpc=pc+4u;
       AE3_BootPrefix t=s->cpu;
       const int delay=s->cpu.in_delay_slot;
       if(delay){nextpc=t.branch_target;t.in_delay_slot=0;t.branch_target=0;}
       uint64_t value=0;uint32_t addr=0;uint8_t raw[8];
       int load=0;
       switch(pc){
       case 0x440354:
           if(w!=0x8c430000u||rr(&t,2)!=UINT32_C(0x10001810))return AE3_TIMER_BOOT_UNSUPPORTED;
           {
             uint32_t mmio=0;
             if(ae3_ee_bus_read32(b,(uint32_t)rr(&t,2),&mmio)!=AE3_EE_BUS_OK)
                return AE3_TIMER_BOOT_UNMAPPED;
             wr(&t,3,sx(mmio));
           }
           break;
       case 0x440358:
           if(w!=0x30630100u)return AE3_TIMER_BOOT_UNSUPPORTED;
           wr(&t,3,rr(&t,3)&0x100u);break;
       case 0x44035c:
           if(w!=0x14600026u||delay)return AE3_TIMER_BOOT_UNSUPPORTED;
           t.in_delay_slot=1;
           t.branch_target=rr(&t,3)?UINT32_C(0x4403f8):UINT32_C(0x440364);
           break;
       case 0x440360:case 0x4403f8:case 0x4403fc:case 0x440400:
           if(op!=55u||rs!=29u ||(imm&7u))return AE3_TIMER_BOOT_UNSUPPORTED;
           addr=(uint32_t)rr(&t,rs)+(uint32_t)offset;
           if((addr&7u)||ae3_ee_bus_read(b,addr,raw,8)!=AE3_EE_BUS_OK)
              return AE3_TIMER_BOOT_UNMAPPED;
           for(unsigned i=0;i<8;i++)value|=(uint64_t)raw[i]<<(i*8);
           wr(&t,rt,value);load=1;(void)load;break;
       case 0x440364:case 0x440368:case 0x44036c:
           if(pc==0x440364){if(w!=0x3c02006au)return AE3_TIMER_BOOT_UNSUPPORTED;wr(&t,2,sx(0x006a0000u));}
           if(pc==0x440368){if(w!=0x24120002u)return AE3_TIMER_BOOT_UNSUPPORTED;wr(&t,18,2);}
           if(pc==0x44036c){if(w!=0x24502de0u)return AE3_TIMER_BOOT_UNSUPPORTED;wr(&t,16,sx((uint32_t)rr(&t,2)+0x2de0u));}
           break;
       case 0x440370:case 0x440374:
           if(op!=35u|| (pc==0x440370 && w!=0x8c442de0u)||
              (pc==0x440374 && w!=0x8e050004u))return AE3_TIMER_BOOT_UNSUPPORTED;
           addr=(uint32_t)rr(&t,rs)+(uint32_t)offset;
           if((addr&3u)||ae3_ee_bus_read(b,addr,raw,4)!=AE3_EE_BUS_OK)
              return AE3_TIMER_BOOT_UNMAPPED;
           wr(&t,rt,sx((uint32_t)raw[0]|(uint32_t)raw[1]<<8|
                      (uint32_t)raw[2]<<16|(uint32_t)raw[3]<<24));break;
       case 0x440404:
           if(w!=0x03e00008u ||delay||rr(&t,31)!=AE3_TIMER3_RETURN_PC)
               return AE3_TIMER_BOOT_UNSUPPORTED;
           t.in_delay_slot=1;t.branch_target=(uint32_t)rr(&t,31);break;
       case 0x440408:
           if(w!=0x27bd0040u||!delay)return AE3_TIMER_BOOT_UNSUPPORTED;
           wr(&t,29,sx((uint32_t)rr(&t,29)+64u));break;
       default:return AE3_TIMER_BOOT_UNSUPPORTED;
       }
       if(!code_pc(nextpc)&&!checkpoint(nextpc))return AE3_TIMER_BOOT_UNSUPPORTED;
       if(t.in_delay_slot&&!code_pc(t.branch_target)&&!checkpoint(t.branch_target))
           return AE3_TIMER_BOOT_UNSUPPORTED;
       t.pc=nextpc;t.retired++;s->cpu=t;
       if(checkpoint(nextpc)){
           s->complete=1;
           return nextpc==AE3_TIMER3_NEXT_CALL?AE3_TIMER_BOOT_NEXT_CALL:AE3_TIMER_BOOT_RETURNED;
       }
    }
    return AE3_TIMER_BOOT_YIELDED;
}