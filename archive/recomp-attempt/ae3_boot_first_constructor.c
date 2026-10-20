#include "ae3_boot_first_constructor.h"
#include <stddef.h>

/* Exact opcodes from the owner-supplied retail EE RAM reference. The runtime
   fetches from the guest bus AND checks the opcode against this allowlist. */
static const struct {uint32_t pc, op;} retail[] = {
    {UINT32_C(0x00428AE8),UINT32_C(0x01C0F809)},
    {UINT32_C(0x00428AEC),UINT32_C(0x2610FFFC)},
    {UINT32_C(0x00428AF0),UINT32_C(0x8E0F0000)},
    {UINT32_C(0x00428AF4),UINT32_C(0x15F1FFFC)},
    {UINT32_C(0x00428AF8),UINT32_C(0x01E0702D)},
    {UINT32_C(0x004173B0),UINT32_C(0x03E00008)},
    {UINT32_C(0x004173B4),UINT32_C(0x00000000)},
    {UINT32_C(0x0041A088),UINT32_C(0x27BDFFE0)},
    {UINT32_C(0x0041A08C),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A090),UINT32_C(0xFFB00000)},
    {UINT32_C(0x0041A094),UINT32_C(0xFFB10008)},
    {UINT32_C(0x0041A098),UINT32_C(0xFFBF0010)},
    {UINT32_C(0x0041A09C),UINT32_C(0x00A0802D)},
    {UINT32_C(0x0041A0A0),UINT32_C(0x14AF000E)},
    {UINT32_C(0x0041A0A4),UINT32_C(0x0080882D)},
    {UINT32_C(0x0041A0A8),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A0AC),UINT32_C(0x148F000B)},
    {UINT32_C(0x0041A0B0),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A0B4),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A0B8),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A0BC),UINT32_C(0xDDEED608)},
    {UINT32_C(0x0041A0C0),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A0C4),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A0C8),UINT32_C(0xFDEED608)},
    {UINT32_C(0x0041A0CC),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A0D0),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A0D4),UINT32_C(0x2484D5B8)},
    {UINT32_C(0x0041A0D8),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A0DC),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A0E0),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A0E4),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A0E8),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A0EC),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A0F0),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A0F4),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A0F8),UINT32_C(0xDDEED600)},
    {UINT32_C(0x0041A0FC),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A100),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A104),UINT32_C(0xFDEED600)},
    {UINT32_C(0x0041A108),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A10C),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A110),UINT32_C(0x2484D618)},
    {UINT32_C(0x0041A114),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A118),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A11C),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A120),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A124),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A128),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A12C),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A130),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A134),UINT32_C(0xDDEED5F0)},
    {UINT32_C(0x0041A138),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A13C),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A140),UINT32_C(0xFDEED5F0)},
    {UINT32_C(0x0041A144),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A148),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A14C),UINT32_C(0x2484D5C0)},
    {UINT32_C(0x0041A150),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A154),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A158),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A15C),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A160),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A164),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A168),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A16C),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A170),UINT32_C(0xDDEED5B0)},
    {UINT32_C(0x0041A174),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A178),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A17C),UINT32_C(0xFDEED5B0)},
    {UINT32_C(0x0041A180),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A184),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A188),UINT32_C(0x2484D630)},
    {UINT32_C(0x0041A18C),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A190),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A194),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A198),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A19C),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A1A0),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A1A4),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A1A8),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A1AC),UINT32_C(0xDDEED638)},
    {UINT32_C(0x0041A1B0),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A1B4),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A1B8),UINT32_C(0xFDEED638)},
    {UINT32_C(0x0041A1BC),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A1C0),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A1C4),UINT32_C(0x2484D5E8)},
    {UINT32_C(0x0041A1C8),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A1CC),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A1D0),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A1D4),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A1D8),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A1DC),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A1E0),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A1E4),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A1E8),UINT32_C(0xDDEED5C8)},
    {UINT32_C(0x0041A1EC),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A1F0),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A1F4),UINT32_C(0xFDEED5C8)},
    {UINT32_C(0x0041A1F8),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A1FC),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A200),UINT32_C(0x2484D650)},
    {UINT32_C(0x0041A204),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A208),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A20C),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A210),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A214),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A218),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A21C),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A220),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A224),UINT32_C(0xDDEED610)},
    {UINT32_C(0x0041A228),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A22C),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A230),UINT32_C(0xFDEED610)},
    {UINT32_C(0x0041A234),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A238),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A23C),UINT32_C(0x2484D5D0)},
    {UINT32_C(0x0041A240),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A244),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A248),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A24C),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A250),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A254),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A258),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A25C),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A260),UINT32_C(0xDDEED5D8)},
    {UINT32_C(0x0041A264),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A268),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A26C),UINT32_C(0xFDEED5D8)},
    {UINT32_C(0x0041A270),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A274),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A278),UINT32_C(0x2484D620)},
    {UINT32_C(0x0041A27C),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A280),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A284),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A288),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A28C),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A290),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A294),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A298),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A29C),UINT32_C(0xDDEED5F8)},
    {UINT32_C(0x0041A2A0),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A2A4),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A2A8),UINT32_C(0xFDEED5F8)},
    {UINT32_C(0x0041A2AC),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A2B0),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A2B4),UINT32_C(0x2484D628)},
    {UINT32_C(0x0041A2B8),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A2BC),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A2C0),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A2C4),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A2C8),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A2CC),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A2D0),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A2D4),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A2D8),UINT32_C(0xDDEED658)},
    {UINT32_C(0x0041A2DC),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A2E0),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A2E4),UINT32_C(0xFDEED658)},
    {UINT32_C(0x0041A2E8),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A2EC),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A2F0),UINT32_C(0x2484D640)},
    {UINT32_C(0x0041A2F4),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A2F8),UINT32_C(0x160F000E)},
    {UINT32_C(0x0041A2FC),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A300),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A304),UINT32_C(0x162F000B)},
    {UINT32_C(0x0041A308),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A30C),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A310),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A314),UINT32_C(0xDDEED5A8)},
    {UINT32_C(0x0041A318),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A31C),UINT32_C(0x15CD0004)},
    {UINT32_C(0x0041A320),UINT32_C(0xFDEED5A8)},
    {UINT32_C(0x0041A324),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A328),UINT32_C(0x0C105CEC)},
    {UINT32_C(0x0041A32C),UINT32_C(0x2484D5A0)},
    {UINT32_C(0x0041A330),UINT32_C(0x340FFFFF)},
    {UINT32_C(0x0041A334),UINT32_C(0x160F0011)},
    {UINT32_C(0x0041A338),UINT32_C(0xDFB00000)},
    {UINT32_C(0x0041A33C),UINT32_C(0x240F0001)},
    {UINT32_C(0x0041A340),UINT32_C(0x162F000F)},
    {UINT32_C(0x0041A344),UINT32_C(0xDFB10008)},
    {UINT32_C(0x0041A348),UINT32_C(0x3C0F0073)},
    {UINT32_C(0x0041A34C),UINT32_C(0x240D0001)},
    {UINT32_C(0x0041A350),UINT32_C(0xDDEED5E0)},
    {UINT32_C(0x0041A354),UINT32_C(0x65CE0001)},
    {UINT32_C(0x0041A358),UINT32_C(0x15CD0009)},
    {UINT32_C(0x0041A35C),UINT32_C(0xFDEED5E0)},
    {UINT32_C(0x0041A360),UINT32_C(0x3C040073)},
    {UINT32_C(0x0041A364),UINT32_C(0xDFB00000)},
    {UINT32_C(0x0041A368),UINT32_C(0xDFB10008)},
    {UINT32_C(0x0041A36C),UINT32_C(0x2484D648)},
    {UINT32_C(0x0041A370),UINT32_C(0xDFBF0010)},
    {UINT32_C(0x0041A374),UINT32_C(0x08105CEC)},
    {UINT32_C(0x0041A378),UINT32_C(0x27BD0020)},
    {UINT32_C(0x0041A37C),UINT32_C(0xDFB10008)},
    {UINT32_C(0x0041A380),UINT32_C(0xDFBF0010)},
    {UINT32_C(0x0041A384),UINT32_C(0x03E00008)},
    {UINT32_C(0x0041A388),UINT32_C(0x27BD0020)},
    {UINT32_C(0x0041A38C),UINT32_C(0x00000000)},
    {UINT32_C(0x0041A390),UINT32_C(0x27BDFFF0)},
    {UINT32_C(0x0041A394),UINT32_C(0x24040001)},
    {UINT32_C(0x0041A398),UINT32_C(0xFFBF0000)},
    {UINT32_C(0x0041A39C),UINT32_C(0x3405FFFF)},
    {UINT32_C(0x0041A3A0),UINT32_C(0xDFBF0000)},
    {UINT32_C(0x0041A3A4),UINT32_C(0x08106822)},
    {UINT32_C(0x0041A3A8),UINT32_C(0x27BD0010)},
};
static uint64_t rg(const AE3_BootPrefix *s,unsigned id){
    return id?s->gpr[id][0]:0;
}
static void wr(AE3_BootPrefix *s,unsigned id,uint64_t v){
    if(id)s->gpr[id][0]=v;
}
static uint64_t sext32(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static uint32_t jump_target(uint32_t pc,uint32_t word){
    return ((pc+4u)&UINT32_C(0xF0000000)) |
      ((word&UINT32_C(0x03FFFFFF))<<2);
}
static AE3_EEBusStatus read_mem(const AE3_EEMemoryBus *b,
    uint32_t address,unsigned n,uint64_t *out){
    uint8_t raw[8];uint64_t v=0;
    if(address&(n-1u))return AE3_EE_BUS_MISALIGNED;
    const AE3_EEBusStatus st=ae3_ee_bus_read(b,address,raw,n);
    if(st!=AE3_EE_BUS_OK)return st;
    for(unsigned i=0;i<n;i++)v|=(uint64_t)raw[i]<<(i*8u);
    *out=n==4?sext32((uint32_t)v):v;
    return AE3_EE_BUS_OK;
}
static AE3_EEBusStatus write_mem(AE3_EEMemoryBus *b,
    uint32_t address,unsigned n,uint64_t value){
    uint8_t raw[8];
    if(address&(n-1u))return AE3_EE_BUS_MISALIGNED;
    for(unsigned i=0;i<n;i++)raw[i]=(uint8_t)(value>>(i*8u));
    return ae3_ee_bus_write(b,address,raw,n);
}
AE3_FirstCtorResult ae3_boot_first_ctor_enter(AE3_BootFirstConstructor *out,
    const AE3_BootStaticDispatch *prior){
    if(!out||!prior||!prior->active||prior->cpu.in_delay_slot||
      prior->cpu.pc!=UINT32_C(0x00428AE8)||
      prior->guest_ctor_word!=AE3_FIRST_CTOR_PC||
      (uint32_t)rg(&prior->cpu,14)!=AE3_FIRST_CTOR_PC||
      (uint32_t)rg(&prior->cpu,17)!=UINT32_C(0xFFFFFFFF))
        return AE3_FCTOR_INVALID;
    *out=(AE3_BootFirstConstructor){0};
    out->cpu=prior->cpu;out->active=1;
    return AE3_FCTOR_YIELDED;
}
AE3_FirstCtorResult ae3_boot_first_ctor_slice(AE3_BootFirstConstructor *state,
    AE3_EEMemoryBus *bus,uint32_t budget){
    if(!state||!state->active||!bus||state->cpu.in_delay_slot>1)
      return AE3_FCTOR_INVALID;
    for(uint32_t step=0;step<budget;step++){
        const uint32_t pc=state->cpu.pc;
        if(!state->cpu.in_delay_slot && pc==UINT32_C(0x00428AE8) &&
           state->next_constructor){
            if(state->next_constructor!=AE3_SECOND_CTOR_PC ||
              (uint32_t)rg(&state->cpu,14)!=AE3_SECOND_CTOR_PC)
              return AE3_FCTOR_UNSUPPORTED;
            return AE3_FCTOR_SECOND_PENDING;
        }
        if(!state->cpu.in_delay_slot && pc==UINT32_C(0x00428AFC))
          return AE3_FCTOR_DISPATCH_DONE;
        size_t index=0;
        for(;index<sizeof(retail)/sizeof(retail[0]);index++)
            if(retail[index].pc==pc)break;
        if(index==sizeof(retail)/sizeof(retail[0]))
            return AE3_FCTOR_UNSUPPORTED;
        uint32_t op=0;
        if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)
            return AE3_FCTOR_MEMORY_FAULT;
        if(op!=retail[index].op)return AE3_FCTOR_UNSUPPORTED;
        AE3_BootFirstConstructor next=*state;
        const unsigned delay=next.cpu.in_delay_slot;
        uint32_t target=delay?next.cpu.branch_target:pc+4u;
        if(delay){next.cpu.in_delay_slot=0;next.cpu.branch_target=0;}
        const unsigned primary=op>>26,rs=(op>>21)&31u,rt=(op>>16)&31u;
        const unsigned rd=(op>>11)&31u,funct=op&63u;
        const int32_t imm=(int16_t)(op&UINT32_C(0xFFFF));
        uint32_t address=0;uint64_t value=0;
        unsigned load=0,store=0,bytes=0;
        if(op==0){ /* NOP */
        }else if(primary==0){
            if(funct==0x08 && rt==0&&rd==0&&!delay){
                next.cpu.in_delay_slot=1;
                next.cpu.branch_target=(uint32_t)rg(&next.cpu,rs);
            }else if(funct==0x09 && rt==0 && !delay){
                next.cpu.in_delay_slot=1;
                next.cpu.branch_target=(uint32_t)rg(&next.cpu,rs);
                wr(&next.cpu,rd,pc+8u);
            }else if(funct==0x2d && (op&UINT32_C(0x7C0))==0){
                if(delay && pc!=UINT32_C(0x00428AF8) &&
                   pc!=UINT32_C(0x0041A0A4))return AE3_FCTOR_UNSUPPORTED;
                wr(&next.cpu,rd,rg(&next.cpu,rs)+rg(&next.cpu,rt));
            }else return AE3_FCTOR_UNSUPPORTED;
        }else if(primary==0x09){
            wr(&next.cpu,rt,sext32((uint32_t)rg(&next.cpu,rs)+(uint32_t)imm));
        }else if(primary==0x19){
            wr(&next.cpu,rt,rg(&next.cpu,rs)+(uint64_t)(int64_t)imm);
        }else if(primary==0x0d){
            wr(&next.cpu,rt,rg(&next.cpu,rs)|(op&UINT32_C(0xFFFF)));
        }else if(primary==0x0f){
            if(rs)return AE3_FCTOR_UNSUPPORTED;
            wr(&next.cpu,rt,sext32((op&UINT32_C(0xFFFF))<<16));
        }else if(primary==0x04||primary==0x05){
            if(delay)return AE3_FCTOR_UNSUPPORTED;
            const int condition=(primary==0x04)?
                rg(&next.cpu,rs)==rg(&next.cpu,rt):
                rg(&next.cpu,rs)!=rg(&next.cpu,rt);
            next.cpu.in_delay_slot=1;
            next.cpu.branch_target=condition?
                (uint32_t)((int64_t)(uint64_t)(pc+4u)+(int64_t)imm*4):pc+8u;
        }else if(primary==0x02||primary==0x03){
            if(delay)return AE3_FCTOR_UNSUPPORTED;
            next.cpu.in_delay_slot=1;
            next.cpu.branch_target=jump_target(pc,op);
            if(primary==3){
                wr(&next.cpu,31,pc+8u);

            }
        }else if(primary==0x23||primary==0x37||primary==0x3F){
            if(delay && pc!=UINT32_C(0x0041A0C8) &&
                pc!=UINT32_C(0x0041A104) && pc!=UINT32_C(0x0041A140) &&
                pc!=UINT32_C(0x0041A17C) && pc!=UINT32_C(0x0041A1B8) &&
                pc!=UINT32_C(0x0041A1F4) && pc!=UINT32_C(0x0041A230) &&
                pc!=UINT32_C(0x0041A26C) && pc!=UINT32_C(0x0041A2A8) &&
                pc!=UINT32_C(0x0041A2E4) && pc!=UINT32_C(0x0041A320) &&
                pc!=UINT32_C(0x0041A338) && pc!=UINT32_C(0x0041A344) &&
                pc!=UINT32_C(0x0041A35C))return AE3_FCTOR_UNSUPPORTED;
            address=(uint32_t)(rg(&next.cpu,rs)+(uint64_t)(int64_t)imm);
            bytes=primary==0x23?4:8;
            load=primary!=0x3F;store=!load;
        }else return AE3_FCTOR_UNSUPPORTED;
        if(load){
            if(read_mem(bus,address,bytes,&value)!=AE3_EE_BUS_OK)
                return AE3_FCTOR_MEMORY_FAULT;
            wr(&next.cpu,rt,value);
            if(pc==UINT32_C(0x00428AF0))
                next.next_constructor=(uint32_t)value;
        }
        if(store){
            if(write_mem(bus,address,bytes,rg(&next.cpu,rt))!=AE3_EE_BUS_OK)
                return AE3_FCTOR_MEMORY_FAULT;
            if(pc>=UINT32_C(0x0041A0C8)&&pc<=UINT32_C(0x0041A35C))
                next.reference_count_stores++;
        }
        if(pc==AE3_FIRST_CTOR_STUB)next.stub_calls++;
        next.cpu.pc=target;
        next.cpu.retired++;
        *state=next;
        if(!next.cpu.in_delay_slot && next.cpu.pc==UINT32_C(0x00428AE8) &&
           next.next_constructor==AE3_SECOND_CTOR_PC)
            return AE3_FCTOR_SECOND_PENDING;
        if(!next.cpu.in_delay_slot&&next.cpu.pc==UINT32_C(0x00428AFC))
            return AE3_FCTOR_DISPATCH_DONE;
    }
    return AE3_FCTOR_YIELDED;
}