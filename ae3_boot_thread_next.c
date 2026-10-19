#include "ae3_boot_thread_next.h"
#include <stddef.h>
static uint64_t r(const AE3_BootPrefix *s,unsigned n){return n?s->gpr[n][0]:0;}
static void w(AE3_BootPrefix *s,unsigned n,uint64_t v){if(n)s->gpr[n][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static const struct {uint32_t pc,op;} code[]={
 {0x00436C20,0x27BDFF90},{0x00436C24,0xFFB20050},
 {0x00436C28,0xFFB10040},{0x00436C2C,0x00A0902D},
 {0x00436C30,0xFFBF0060},{0x00436C34,0x0080882D},
 {0x00436C38,0x0C10D96C},{0x00436C3C,0xFFB00030},
 {0x004365B0,0x40026000},{0x004365B4,0x38420001},
 {0x004365B8,0x03E00008},{0x004365BC,0x30420001},
 {0x00436C40,0x1440004F},{0x00436C44,0x2402FFFF},
 {0x00436D80,0xDFBF0060},{0x00436D84,0xDFB20050},
 {0x00436D88,0xDFB10040},{0x00436D8C,0xDFB00030},
 {0x00436D90,0x03E00008},{0x00436D94,0x27BD0070},
 {0x00436974,0x0C10D674},{0x00436978,0x00000000},
 {0x004359D0,0x2403002F}
};
static AE3_ThreadNextResult boundary(const AE3_BootThreadNext *s){
 if(s->cpu.in_delay_slot)return AE3_TNEXT_YIELDED;
 if(s->cpu.pc==AE3_TNEXT_GET_THREAD_ID)return AE3_TNEXT_GETID_PENDING;
 if(s->cpu.pc==AE3_TNEXT_CRITICAL_CALL)return AE3_TNEXT_CRITICAL_PENDING;
 return AE3_TNEXT_YIELDED;
}
AE3_ThreadNextResult ae3_boot_thread_next_enter(AE3_BootThreadNext *s,
 const AE3_BootThreadContinue *prior,uint32_t status){
 if(!s||!prior||!prior->active||prior->thread_id<=0||
    prior->cpu.in_delay_slot||prior->cpu.pc!=AE3_THR_NEXT_CALLEE||
    (status&UINT32_C(0x10000)))return AE3_TNEXT_INVALID;
 *s=(AE3_BootThreadNext){0};s->cpu=prior->cpu;
 s->cop0_status=status;s->active=1;
 return AE3_TNEXT_YIELDED;
}
AE3_ThreadNextResult ae3_boot_thread_next_slice(AE3_BootThreadNext *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_TNEXT_INVALID;
 if(boundary(s)!=AE3_TNEXT_YIELDED)return boundary(s);
 for(uint32_t i=0;i<budget;i++){
  const uint32_t pc=s->cpu.pc;uint32_t op=0;size_t j=0;
  for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
  if(j==sizeof(code)/sizeof(code[0]))return AE3_TNEXT_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)return AE3_TNEXT_MEMORY_FAULT;
  if(op!=code[j].op)return AE3_TNEXT_UNSUPPORTED;
  AE3_BootThreadNext n=*s;
  const int delay=n.cpu.in_delay_slot;
  uint32_t target=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  unsigned reg=0;uint32_t address=0;int access=0;/* 1=load, 2=store */
  switch(pc){
  case 0x00436C20:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)-0x70u));break;
  case 0x00436C24:case 0x00436C28:case 0x00436C30:case 0x00436C3C:
   if(delay!=(pc==0x00436c3cu))return AE3_TNEXT_UNSUPPORTED;
   if(pc==0x00436c3c && target!=0x004365b0u)return AE3_TNEXT_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,29)+(pc==0x00436c24?0x50u:
       pc==0x00436c28?0x40u:pc==0x00436c30?0x60u:0x30u);
   reg=pc==0x00436c24?18u:pc==0x00436c28?17u:
       pc==0x00436c30?31u:16u;access=2;break;
  case 0x00436C2C:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,18,r(&n.cpu,5));break;
  case 0x00436C34:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,17,r(&n.cpu,4));break;
  case 0x00436C38:case 0x00436974:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=pc==0x00436c38?0x004365b0u:0x004359d0u;break;
  case 0x004365B0:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,2,sx(n.cop0_status));break;
  case 0x004365B4:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,2,r(&n.cpu,2)^1u);break;
  case 0x004365B8:
   if(delay||r(&n.cpu,31)!=0x00436c40u)return AE3_TNEXT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x004365BC:
   if(!delay||target!=0x00436c40u)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,2,r(&n.cpu,2)&1u);break;
  case 0x00436C40:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=r(&n.cpu,2)?0x00436d80u:AE3_TNEXT_CRITICAL_CALL;break;
  case 0x00436C44:
   if(!delay||(target!=0x00436d80u&&target!=AE3_TNEXT_CRITICAL_CALL))
      return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,2,sx(0xffffffffu));break;
  case 0x00436D80:case 0x00436D84:case 0x00436D88:case 0x00436D8C:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   address=(uint32_t)r(&n.cpu,29)+(pc==0x00436d80?0x60u:
         pc==0x00436d84?0x50u:pc==0x00436d88?0x40u:0x30u);
   reg=pc==0x00436d80?31u:pc==0x00436d84?18u:
       pc==0x00436d88?17u:16u;access=1;break;
  case 0x00436D90:
   if(delay||r(&n.cpu,31)!=0x00436974u)return AE3_TNEXT_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)r(&n.cpu,31);break;
  case 0x00436D94:
   if(!delay||target!=0x00436974u)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,29,sx((uint32_t)r(&n.cpu,29)+0x70u));break;
  case 0x00436978:
   if(!delay||target!=0x004359d0u)return AE3_TNEXT_UNSUPPORTED;
   break;
  case 0x004359D0:
   if(delay)return AE3_TNEXT_UNSUPPORTED;
   w(&n.cpu,3,0x2fu);break;
  default:return AE3_TNEXT_UNSUPPORTED;
  }
  if(access){
   if(address&7u)return AE3_TNEXT_MEMORY_FAULT;
   uint8_t raw[8]={0};
   if(access==2){
    uint64_t value=r(&n.cpu,reg);
    for(unsigned q=0;q<8;q++)raw[q]=(uint8_t)(value>>(q*8u));
    if(ae3_ee_bus_write(bus,address,raw,8)!=AE3_EE_BUS_OK)
      return AE3_TNEXT_MEMORY_FAULT;
   }else{
    if(ae3_ee_bus_read(bus,address,raw,8)!=AE3_EE_BUS_OK)
      return AE3_TNEXT_MEMORY_FAULT;
    uint64_t value=0;
    for(unsigned q=0;q<8;q++)value|=(uint64_t)raw[q]<<(q*8u);
    w(&n.cpu,reg,value);
   }
  }
  n.cpu.pc=target;n.cpu.retired++;*s=n;
  if(boundary(s)!=AE3_TNEXT_YIELDED)return boundary(s);
 }
 return AE3_TNEXT_YIELDED;
}