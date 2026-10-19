#include "ae3_boot_timer2_setup.h"
static uint64_t get(const AE3_BootPrefix *s,unsigned r){return r?s->gpr[r][0]:0;}
static void put(AE3_BootPrefix *s,unsigned r,uint64_t v){if(r)s->gpr[r][0]=v;}
static uint64_t sx(uint32_t x){return (uint64_t)(int64_t)(int32_t)x;}
static const struct {uint32_t pc,word;} code[]={
 {0x004405b0,0x0c110120},{0x004405b4,0x36100c80},
 {0x004405b8,0x0c110128},{0x004405bc,0x3404ffff},
 {0x004405c0,0x0c110124},{0x004405c4,0x0200202d},
 {0x00440744,0x0c110124},{0x00440748,0x34840080},
 {0x00440480,0x0080282d},{0x00440484,0x3c04b000},
 {0x00440488,0x08110104},{0x0044048c,0x34841000},
 {0x00440490,0x0080282d},{0x00440494,0x3c04b000},
 {0x00440498,0x08110104},{0x0044049c,0x34841010},
 {0x004404a0,0x0080282d},{0x004404a4,0x3c04b000},
 {0x004404a8,0x08110104},{0x004404ac,0x34841020},
 {0x00440410,0x40036000},{0x00440414,0x3c020001},
 {0x00440418,0x00621824},{0x0044041c,0x1060000b},
 {0x00440420,0x3c060001},{0x0044044c,0x40026000},
 {0x00440450,0x0000040f},{0x00440454,0x34420006},
 {0x00440458,0x38420002},{0x0044045c,0x00431025},
 {0x00440460,0x40826000},{0x00440464,0x0000040f},
 {0x00440468,0xac850000},{0x0044046c,0x409ff000},
 {0x00440470,0x0000040f},{0x00440474,0x42000018}
};
AE3_BootTimer2SetupResult ae3_boot_timer2_setup_enter(AE3_BootTimer2Setup *s,
 const AE3_BootTimer2Continue *prior){
 if(!s||!prior||!prior->entered||prior->cpu.in_delay_slot||
   (prior->cpu.pc!=AE3_T2_GUEST_COUNT_SETUP &&
    prior->cpu.pc!=AE3_T2_GUEST_MODE_SETUP)||
   (prior->cop0_status&UINT32_C(0x10000)))return AE3_BT2S_INVALID;
 *s=(AE3_BootTimer2Setup){0};s->cpu=prior->cpu;
 s->cop0_status=prior->cop0_status;s->active=1;
 return AE3_BT2S_YIELDED;
}
AE3_BootTimer2SetupResult ae3_boot_timer2_setup_recheck_enter(
 AE3_BootTimer2Setup *s,const AE3_BootPrefix *cpu,uint32_t status){
 if(!s||!cpu||cpu->pc!=UINT32_C(0x00440744)||cpu->in_delay_slot||
    (status&UINT32_C(0x10000)))return AE3_BT2S_INVALID;
 *s=(AE3_BootTimer2Setup){0};s->cpu=*cpu;s->cop0_status=status;
 s->active=1;return AE3_BT2S_YIELDED;
}
AE3_BootTimer2SetupResult ae3_boot_timer2_setup_slice(
 AE3_BootTimer2Setup *s,AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_BT2S_INVALID;
 for(uint32_t i=0;i<budget;i++){
   uint32_t pc=s->cpu.pc,word=0;
   if(pc==AE3_T2_GUEST_NEXT_ENABLE_INTC)return AE3_BT2S_ENABLE_INTC_PENDING;
   if(pc==UINT32_C(0x0044074c))return AE3_BT2S_RECHECK_NEXT_CALL;
   size_t j=0;
   for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
   if(j==sizeof(code)/sizeof(code[0]))return AE3_BT2S_UNSUPPORTED;
   if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_BT2S_MEMORY_FAULT;
   if(word!=code[j].word)return AE3_BT2S_UNSUPPORTED;
   AE3_BootTimer2Setup next=*s;
   int delay=next.cpu.in_delay_slot;
   uint32_t target=delay?next.cpu.branch_target:pc+4u;
   if(delay){next.cpu.in_delay_slot=0;next.cpu.branch_target=0;}
   switch(pc){
   case 0x004405b0:case 0x004405b8:case 0x004405c0:case 0x00440744:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,31,pc+8u);
      next.cpu.in_delay_slot=1;
      next.cpu.branch_target=(pc==0x004405b0)?0x00440480u:
         (pc==0x004405b8)?0x004404a0u:0x00440490u;
      break;
   case 0x004405b4:
      if(!delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,16,sx((uint32_t)get(&next.cpu,16)|0xc80u));break;
   case 0x004405bc:
      if(!delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,4,UINT64_C(0xffff));break;
   case 0x00440748:
      if(!delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,4,sx((uint32_t)get(&next.cpu,4)|0x80u));break;
   case 0x004405c4:
      if(!delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,4,get(&next.cpu,16));break;
   case 0x00440480:case 0x00440490:case 0x004404a0:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,5,get(&next.cpu,4));break;
   case 0x00440484:case 0x00440494:case 0x004404a4:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,4,sx(0xb0000000u));break;
   case 0x00440488:case 0x00440498:case 0x004404a8:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      next.cpu.in_delay_slot=1;next.cpu.branch_target=0x00440410u;break;
   case 0x0044048c:case 0x0044049c:case 0x004404ac:
      if(!delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,4,sx((uint32_t)get(&next.cpu,4)|
        (pc==0x0044048c?0x1000u:pc==0x0044049c?0x1010u:0x1020u)));break;
   case 0x00440410:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,3,sx(next.cop0_status));break;
   case 0x00440414:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,2,sx(0x10000u));break;
   case 0x00440418:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,3,get(&next.cpu,3)&get(&next.cpu,2));break;
   case 0x0044041c:
      if(delay||get(&next.cpu,3))return AE3_BT2S_UNSUPPORTED;
      next.cpu.in_delay_slot=1;next.cpu.branch_target=0x0044044cu;break;
   case 0x00440420:
      if(!delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,6,sx(0x10000u));break;
   case 0x0044044c:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,2,sx(next.cop0_status));break;
   case 0x00440450:case 0x00440464:case 0x00440470:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      break; /* SYNC: guest ordering barrier; no cross-thread model here. */
   case 0x00440454:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,2,sx((uint32_t)get(&next.cpu,2)|6u));break;
   case 0x00440458:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,2,sx((uint32_t)get(&next.cpu,2)^2u));break;
   case 0x0044045c:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      put(&next.cpu,2,get(&next.cpu,2)|get(&next.cpu,3));break;
   case 0x00440460:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      next.cop0_status=(uint32_t)get(&next.cpu,2);break;
   case 0x00440468:
      if(delay||(uint32_t)get(&next.cpu,4)<0xb0001000u||
          (uint32_t)get(&next.cpu,4)>0xb0001020u)
          return AE3_BT2S_UNSUPPORTED;
      if(ae3_ee_bus_write32(bus,(uint32_t)get(&next.cpu,4),
            (uint32_t)get(&next.cpu,5))!=AE3_EE_BUS_OK)
          return AE3_BT2S_MEMORY_FAULT;
      next.io_stores++;
      break;
   case 0x0044046c:
      if(delay)return AE3_BT2S_UNSUPPORTED;
      next.error_epc=(uint32_t)get(&next.cpu,31);break;
   case 0x00440474:
      if(delay||!(next.cop0_status&4u)||
         (next.error_epc!=0x004405b8u &&next.error_epc!=0x004405c0u&&
          next.error_epc!=0x004405c8u&&
          next.error_epc!=0x0044074cu))return AE3_BT2S_UNSUPPORTED;
      next.cop0_status&=~4u;target=next.error_epc;
      break;
   default:return AE3_BT2S_UNSUPPORTED;
   }
   next.cpu.pc=target;
   next.cpu.retired++;
   *s=next;
   if(target==AE3_T2_GUEST_NEXT_ENABLE_INTC)
       return AE3_BT2S_ENABLE_INTC_PENDING;
   if(target==UINT32_C(0x0044074c))return AE3_BT2S_RECHECK_NEXT_CALL;
 }
 return AE3_BT2S_YIELDED;
}
