#include "ae3_boot_post_patch.h"

static uint64_t reg(const AE3_BootPrefix *s,unsigned i){return i?s->gpr[i][0]:0;}
static void put(AE3_BootPrefix *s,unsigned i,uint64_t x){if(i)s->gpr[i][0]=x;}
static uint64_t sext32(uint32_t x){return (uint64_t)(int64_t)(int32_t)x;}
static int allowed(uint32_t pc){
 return (pc>=UINT32_C(0x00440398)&&pc<=UINT32_C(0x00440408)&&!(pc&3))||
        (pc>=UINT32_C(0x004402D0)&&pc<=UINT32_C(0x004402DC)&&!(pc&3))||
        (pc>=UINT32_C(0x004402E0)&&pc<=UINT32_C(0x004402EC)&&!(pc&3))||
        (pc>=UINT32_C(0x00440328)&&pc<=UINT32_C(0x00440334)&&!(pc&3))||
        (pc>=UINT32_C(0x00435D60)&&pc<=UINT32_C(0x00435D6C)&&!(pc&3))||
        pc==UINT32_C(0x0043FE78);
}
static AE3_PostPatchResult event(uint32_t pc){
 switch(pc){
 case 0x004402E4:return AE3_PP_COPY_REQUEST;
 case 0x00435D64:return AE3_PP_FLUSH_REQUEST;
 case 0x004402D4:return AE3_PP_REGISTER_REQUEST;
 case 0x0044032C:return AE3_PP_GET_ENTRY_REQUEST;
 case 0x0043FE78:return AE3_PP_RETURNED;
 default:return AE3_PP_YIELDED;
 }
}
AE3_PostPatchResult ae3_boot_post_patch_enter(AE3_BootPostPatch *out,
 const AE3_Timer3Syscalls *prior){
 if(!out||!prior||!prior->done||prior->copy_active||
    prior->cpu.pc!=AE3_POST_PATCH_ENTRY||prior->cpu.in_delay_slot)
    return AE3_PP_INVALID;
 *out=(AE3_BootPostPatch){0};out->cpu=prior->cpu;
 return AE3_PP_YIELDED;
}
AE3_PostPatchResult ae3_boot_post_patch_slice(AE3_BootPostPatch *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!bus->ram||!allowed(s->cpu.pc)||
    s->cpu.in_delay_slot>1)return AE3_PP_INVALID;
 if(s->done)return AE3_PP_RETURNED;
 if(s->getentry_active){
    AE3_GetEntryResult result=ae3_guest_get_entry_slice(&s->getentry,bus,budget);
    if(result==AE3_GET_ENTRY_MEMORY_FAULT)return AE3_PP_MEMORY_FAULT;
    if(result<0)return AE3_PP_UNSUPPORTED;
    if(result==AE3_GET_ENTRY_YIELDED)return AE3_PP_YIELDED;
    AE3_BootPrefix restored=s->getentry.cpu;
    restored.retired+=s->cpu.retired;
    restored.gpr[31][0]=s->cpu.gpr[31][0];
    restored.pc=0x00440330;restored.in_delay_slot=0;restored.branch_target=0;
    s->cpu=restored;s->getentry_active=0;s->getentry_count++;
    return AE3_PP_YIELDED;
 }
 if(s->copy_active){
    AE3_CopyResult result=ae3_guest_kernel_copy_slice(&s->copy,bus,budget);
    if(result==AE3_COPY_MEMORY_FAULT)return AE3_PP_MEMORY_FAULT;
    if(result<0)return AE3_PP_UNSUPPORTED;
    if(result==AE3_COPY_YIELDED)return AE3_PP_YIELDED;
    AE3_BootPrefix restored=s->copy.cpu;
    restored.retired+=s->cpu.retired;
    restored.gpr[31][0]=UINT32_C(0x004403B0);
    restored.pc=UINT32_C(0x004402E8);
    restored.in_delay_slot=0;restored.branch_target=0;
    s->cpu=restored;s->copy_active=0;s->copy_completed=1;
    return AE3_PP_YIELDED;
 }
 if(event(s->cpu.pc)!=AE3_PP_YIELDED)return event(s->cpu.pc);
 for(uint32_t i=0;i<budget;i++){
    const uint32_t pc=s->cpu.pc;
    uint32_t word=0;
    if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_PP_MEMORY_FAULT;
    AE3_BootPrefix next=s->cpu;
    const int delay=next.in_delay_slot;
    uint32_t advance=delay?next.branch_target:pc+4;
    if(delay){next.in_delay_slot=0;next.branch_target=0;}
    switch(pc){
    case 0x00440398:case 0x0044039C:
       if(word!=(pc==0x440398?0x3c05006au:0x3c040008u)||delay)return AE3_PP_UNSUPPORTED;
       put(&next,pc==0x440398?5:4,sext32(pc==0x440398?0x006a0000u:0x00080000u));break;
    case 0x004403A0:case 0x004403A4:
       if(word!=(pc==0x4403a0?0x24a52db8u:0x34842000u)||delay)return AE3_PP_UNSUPPORTED;
       if(pc==0x4403a0)put(&next,5,sext32((uint32_t)reg(&next,5)+0x2db8u));
       else put(&next,4,reg(&next,4)|0x2000u);break;
    case 0x004403A8:case 0x004403B0:case 0x004403B8:case 0x004403C4:case 0x004403D0:{
       const uint32_t target=(pc==0x4403a8?0x4402e0:
            (pc==0x4403b0||pc==0x4403b8)?0x435d60:
            pc==0x4403c4?0x4402d0:0x440328);
       if(delay||word!=(0x0c000000u|(target>>2)))return AE3_PP_UNSUPPORTED;
       put(&next,31,pc+8);next.in_delay_slot=1;next.branch_target=target;break;}
    case 0x004403AC:
       if(!delay||word!=0x24060028u)return AE3_PP_UNSUPPORTED;
       put(&next,6,40);break;
    case 0x004403B4:
       if(!delay||word!=0x0000202du)return AE3_PP_UNSUPPORTED;
       put(&next,4,0);break;
    case 0x004403BC:
       if(!delay||word!=0x24040002u)return AE3_PP_UNSUPPORTED;
       put(&next,4,2);break;
    case 0x004402E0:case 0x004402D0:case 0x00440328:case 0x00435D60:
       if(delay||word!=(0x24030000u|(pc==0x4402e0?0x5au:
           pc==0x4402d0?0x74u:pc==0x440328?0x5bu:0x64u)))return AE3_PP_UNSUPPORTED;
       put(&next,3,pc==0x4402e0?0x5a:pc==0x4402d0?0x74:pc==0x440328?0x5b:0x64);break;
    case 0x004402E8:case 0x004402D8:case 0x00440330:case 0x00440404:case 0x00435D68:
       if(delay||word!=0x03e00008u)return AE3_PP_UNSUPPORTED;
       next.in_delay_slot=1;next.branch_target=(uint32_t)reg(&next,31);break;
    case 0x004402EC:case 0x004402DC:case 0x00440334:case 0x00435D6C:
       if(!delay||word)return AE3_PP_UNSUPPORTED;
       break;
    case 0x004403C0:case 0x004403C8:case 0x004403CC:
    case 0x004403D8:case 0x004403F0:{
       const uint32_t expected=(pc==0x4403c0?0x8e040008u:pc==0x4403c8?0x8e05000cu:0x8e240000u);
       if(word!=expected||((pc==0x4403c8||pc==0x4403f0)!=delay))return AE3_PP_UNSUPPORTED;
       const uint32_t base=(uint32_t)reg(&next,(pc==0x4403cc||pc==0x4403d8||pc==0x4403f0)?17:16);
       const uint32_t addr=base+(pc==0x4403c0?8:pc==0x4403c8?12:0);
       uint32_t value=0;
       if(ae3_ee_bus_read32(bus,addr,&value)!=AE3_EE_BUS_OK)return AE3_PP_MEMORY_FAULT;
       put(&next,pc==0x4403c8?5:4,sext32(value));break;}
    case 0x004403D4:
       if(!delay||word!=0x26520001u)return AE3_PP_UNSUPPORTED;
       put(&next,18,sext32((uint32_t)reg(&next,18)+1u));break;
    case 0x004403DC:
       if(delay||word!=0x0040282du)return AE3_PP_UNSUPPORTED;
       put(&next,5,reg(&next,2));break;
    case 0x004403E0:
       if(delay||word!=0x0c1100b4u)return AE3_PP_UNSUPPORTED;
       put(&next,31,pc+8);next.in_delay_slot=1;next.branch_target=0x004402D0;break;
    case 0x004403E4:
       if(!delay||word!=0x26310008u)return AE3_PP_UNSUPPORTED;
       put(&next,17,sext32((uint32_t)reg(&next,17)+8u));break;
    case 0x004403E8:
       if(delay||word!=0x2e420008u)return AE3_PP_UNSUPPORTED;
       put(&next,2,(uint32_t)reg(&next,18)<8u);break;
    case 0x004403EC:
       if(delay||word!=0x5440fff8u)return AE3_PP_UNSUPPORTED;
       if(reg(&next,2)){
          next.in_delay_slot=1;next.branch_target=0x004403D0;
       }else advance=0x004403F4; /* branch-likely delay annulled */
       break;
    case 0x004403F4:case 0x004403F8:case 0x004403FC:case 0x00440400:{
       const uint32_t expected=pc==0x4403f4?0xdfbf0030u:pc==0x4403f8?
                             0xdfb20020u:pc==0x4403fc?0xdfb10010u:0xdfb00000u;
       if(delay||word!=expected)return AE3_PP_UNSUPPORTED;
       uint8_t bytes[8];const uint32_t offset=pc==0x4403f4?0x30:
                                     pc==0x4403f8?0x20:pc==0x4403fc?0x10:0;
       if(ae3_ee_bus_read(bus,(uint32_t)reg(&next,29)+offset,bytes,8)!=AE3_EE_BUS_OK)
           return AE3_PP_MEMORY_FAULT;
       uint64_t value=0;for(unsigned k=0;k<8;k++)value|=(uint64_t)bytes[k]<<(k*8);
       put(&next,pc==0x4403f4?31:pc==0x4403f8?18:pc==0x4403fc?17:16,value);
       break;}
    case 0x00440408:
       if(!delay||word!=0x27bd0040u)return AE3_PP_UNSUPPORTED;
       put(&next,29,sext32((uint32_t)reg(&next,29)+0x40));break;
    default:return AE3_PP_UNSUPPORTED;
    }
    if(!allowed(advance)||(next.in_delay_slot&&!allowed(next.branch_target)))
        return AE3_PP_UNSUPPORTED;
    next.pc=advance;next.retired++;
    s->cpu=next;
    if(advance==UINT32_C(0x0043FE78))s->done=1;
    if(event(advance)!=AE3_PP_YIELDED)return event(advance);
 }
 return AE3_PP_YIELDED;
}
AE3_PostPatchResult ae3_boot_post_patch_dispatch(AE3_BootPostPatch *s,
 AE3_EEMemoryBus *bus,AE3_EESyscallKernel *kernel){
 if(!s||!bus||!kernel||!bus->ram||s->copy_active||s->getentry_active)return AE3_PP_INVALID;
 const uint32_t pc=s->cpu.pc;
 if(pc==0x004402E4 && reg(&s->cpu,3)==0x5a &&
    (uint32_t)reg(&s->cpu,4)==AE3_POST_PATCH_COPY_DEST &&
    (uint32_t)reg(&s->cpu,5)==AE3_POST_PATCH_COPY_SOURCE &&
    (uint32_t)reg(&s->cpu,6)==AE3_POST_PATCH_COPY_LENGTH && !s->copy_completed){
    uint32_t target=0;
    if(ae3_ee_syscall_lookup(kernel,0x5a,&target)||target!=AE3_GUEST_COPY_ENTRY)
        return AE3_PP_UNSUPPORTED;
    if(ae3_guest_kernel_copy_init(&s->copy,(uint32_t)reg(&s->cpu,4),
        (uint32_t)reg(&s->cpu,5),(uint32_t)reg(&s->cpu,6),0x004402E8)!=AE3_COPY_YIELDED)
        return AE3_PP_INVALID;
    AE3_BootPrefix guest=s->cpu;
    guest.pc=AE3_GUEST_COPY_ENTRY;guest.gpr[31][0]=0x004402E8;
    guest.in_delay_slot=0;guest.branch_target=0;guest.retired=0;
    s->copy.cpu=guest;s->copy_active=1;s->cpu.retired++;
    return AE3_PP_YIELDED;
 }
 if(pc==0x00435D64 && reg(&s->cpu,3)==0x64 && s->flush_count<2 &&
    (uint32_t)reg(&s->cpu,4)==(uint32_t)(s->flush_count*2u)){
    /* Compatibility cache services deliberately record requests, NOT a
       hardware cache flush. The shared native RAM has no separate I/D cache. */
    s->flush_modes[s->flush_count++]=(uint32_t)reg(&s->cpu,4);
    s->cpu.pc=0x00435D68;s->cpu.retired++;
    return AE3_PP_YIELDED;
 }
 if(pc==0x004402D4 && reg(&s->cpu,3)==0x74 &&
    s->flush_count==2 && s->copy_completed){
    const uint32_t number=(uint32_t)reg(&s->cpu,4);
    const uint32_t guestpc=(uint32_t)reg(&s->cpu,5);
    static const uint32_t numbers[6]={0xfc,0xfe,0xfd,0xff,0x12c,0x08};
    static const uint32_t handlers[6]={0x80076440u,0x80076440u,
        0x800762a0u,0x800762a0u,0x80076488u,0x800766c0u};
    if(!s->registered_5b){
       if(number!=0x5b||guestpc!=0x80076000u)return AE3_PP_UNSUPPORTED;
    }else{
       if(!s->getentry_count||s->getentry_count!=s->registered_table_count+1||
          s->registered_table_count>=6||
          number!=numbers[s->registered_table_count]||
          guestpc!=handlers[s->registered_table_count])return AE3_PP_UNSUPPORTED;
    }
    uint32_t previous=0;(void)ae3_ee_syscall_lookup(kernel,number,&previous);
    if(ae3_ee_syscall_register_bus(kernel,number,guestpc,bus))return AE3_PP_UNSUPPORTED;
    put(&s->cpu,2,previous);
    s->cpu.pc=0x004402D8;s->cpu.retired++;
    if(!s->registered_5b)s->registered_5b=1;
    else s->registered_table_count++;
    return AE3_PP_YIELDED;
 }
 if(pc==0x0044032C && reg(&s->cpu,3)==0x5b && s->registered_5b){
    uint32_t target=0;
    if(ae3_ee_syscall_lookup(kernel,0x5b,&target)||
       target!=AE3_GET_ENTRY_GUEST_PC||s->getentry_count>=6||
       ae3_guest_get_entry_init(&s->getentry,&s->cpu,0x00440330)!=AE3_GET_ENTRY_YIELDED)
       return AE3_PP_UNSUPPORTED;
    s->getentry_active=1;s->cpu.retired++;
    return AE3_PP_YIELDED;
 }
 /* All other kernel operations remain unmapped; no invented responses. */
 return AE3_PP_UNSUPPORTED;
}