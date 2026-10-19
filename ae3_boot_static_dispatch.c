#include "ae3_boot_static_dispatch.h"
#include <stddef.h>

static const struct { uint32_t pc, op; } retail[] = {
 {UINT32_C(0x0010033c),UINT32_C(0x0200f809)},
 {UINT32_C(0x00100340),UINT32_C(0x00000000)},
 {UINT32_C(0x00100344),UINT32_C(0x3c040063)},
 {UINT32_C(0x00100348),UINT32_C(0x24848234)},
 {UINT32_C(0x0010034c),UINT32_C(0x10800003)},
 {UINT32_C(0x00100350),UINT32_C(0x00000000)},
 {UINT32_C(0x00100354),UINT32_C(0x0c10a2e0)},
 {UINT32_C(0x00628210),UINT32_C(0x67bdffd0)},
 {UINT32_C(0x00628214),UINT32_C(0xffbf0028)},
 {UINT32_C(0x00628218),UINT32_C(0x0c040028)},
 {UINT32_C(0x0062821c),UINT32_C(0x00000000)},
 {UINT32_C(0x00628220),UINT32_C(0x0c10a2ae)},
 {UINT32_C(0x00628224),UINT32_C(0x00000000)},
 {UINT32_C(0x00628228),UINT32_C(0xdfbf0028)},
 {UINT32_C(0x0062822c),UINT32_C(0x03e00008)},
 {UINT32_C(0x00628230),UINT32_C(0x67bd0030)},
 {UINT32_C(0x001000a0),UINT32_C(0x27bdfff0)},
 {UINT32_C(0x001000a4),UINT32_C(0x3c0f0042)},
 {UINT32_C(0x001000a8),UINT32_C(0xffbf0000)},
 {UINT32_C(0x001000ac),UINT32_C(0x25ef70f0)},
 {UINT32_C(0x001000b0),UINT32_C(0x11e00005)},
 {UINT32_C(0x001000b4),UINT32_C(0x3c04006b)},
 {UINT32_C(0x001000b8),UINT32_C(0x3c050073)},
 {UINT32_C(0x001000bc),UINT32_C(0x24847f90)},
 {UINT32_C(0x001000c0),UINT32_C(0x0c109c3c)},
 {UINT32_C(0x001000c4),UINT32_C(0x24a5d688)},
 {UINT32_C(0x001000c8),UINT32_C(0x3c0f006b)},
 {UINT32_C(0x001000cc),UINT32_C(0x8dee7f90)},
 {UINT32_C(0x001000d0),UINT32_C(0x11c00007)},
 {UINT32_C(0x001000d4),UINT32_C(0x25e47f90)},
 {UINT32_C(0x001000f0),UINT32_C(0xdfbf0000)},
 {UINT32_C(0x001000f4),UINT32_C(0x03e00008)},
 {UINT32_C(0x001000f8),UINT32_C(0x27bd0010)},
 {UINT32_C(0x004270f0),UINT32_C(0x27bdfff0)},
 {UINT32_C(0x004270f4),UINT32_C(0x0000302d)},
 {UINT32_C(0x004270f8),UINT32_C(0xffbf0000)},
 {UINT32_C(0x004270fc),UINT32_C(0x0000382d)},
 {UINT32_C(0x00427100),UINT32_C(0xdfbf0000)},
 {UINT32_C(0x00427104),UINT32_C(0x08109c1e)},
 {UINT32_C(0x00427108),UINT32_C(0x27bd0010)},
 {UINT32_C(0x00427078),UINT32_C(0x27bdfff0)},
 {UINT32_C(0x0042707c),UINT32_C(0x8c8f0000)},
 {UINT32_C(0x00427080),UINT32_C(0xffb00000)},
 {UINT32_C(0x00427084),UINT32_C(0xffbf0008)},
 {UINT32_C(0x00427088),UINT32_C(0x11e00014)},
 {UINT32_C(0x0042708c),UINT32_C(0x00a0802d)},
 {UINT32_C(0x004270dc),UINT32_C(0xdfb00000)},
 {UINT32_C(0x004270e0),UINT32_C(0xdfbf0008)},
 {UINT32_C(0x004270e4),UINT32_C(0x03e00008)},
 {UINT32_C(0x004270e8),UINT32_C(0x27bd0010)},
 {UINT32_C(0x00428ab8),UINT32_C(0x27bdffe0)},
 {UINT32_C(0x00428abc),UINT32_C(0x3c0f0063)},
 {UINT32_C(0x00428ac0),UINT32_C(0xffb00000)},
 {UINT32_C(0x00428ac4),UINT32_C(0x25efe8a0)},
 {UINT32_C(0x00428ac8),UINT32_C(0xffb10008)},
 {UINT32_C(0x00428acc),UINT32_C(0x240effff)},
 {UINT32_C(0x00428ad0),UINT32_C(0xffbf0010)},
 {UINT32_C(0x00428ad4),UINT32_C(0x8dedfffc)},
 {UINT32_C(0x00428ad8),UINT32_C(0x11ae0008)},
 {UINT32_C(0x00428adc),UINT32_C(0x25f0fffc)},
 {UINT32_C(0x00428ae0),UINT32_C(0x01a0702d)},
 {UINT32_C(0x00428ae4),UINT32_C(0x2411ffff)},
 {UINT32_C(0x00428ae8),UINT32_C(0x01c0f809)},
 {UINT32_C(0x00428afc),UINT32_C(0xdfb00000)},
 {UINT32_C(0x00428b00),UINT32_C(0xdfb10008)},
 {UINT32_C(0x00428b04),UINT32_C(0xdfbf0010)},
 {UINT32_C(0x00428b08),UINT32_C(0x03e00008)},
 {UINT32_C(0x00428b0c),UINT32_C(0x27bd0020)},
};
static uint64_t r(const AE3_BootPrefix *cpu,unsigned id){
    return id?cpu->gpr[id][0]:0;
}
static void w(AE3_BootPrefix *cpu,unsigned id,uint64_t v){
    if(id)cpu->gpr[id][0]=v;
}
static uint64_t sext32(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static uint32_t jtarget(uint32_t pc,uint32_t op){
    return ((pc+4u)&UINT32_C(0xf0000000))|((op&UINT32_C(0x03ffffff))<<2);
}
static AE3_EEBusStatus load(const AE3_EEMemoryBus *bus,
    uint32_t a,unsigned n,uint64_t *out){
    uint8_t raw[8]={0};
    if(a&(n-1u))return AE3_EE_BUS_MISALIGNED;
    AE3_EEBusStatus q=ae3_ee_bus_read(bus,a,raw,n);
    if(q!=AE3_EE_BUS_OK)return q;
    uint64_t v=0;
    for(unsigned i=0;i<n;i++)v|=(uint64_t)raw[i]<<(i*8u);
    *out=n==4?sext32((uint32_t)v):v;
    return AE3_EE_BUS_OK;
}
static AE3_EEBusStatus store(AE3_EEMemoryBus *bus,
    uint32_t a,unsigned n,uint64_t value){
    uint8_t raw[8];
    if(a&(n-1u))return AE3_EE_BUS_MISALIGNED;
    for(unsigned i=0;i<n;i++)raw[i]=(uint8_t)(value>>(i*8u));
    return ae3_ee_bus_write(bus,a,raw,n);
}
AE3_StaticInitResult ae3_boot_static_enter(AE3_BootStaticDispatch *out,
  const AE3_BootMainResume *prior){
    if(!out||!prior||!prior->active||prior->flush_requests!=1||
      prior->cpu.pc!=AE3_STATIC_INIT_ENTRY||prior->cpu.in_delay_slot||
      (uint32_t)r(&prior->cpu,16)!=UINT32_C(0x00628210))
      return AE3_STATIC_INVALID;
    *out=(AE3_BootStaticDispatch){0};
    out->active=1;out->cpu=prior->cpu;
    return AE3_STATIC_YIELDED;
}
AE3_StaticInitResult ae3_boot_static_slice(AE3_BootStaticDispatch *state,
 AE3_EEMemoryBus *bus,uint32_t budget){
    if(!state||!state->active||!bus||state->cpu.in_delay_slot>1)
      return AE3_STATIC_INVALID;
    for(uint32_t i=0;i<budget;i++){
      const uint32_t pc=state->cpu.pc;
      if(pc==AE3_STATIC_NEXT_JAL&&!state->cpu.in_delay_slot)
        return state->empty_ctor_path&&state->registered_zero_path&&
            r(&state->cpu,4)==UINT32_C(0x00628234)?
            AE3_STATIC_NEXT_CALL:AE3_STATIC_UNSUPPORTED;
      if(pc==UINT32_C(0x00428ae8) && !state->cpu.in_delay_slot){
        const uint32_t target=(uint32_t)r(&state->cpu,14);
        uint32_t current_op=0,callee_op=0;
        if(ae3_ee_bus_read32(bus,pc,&current_op)!=AE3_EE_BUS_OK)
          return AE3_STATIC_MEMORY_FAULT;
        if(current_op!=UINT32_C(0x01c0f809)||
          state->guest_ctor_word!=target||
          target<UINT32_C(0x00100000)||target>=UINT32_C(0x02000000)||
          (target&3u))return AE3_STATIC_UNSUPPORTED;
        if(ae3_ee_bus_read32(bus,target,&callee_op)!=AE3_EE_BUS_OK)
          return AE3_STATIC_MEMORY_FAULT;
        (void)callee_op;
        return AE3_STATIC_CONSTRUCTOR_PENDING;
      }
      if(pc==UINT32_C(0x00427090) && !state->cpu.in_delay_slot)
        return AE3_STATIC_DYNAMIC_REGISTRATION_PENDING;
      if(pc>=UINT32_C(0x001000d8)&&pc<=UINT32_C(0x001000ec))
        return AE3_STATIC_DYNAMIC_REGISTRATION_PENDING;
      size_t index=0;
      for(;index<sizeof(retail)/sizeof(retail[0]);index++)
        if(retail[index].pc==pc)break;
      if(index==sizeof(retail)/sizeof(retail[0]))return AE3_STATIC_UNSUPPORTED;
      uint32_t op=0;
      if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)
         return AE3_STATIC_MEMORY_FAULT;
      if(op!=retail[index].op)return AE3_STATIC_UNSUPPORTED;
      AE3_BootStaticDispatch next=*state;
      unsigned in_delay=next.cpu.in_delay_slot;
      uint32_t target=in_delay?next.cpu.branch_target:pc+4u;
      if(in_delay){next.cpu.in_delay_slot=0;next.cpu.branch_target=0;}
      unsigned primary=op>>26,rs=(op>>21)&31u,rt=(op>>16)&31u;
      unsigned rd=(op>>11)&31u,fn=op&63u;
      uint32_t imm=op&UINT32_C(0xffff);
      int32_t signed_imm=(int16_t)imm;
      uint32_t addr=0;
      uint64_t val=0;
      unsigned nbytes=0,do_store=0,do_load=0;
      if(primary==0){
         if(op==0) { /* MIPS NOP */ }
         else if(fn==0x08 && rt==0 && rd==0 && !in_delay){
             next.cpu.in_delay_slot=1;
             next.cpu.branch_target=(uint32_t)r(&next.cpu,rs);
         }else if(fn==0x09 && rt==0 && !in_delay){
             next.cpu.in_delay_slot=1;
             next.cpu.branch_target=(uint32_t)r(&next.cpu,rs);
             w(&next.cpu,rd,pc+8u);
         }else if(fn==0x2d && (op&UINT32_C(0x7c0))==0){
             if(in_delay && pc!=UINT32_C(0x0042708c)&&
                 pc!=UINT32_C(0x00428adc))return AE3_STATIC_UNSUPPORTED;
             w(&next.cpu,rd,r(&next.cpu,rs)+r(&next.cpu,rt));
         }else return AE3_STATIC_UNSUPPORTED;
      }else if(primary==0x09){
          w(&next.cpu,rt,sext32((uint32_t)r(&next.cpu,rs)+(uint32_t)signed_imm));
      }else if(primary==0x19){
          w(&next.cpu,rt,r(&next.cpu,rs)+(uint64_t)(int64_t)signed_imm);
      }else if(primary==0x0f){
          if(rs)return AE3_STATIC_UNSUPPORTED;
          w(&next.cpu,rt,sext32(imm<<16));
      }else if(primary==0x04||primary==0x05){
          if(in_delay)return AE3_STATIC_UNSUPPORTED;
          int taken=(primary==4)?r(&next.cpu,rs)==r(&next.cpu,rt):
                                 r(&next.cpu,rs)!=r(&next.cpu,rt);
          next.cpu.in_delay_slot=1;
          next.cpu.branch_target=taken?
              (uint32_t)((int64_t)(uint64_t)(pc+4u)+(int64_t)signed_imm*4):pc+8u;
      }else if(primary==0x02||primary==0x03){
          if(in_delay)return AE3_STATIC_UNSUPPORTED;
          next.cpu.in_delay_slot=1;next.cpu.branch_target=jtarget(pc,op);
          if(primary==3)w(&next.cpu,31,pc+8u);
      }else if(primary==0x23||primary==0x37||primary==0x3f){
          addr=(uint32_t)(r(&next.cpu,rs)+(uint64_t)(int64_t)signed_imm);
          nbytes=primary==0x23?4:8;
          do_store=primary==0x3f;
          do_load=!do_store;
          if(in_delay)return AE3_STATIC_UNSUPPORTED;
      }else return AE3_STATIC_UNSUPPORTED;
      /* The retail constructor terminator must be observed in actual RAM.
         The saved later-game snapshot contains guest callbacks; that is a pending guest
         callback, NOT permission to invent a terminator or a host function. */
      if(do_load){
          if(load(bus,addr,nbytes,&val)!=AE3_EE_BUS_OK)
             return AE3_STATIC_MEMORY_FAULT;
          w(&next.cpu,rt,val);
          if(pc==UINT32_C(0x001000cc))next.registration_word=(uint32_t)val;
          if(pc==UINT32_C(0x00428ad4))next.guest_ctor_word=(uint32_t)val;
      }
      if(do_store && store(bus,addr,nbytes,r(&next.cpu,rt))!=AE3_EE_BUS_OK)
        return AE3_STATIC_MEMORY_FAULT;
      if(pc==UINT32_C(0x00427088)&&
         next.cpu.branch_target==UINT32_C(0x004270dc))next.registered_zero_path=1;
      if(pc==UINT32_C(0x00428ad8)&&
         next.cpu.branch_target==UINT32_C(0x00428afc))next.empty_ctor_path=1;
      next.code_reads++;
      next.cpu.pc=target;next.cpu.retired++;
      *state=next;
      if(target==AE3_STATIC_NEXT_JAL&&!next.cpu.in_delay_slot)
         return next.empty_ctor_path&&next.registered_zero_path&&
              r(&next.cpu,4)==UINT32_C(0x00628234)?
                   AE3_STATIC_NEXT_CALL:AE3_STATIC_UNSUPPORTED;
    }
    return AE3_STATIC_YIELDED;
}

AE3_StaticInitResult ae3_boot_static_scan_constructors(
  AE3_BootConstructorList *out,const AE3_EEMemoryBus *bus){
    if(!out||!bus)return AE3_STATIC_INVALID;
    AE3_BootConstructorList staged={0};
    for(uint32_t i=0;i<AE3_STATIC_MAX_CONSTRUCTORS;i++){
        const uint32_t entry_address=AE3_STATIC_CONSTRUCTOR_TABLE-i*4u;
        uint32_t entry=0,first=0;
        if(ae3_ee_bus_read32(bus,entry_address,&entry)!=AE3_EE_BUS_OK)
          return AE3_STATIC_MEMORY_FAULT;
        if(entry==UINT32_C(0xffffffff)){
          *out=staged;
          return AE3_STATIC_YIELDED;
        }
        if(entry<UINT32_C(0x00100000)||entry>=UINT32_C(0x02000000)||
             (entry&3u))return AE3_STATIC_UNSUPPORTED;
        if(ae3_ee_bus_read32(bus,entry,&first)!=AE3_EE_BUS_OK)
          return AE3_STATIC_MEMORY_FAULT;
        staged.entries[staged.count++]=entry;
        (void)first; /* a NOP is a legal first instruction */
    }
    return AE3_STATIC_UNSUPPORTED;
}