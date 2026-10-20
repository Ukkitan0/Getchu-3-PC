#include "ae3_boot_kernel_bridge.h"
#include <limits.h>

static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static uint64_t sign32(uint32_t x) {return (uint64_t)(int64_t)(int32_t)x;}
static uint64_t gpr(const AE3_BootPrefix *s,unsigned index) {return index?s->gpr[index][0]:0;}
static void assign(AE3_BootPrefix *s,unsigned index,uint64_t value) {if(index)s->gpr[index][0]=value;}
static int correct_state(const AE3_BootKernelBridge *s) {
    return s && (s->cpu.pc&3)==0 && !s->cpu.in_delay_slot && !s->cpu.done &&
        s->cpu.pc>=AE3_KERNEL_THREAD_PC && s->cpu.pc<=AE3_KERNEL_NEXT_CALL_PC &&
        s->completed_thread<=1 && s->completed_heap<=1 &&
        s->finished<=1 && s->pending_syscall<=2;
}
static AE3_KernelBootResult halted(const AE3_BootKernelBridge *s) {
    if (s->pending_syscall==1) return AE3_KBOOT_SETUP_THREAD;
    if (s->pending_syscall==2) return AE3_KBOOT_SETUP_HEAP;
    return AE3_KBOOT_NEXT_CALL;
}
AE3_KernelBootResult ae3_boot_kernel_enter(AE3_BootKernelBridge *out,
                                            const AE3_BootThreadSetup *thread) {
    if(!out || !thread || !thread->finished || thread->cpu.in_delay_slot ||
       thread->cpu.pc!=AE3_KERNEL_THREAD_PC ||
       (uint32_t)gpr(&thread->cpu,3)!=0x3c) return AE3_KBOOT_INVALID;
    *out=(AE3_BootKernelBridge){0};
    out->cpu=thread->cpu;
    return AE3_KBOOT_YIELDED;
}
AE3_KernelBootResult ae3_boot_kernel_slice(AE3_BootKernelBridge *s,
    const uint8_t *ram, size_t ram_size, uint32_t budget) {
    if(!correct_state(s)||!ram)return AE3_KBOOT_INVALID;
    if(s->pending_syscall || s->finished)return halted(s);
    if(!budget)return AE3_KBOOT_YIELDED;
    for(uint32_t i=0;i<budget;++i) {
        uint32_t pc=s->cpu.pc;
        if(pc==AE3_KERNEL_NEXT_CALL_PC) {
            s->finished=1;
            return AE3_KBOOT_NEXT_CALL;
        }
        if ((size_t)pc>ram_size || ram_size-(size_t)pc<4)return AE3_KBOOT_MEMORY_FAULT;
        const uint32_t w=get32(ram+pc);
        if(pc==AE3_KERNEL_THREAD_PC || pc==AE3_KERNEL_HEAP_PC) {
            const uint32_t number=pc==AE3_KERNEL_THREAD_PC?0x3cu:0x3du;
            if(w!=0x0000000cu || (uint32_t)gpr(&s->cpu,3)!=number ||
              (number==0x3du && !s->completed_thread))return AE3_KBOOT_UNSUPPORTED;
            s->pending_syscall=(uint8_t)(number==0x3cu?1:2);
            s->request.number=number;
            s->request.pc=pc;
            for(unsigned j=0;j<5;j++)s->request.arguments[j]=(uint32_t)gpr(&s->cpu,4+j);
            return halted(s);
        }
        AE3_BootPrefix next=s->cpu;
        unsigned op=w>>26,rs=(w>>21)&31,rt=(w>>16)&31,rd=(w>>11)&31,sh=(w>>6)&31,fn=w&63;
        int32_t imm=(int16_t)(w&0xffff);
        if(w==0) { }
        else if(op==15 && rs==0)assign(&next,rt,sign32((uint32_t)(w&0xffff)<<16));
        else if(op==9)assign(&next,rt,sign32((uint32_t)gpr(&s->cpu,rs)+(uint32_t)imm));
        else if(op==0 && fn==0x2d && sh==0)assign(&next,rd,gpr(&s->cpu,rs)+gpr(&s->cpu,rt));
        else return AE3_KBOOT_UNSUPPORTED;
        next.pc+=4;
        next.retired++;
        s->cpu=next;
        if(s->cpu.pc==AE3_KERNEL_NEXT_CALL_PC) {
            s->finished=1;
            return AE3_KBOOT_NEXT_CALL;
        }
    }
    return AE3_KBOOT_YIELDED;
}
AE3_KernelBootResult ae3_boot_kernel_return_thread(AE3_BootKernelBridge *s,
                                                    uint32_t sp) {
    if(!correct_state(s) || s->pending_syscall!=1 || s->completed_thread ||
       s->cpu.pc!=AE3_KERNEL_THREAD_PC || s->request.pc!=AE3_KERNEL_THREAD_PC ||
       s->request.number!=0x3c)return AE3_KBOOT_INVALID;
    uint32_t base=s->request.arguments[1],n=s->request.arguments[2];
    uint64_t end=(uint64_t)base+n;
    /* Not an HLE guess: require kernel-provided 16-byte-aligned stack pointer. */
    if(!n || end>UINT32_MAX || sp<(uint64_t)base || (uint64_t)sp>end || (sp&15))
        return AE3_KBOOT_REJECTED;
    s->initial_stack_pointer=sp;
    assign(&s->cpu,2,sign32(sp)); /* actual syscall return in v0 */
    s->cpu.pc+=4;
    s->cpu.retired++;
    s->completed_thread=1;
    s->pending_syscall=0;
    return AE3_KBOOT_YIELDED;
}
AE3_KernelBootResult ae3_boot_kernel_ack_heap(AE3_BootKernelBridge *s) {
    if(!correct_state(s)||s->pending_syscall!=2||s->completed_heap||
       !s->completed_thread || s->cpu.pc!=AE3_KERNEL_HEAP_PC ||
       s->request.pc!=AE3_KERNEL_HEAP_PC ||s->request.number!=0x3d)
        return AE3_KBOOT_INVALID;
    uint32_t start=s->request.arguments[0],size=s->request.arguments[1];
    uint64_t end=(uint64_t)start+size;
    if(!start || (start&15) || !size || end>UINT32_C(0x02000000))
        return AE3_KBOOT_REJECTED;
    s->heap_start=start;
    s->heap_size=size;
    /* Void syscall; no invented value is placed in v0. */
    s->cpu.pc+=4;
    s->cpu.retired++;
    s->completed_heap=1;
    s->pending_syscall=0;
    return AE3_KBOOT_YIELDED;
}