#include "ae3_boot_second_numeric.h"
#include <stddef.h>
/* Pinned guest opcodes: entire helper prologue and branch-specific math prefix,
   including unreachable opcodes which are nevertheless checked if encountered. */
static const struct {uint32_t pc,word;} code[]={
 {UINT32_C(0x0041EFD0),UINT32_C(0x27BDFFB0)},
 {UINT32_C(0x0041EFD4),UINT32_C(0x3C0F000F)},
 {UINT32_C(0x0041EFD8),UINT32_C(0xFFB20010)},
 {UINT32_C(0x0041EFDC),UINT32_C(0x35EFFFFF)},
 {UINT32_C(0x0041EFE0),UINT32_C(0xFFB50028)},
 {UINT32_C(0x0041EFE4),UINT32_C(0x0004703C)},
 {UINT32_C(0x0041EFE8),UINT32_C(0x0004903F)},
 {UINT32_C(0x0041EFEC),UINT32_C(0xFFB00000)},
 {UINT32_C(0x0041EFF0),UINT32_C(0xFFB10008)},
 {UINT32_C(0x0041EFF4),UINT32_C(0x0080102D)},
 {UINT32_C(0x0041EFF8),UINT32_C(0xFFB30018)},
 {UINT32_C(0x0041EFFC),UINT32_C(0x000E703F)},
 {UINT32_C(0x0041F000),UINT32_C(0xFFB40020)},
 {UINT32_C(0x0041F004),UINT32_C(0x01F2782A)},
 {UINT32_C(0x0041F008),UINT32_C(0xFFB60030)},
 {UINT32_C(0x0041F00C),UINT32_C(0xFFB70038)},
 {UINT32_C(0x0041F010),UINT32_C(0xFFBF0040)},
 {UINT32_C(0x0041F014),UINT32_C(0x15E00020)},
 {UINT32_C(0x0041F018),UINT32_C(0x0000A82D)},
 {UINT32_C(0x0041F01C),UINT32_C(0x3C0F7FFF)},
 {UINT32_C(0x0041F020),UINT32_C(0x35EFFFFF)},
 {UINT32_C(0x0041F024),UINT32_C(0x024F7824)},
 {UINT32_C(0x0041F028),UINT32_C(0x01EE7825)},
 {UINT32_C(0x0041F02C),UINT32_C(0x15E0000D)},
 {UINT32_C(0x0041F030),UINT32_C(0x3C0F0073)},
 {UINT32_C(0x0041F034),UINT32_C(0xDDE298E0)},
 {UINT32_C(0x0041F038),UINT32_C(0xDFB00000)},
 {UINT32_C(0x0041F03C),UINT32_C(0xDFB10008)},
 {UINT32_C(0x0041F040),UINT32_C(0xDFB20010)},
 {UINT32_C(0x0041F044),UINT32_C(0xDFB30018)},
 {UINT32_C(0x0041F048),UINT32_C(0xDFB40020)},
 {UINT32_C(0x0041F04C),UINT32_C(0xDFB50028)},
 {UINT32_C(0x0041F050),UINT32_C(0xDFB60030)},
 {UINT32_C(0x0041F054),UINT32_C(0xDFB70038)},
 {UINT32_C(0x0041F058),UINT32_C(0xDFBF0040)},
 {UINT32_C(0x0041F05C),UINT32_C(0x03E00008)},
 {UINT32_C(0x0041F060),UINT32_C(0x27BD0050)},
 {UINT32_C(0x0041F064),UINT32_C(0x06430008)},
 {UINT32_C(0x0041F068),UINT32_C(0x24050435)},
 {UINT32_C(0x0041F06C),UINT32_C(0x0C108F88)},
 {UINT32_C(0x0041F070),UINT32_C(0x0080282D)},
 {UINT32_C(0x0041F074),UINT32_C(0x0040202D)},
 {UINT32_C(0x0041F078),UINT32_C(0x0C109036)},
 {UINT32_C(0x0041F07C),UINT32_C(0x0000282D)},
 {UINT32_C(0x0041F080),UINT32_C(0x1000FFEE)},
 {UINT32_C(0x0041F084),UINT32_C(0xDFB00000)},
 {UINT32_C(0x0041F088),UINT32_C(0x2415FFCA)},
 {UINT32_C(0x0041F08C),UINT32_C(0x0C108FA2)},
 {UINT32_C(0x0041F090),UINT32_C(0x00052D3C)},
 {UINT32_C(0x0041F094),UINT32_C(0x0002903F)},
 {UINT32_C(0x0041F098),UINT32_C(0x3C0F7FEF)},
 {UINT32_C(0x0041F09C),UINT32_C(0x35EFFFFF)},
 {UINT32_C(0x0041F0A0),UINT32_C(0x01F2782A)},
 {UINT32_C(0x0041F0A4),UINT32_C(0x11E00006)},
 {UINT32_C(0x0041F0A8),UINT32_C(0x0040202D)},
 {UINT32_C(0x0041F0AC),UINT32_C(0x0040282D)},
 {UINT32_C(0x0041F0B0),UINT32_C(0x0C108F72)},
 {UINT32_C(0x0041F0B4),UINT32_C(0x00000000)},
 {UINT32_C(0x0041F0B8),UINT32_C(0x1000FFE0)},
 {UINT32_C(0x0041F0BC),UINT32_C(0xDFB00000)},
 {UINT32_C(0x0041F0C0),UINT32_C(0x3C10000F)},
 {UINT32_C(0x0041F0C4),UINT32_C(0x00126503)},
 {UINT32_C(0x0041F0C8),UINT32_C(0x3610FFFF)},
 {UINT32_C(0x0041F0CC),UINT32_C(0x3C0E0009)},
 {UINT32_C(0x0041F0D0),UINT32_C(0x02509024)},
 {UINT32_C(0x0041F0D4),UINT32_C(0x35CE5F64)},
 {UINT32_C(0x0041F0D8),UINT32_C(0x024E7021)},
 {UINT32_C(0x0041F0DC),UINT32_C(0x3C0D0010)},
 {UINT32_C(0x0041F0E0),UINT32_C(0x01CD9824)},
 {UINT32_C(0x0041F0E4),UINT32_C(0x3C0F3FF0)},
 {UINT32_C(0x0041F0E8),UINT32_C(0x026F7826)},
 {UINT32_C(0x0041F0EC),UINT32_C(0x0002703C)},
 {UINT32_C(0x0041F0F0),UINT32_C(0x024F7825)},
 {UINT32_C(0x0041F0F4),UINT32_C(0x000E703E)},
 {UINT32_C(0x0041F0F8),UINT32_C(0x000F783C)},
 {UINT32_C(0x0041F0FC),UINT32_C(0x02AC6021)},
 {UINT32_C(0x0041F100),UINT32_C(0x01CF1025)},
 {UINT32_C(0x0041F104),UINT32_C(0x2595FC01)},
 {UINT32_C(0x0041F108),UINT32_C(0x00137D03)},
 {UINT32_C(0x0041F10C),UINT32_C(0x240503FF)},
 {UINT32_C(0x0041F110),UINT32_C(0x02AFA821)},
 {UINT32_C(0x0041F114),UINT32_C(0x0040202D)},
 {UINT32_C(0x0041F118),UINT32_C(0x0C108F88)},
};
static uint64_t rr(const AE3_BootPrefix *s,unsigned r){return r?s->gpr[r][0]:0;}
static void wr(AE3_BootPrefix *s,unsigned r,uint64_t v){if(r)s->gpr[r][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
static AE3_EEBusStatus st64(AE3_EEMemoryBus *bus,uint32_t a,uint64_t v){
 if(a&7u)return AE3_EE_BUS_MISALIGNED;
 uint8_t buf[8];for(unsigned i=0;i<8;i++)buf[i]=(uint8_t)(v>>(8*i));
 return ae3_ee_bus_write(bus,a,buf,8);
}
static AE3_EEBusStatus ld64(const AE3_EEMemoryBus *bus,uint32_t a,uint64_t *v){
 if(a&7u)return AE3_EE_BUS_MISALIGNED;
 uint8_t buf[8];AE3_EEBusStatus s=ae3_ee_bus_read(bus,a,buf,8);
 if(s!=AE3_EE_BUS_OK)return s;
 *v=0;for(unsigned i=0;i<8;i++)*v|=(uint64_t)buf[i]<<(8*i);
 return AE3_EE_BUS_OK;
}
AE3_SecondNumericResult ae3_boot_second_numeric_enter(AE3_BootSecondNumeric *out,
 const AE3_BootSecondConstructor *prior){
 if(!out||!prior||!prior->active||prior->cpu.in_delay_slot||
 prior->cpu.pc!=AE3_SECOND_MATH_ENTRY||prior->wrapper_visits!=1||
 prior->stub_calls!=2||prior->cpu.gpr[4][0]!=(UINT64_C(0x1009)<<50)||
 prior->cpu.gpr[31][0]!=UINT64_C(0x00417BD4))return AE3_SN_INVALID;
 *out=(AE3_BootSecondNumeric){0};out->cpu=prior->cpu;out->active=1;
 return AE3_SN_YIELDED;
}
AE3_SecondNumericResult ae3_boot_second_numeric_slice(AE3_BootSecondNumeric *s,
 AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_SN_INVALID;
 for(uint32_t k=0;k<budget;k++){
   uint32_t pc=s->cpu.pc;
   if(pc==AE3_SECOND_MATH_CALLEE_JAL&&!s->cpu.in_delay_slot)
     return AE3_SN_MATH_CALL_PENDING;
   size_t j=0;for(;j<sizeof(code)/sizeof(code[0]);j++)if(code[j].pc==pc)break;
   if(j==sizeof(code)/sizeof(code[0]))return AE3_SN_UNSUPPORTED;
   uint32_t op=0;
   if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)return AE3_SN_MEMORY_FAULT;
   if(op!=code[j].word)return AE3_SN_UNSUPPORTED;
   AE3_BootSecondNumeric next=*s;
   unsigned delay=next.cpu.in_delay_slot;
   uint32_t target=delay?next.cpu.branch_target:pc+4u;
   if(delay){next.cpu.in_delay_slot=0;next.cpu.branch_target=0;}
   unsigned major=op>>26,rs=(op>>21)&31u,rt=(op>>16)&31u,rd=(op>>11)&31u;
   unsigned sh=(op>>6)&31u,fn=op&63u;
   int32_t imm=(int16_t)(op&0xffffu);
   uint64_t val=0;unsigned do_load=0,do_store=0;uint32_t address=0;
   if(op==0){/* NOP */}
   else if(!major){
      if(fn==0x08 && rt==0 && rd==0 && !delay){
         next.cpu.in_delay_slot=1;next.cpu.branch_target=(uint32_t)rr(&next.cpu,rs);
      }else if(fn==0x2d && (op&UINT32_C(0x7C0))==0){
         wr(&next.cpu,rd,rr(&next.cpu,rs)+rr(&next.cpu,rt));
      }else if(fn==0x21){
         wr(&next.cpu,rd,sx((uint32_t)rr(&next.cpu,rs)+(uint32_t)rr(&next.cpu,rt)));
      }else if(fn==0x24){
         wr(&next.cpu,rd,rr(&next.cpu,rs)&rr(&next.cpu,rt));
      }else if(fn==0x25){
         wr(&next.cpu,rd,rr(&next.cpu,rs)|rr(&next.cpu,rt));
      }else if(fn==0x26){
         wr(&next.cpu,rd,rr(&next.cpu,rs)^rr(&next.cpu,rt));
      }else if(fn==0x2a){
         wr(&next.cpu,rd,(int64_t)rr(&next.cpu,rs)<(int64_t)rr(&next.cpu,rt));
      }else if(fn==0x03 && rs==0){
         wr(&next.cpu,rd,sx((uint32_t)((int32_t)(uint32_t)rr(&next.cpu,rt)>>sh)));
      }else if(fn==0x3c && rs==0){
         wr(&next.cpu,rd,rr(&next.cpu,rt)<<(32u+sh));
      }else if(fn==0x3e && rs==0){
         wr(&next.cpu,rd,rr(&next.cpu,rt)>>(32u+sh));
      }else if(fn==0x3f && rs==0){
         wr(&next.cpu,rd,(uint64_t)((int64_t)rr(&next.cpu,rt)>>(32u+sh)));
      }else return AE3_SN_UNSUPPORTED;
   }else if(major==0x09){
      wr(&next.cpu,rt,sx((uint32_t)rr(&next.cpu,rs)+(uint32_t)imm));
   }else if(major==0x19){
      wr(&next.cpu,rt,rr(&next.cpu,rs)+(uint64_t)(int64_t)imm);
   }else if(major==0x0c){
      wr(&next.cpu,rt,rr(&next.cpu,rs)&(op&UINT32_C(0xffff)));
   }else if(major==0x0d){
      wr(&next.cpu,rt,rr(&next.cpu,rs)|(op&UINT32_C(0xffff)));
   }else if(major==0x0f && !rs){
      wr(&next.cpu,rt,sx((op&UINT32_C(0xffff))<<16));
   }else if((major==0x04||major==0x05) && !delay){
      unsigned taken=major==4?rr(&next.cpu,rs)==rr(&next.cpu,rt):
                                rr(&next.cpu,rs)!=rr(&next.cpu,rt);
      next.cpu.in_delay_slot=1;
      next.cpu.branch_target=taken?
          (uint32_t)((int64_t)(pc+4u)+(int64_t)imm*4):pc+8u;
      if(taken)next.math_branches_taken++;
   }else if(major==0x37||major==0x3f){
      address=(uint32_t)(rr(&next.cpu,rs)+(uint64_t)(int64_t)imm);
      do_load=major==0x37;do_store=!do_load;
   }else return AE3_SN_UNSUPPORTED;
   if(do_load){
      if(ld64(bus,address,&val)!=AE3_EE_BUS_OK)return AE3_SN_MEMORY_FAULT;
      wr(&next.cpu,rt,val);
   }
   if(do_store && st64(bus,address,rr(&next.cpu,rt))!=AE3_EE_BUS_OK)
      return AE3_SN_MEMORY_FAULT;
   if(pc==UINT32_C(0x0041EFE8))next.double_high_word=(uint32_t)rr(&next.cpu,18);
   next.cpu.pc=target;next.cpu.retired++;
   *s=next;
   if(!next.cpu.in_delay_slot && next.cpu.pc==AE3_SECOND_MATH_CALLEE_JAL)
      return AE3_SN_MATH_CALL_PENDING;
 }
 return AE3_SN_YIELDED;
}