#include "ae3_boot_timer2_recheck.h"
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t x){return (uint64_t)(int64_t)(int32_t)x;}
static const struct {uint32_t pc,word;} original[]={
 {0x0043FE80,0x0C1101BC},{0x0043FE84,0x00000000},
 {0x004406F0,0x27BDFFE0},{0x004406F4,0xFFB00000},
 {0x004406F8,0xFFBF0010},{0x004406FC,0x0C10FEE8},
 {0x00440700,0x00000000},
 {0x0043FBA0,0x40036000},{0x0043FBA4,0x3C020001},
 {0x0043FBA8,0x00621824},{0x0043FBAC,0x1060000D},
 {0x0043FBB0,0x0003202B},
 {0x0043FBE4,0x0000202D},{0x0043FBE8,0x03E00008},
 {0x0043FBEC,0x0080102D},
 {0x00440704,0x0040802D},{0x00440708,0x3C021000},
 {0x0044070C,0x34421010}
};
AE3_T2RecheckResult ae3_boot_timer2_recheck_enter(AE3_BootTimer2Recheck *s,
 const AE3_BootEnableIntc *prior){
 if(!s||!prior||!prior->active||!prior->serviced_count||
    prior->cpu.pc!=AE3_T2_RECHECK_CALL||prior->cpu.in_delay_slot||
    (prior->cop0_status&UINT32_C(0x10000)))return AE3_T2RC_INVALID;
 *s=(AE3_BootTimer2Recheck){0};s->cpu=prior->cpu;
 s->cop0_status=prior->cop0_status;s->active=1;
 return AE3_T2RC_YIELDED;
}
AE3_T2RecheckResult ae3_boot_timer2_recheck_slice(AE3_BootTimer2Recheck *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_T2RC_INVALID;
 if(s->cpu.pc==AE3_T2_RECHECK_MMIO_LW&&!s->cpu.in_delay_slot)
     return AE3_T2RC_MMIO_PENDING;
 for(uint32_t i=0;i<budget;i++){
    const uint32_t pc=s->cpu.pc;
    unsigned j=0;
    for(;j<sizeof(original)/sizeof(original[0]);j++)if(original[j].pc==pc)break;
    if(j==sizeof(original)/sizeof(original[0]))return AE3_T2RC_UNSUPPORTED;
    uint32_t word=0;
    if(ae3_ee_bus_read32(bus,pc,&word)!=AE3_EE_BUS_OK)return AE3_T2RC_MEMORY_FAULT;
    if(word!=original[j].word)return AE3_T2RC_UNSUPPORTED;
    AE3_BootPrefix n=s->cpu;
    const int delay=n.in_delay_slot;
    uint32_t next=delay?n.branch_target:pc+4u;
    if(delay){n.in_delay_slot=0;n.branch_target=0;}
    uint32_t store=0;unsigned src=0;
    switch(pc){
    case 0x0043FE80:case 0x004406FC:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,31,pc+8u);n.in_delay_slot=1;
        n.branch_target=pc==0x0043FE80?0x004406F0u:0x0043FBA0u;
        break;
    case 0x0043FE84:case 0x00440700:
        if(!delay)return AE3_T2RC_UNSUPPORTED;
        break;
    case 0x004406F0:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,29,sx((uint32_t)r(&n,29)-0x20u));break;
    case 0x004406F4:case 0x004406F8:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        store=(uint32_t)r(&n,29)+(pc==0x004406F8?0x10u:0);
        src=pc==0x004406F8?31u:16u;break;
    case 0x0043FBA0:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,3,sx(s->cop0_status));break;
    case 0x0043FBA4:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,2,sx(0x10000u));break;
    case 0x0043FBA8:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,3,r(&n,3)&r(&n,2));break;
    case 0x0043FBAC:
        if(delay||r(&n,3))return AE3_T2RC_UNSUPPORTED;
        n.in_delay_slot=1;n.branch_target=0x0043FBE4u;break;
    case 0x0043FBB0:
        if(!delay||next!=0x0043FBE4u)return AE3_T2RC_UNSUPPORTED;
        w(&n,4,r(&n,3)!=0);break;
    case 0x0043FBE4:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,4,0);break;
    case 0x0043FBE8:
        if(delay||r(&n,31)!=0x00440704u)return AE3_T2RC_UNSUPPORTED;
        n.in_delay_slot=1;n.branch_target=(uint32_t)r(&n,31);break;
    case 0x0043FBEC:
        if(!delay||next!=0x00440704u)return AE3_T2RC_UNSUPPORTED;
        w(&n,2,r(&n,4));break;
    case 0x00440704:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,16,r(&n,2));break;
    case 0x00440708:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,2,sx(0x10000000u));break;
    case 0x0044070C:
        if(delay)return AE3_T2RC_UNSUPPORTED;
        w(&n,2,r(&n,2)|0x1010u);break;
    default:return AE3_T2RC_UNSUPPORTED;
    }
    if(src){
        uint8_t data[8];
        if(store&7)return AE3_T2RC_MEMORY_FAULT;
        uint64_t value=r(&n,src);
        for(unsigned k=0;k<8;k++)data[k]=(uint8_t)(value>>(8u*k));
        if(ae3_ee_bus_write(bus,store,data,8)!=AE3_EE_BUS_OK)return AE3_T2RC_MEMORY_FAULT;
    }
    n.pc=next;n.retired++;s->cpu=n;
    if(next==AE3_T2_RECHECK_MMIO_LW)return AE3_T2RC_MMIO_PENDING;
 }
 return AE3_T2RC_YIELDED;
}
