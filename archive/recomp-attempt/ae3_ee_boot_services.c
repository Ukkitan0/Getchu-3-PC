#include "ae3_ee_boot_services.h"
#define EE_RAM_LIMIT UINT32_C(0x02000000)
static int range_ok(const AE3_EEMemoryBus *b, uint32_t start, uint32_t bytes) {
    uint32_t scratch;
    uint64_t end=(uint64_t)start+bytes;
    if(!b || !b->ram || !bytes || end>EE_RAM_LIMIT || end>b->ram_size)
        return 0;
    return ae3_ee_bus_read32(b,start&~UINT32_C(3),&scratch)==AE3_EE_BUS_OK &&
        ae3_ee_bus_read32(b,(uint32_t)(end-1)&~UINT32_C(3),&scratch)==AE3_EE_BUS_OK;
}
void ae3_ee_boot_services_init(AE3_EEBootServices *model,uint32_t reserve){
    if(!model)return;
    *model=(AE3_EEBootServices){0};
    model->reserved_top_bytes=reserve;
}
AE3_BootServiceResult ae3_ee_boot_services_dispatch(
    AE3_EEBootServices *model,AE3_BootKernelBridge *bridge,
    const AE3_EEMemoryBus *bus){
    if(!model||!bridge||!bus)return AE3_BS_BAD_ARGUMENT;
    if(bus->ram_size<EE_RAM_LIMIT||!bus->ram)return AE3_BS_UNMAPPED;
    AE3_EEBootServices next_model=*model;
    AE3_BootKernelBridge next_guest=*bridge;
    if(bridge->pending_syscall==1 && bridge->request.number==0x3cu){
        if(model->thread_created || model->heap_created || model->generation)return AE3_BS_WRONG_SYSCALL;
        uint32_t gp=bridge->request.arguments[0],stack=bridge->request.arguments[1],
                 size=bridge->request.arguments[2],args=bridge->request.arguments[3],
                 root=bridge->request.arguments[4],reserve=model->reserved_top_bytes;
        uint64_t top=(uint64_t)stack+size;
        if((stack&15)||(size&15)||size<32 || (reserve&15)||reserve<16 ||
           (uint64_t)reserve>=size || top>EE_RAM_LIMIT ||
           (gp&3)||(args&3)||(root&3))return AE3_BS_BAD_ARGUMENT;
        if(!range_ok(bus,stack,size)||!range_ok(bus,gp,4)||
           !range_ok(bus,args,4)||!range_ok(bus,root,4))return AE3_BS_UNMAPPED;
        uint32_t sp=(uint32_t)(top-reserve);
        if(ae3_boot_kernel_return_thread(&next_guest,sp)!=AE3_KBOOT_YIELDED)
            return AE3_BS_BAD_ARGUMENT;
        next_model.gp=gp;next_model.stack_base=stack;next_model.stack_size=size;
        next_model.stack_pointer=sp;next_model.args_pointer=args;next_model.exit_function=root;
        next_model.thread_created=1;next_model.generation++;
    }else if(bridge->pending_syscall==2 && bridge->request.number==0x3du){
        if(!model->thread_created||model->heap_created||model->generation!=1)
            return AE3_BS_WRONG_SYSCALL;
        uint32_t start=bridge->request.arguments[0],size=bridge->request.arguments[1];
        uint64_t end=(uint64_t)start+size;
        uint64_t stack_end=(uint64_t)model->stack_base+model->stack_size;
        if((start&15)||!size||end>EE_RAM_LIMIT)return AE3_BS_BAD_ARGUMENT;
        if(!range_ok(bus,start,size))return AE3_BS_UNMAPPED;
        if(!((uint64_t)start>=stack_end || end<=model->stack_base))
            return AE3_BS_OVERLAP;
        if(ae3_boot_kernel_ack_heap(&next_guest)!=AE3_KBOOT_YIELDED)
            return AE3_BS_BAD_ARGUMENT;
        next_model.heap_base=start;next_model.heap_size=size;
        next_model.heap_created=1;next_model.generation++;
    }else return AE3_BS_WRONG_SYSCALL;
    *model=next_model;*bridge=next_guest;
    return AE3_BS_APPLIED;
}