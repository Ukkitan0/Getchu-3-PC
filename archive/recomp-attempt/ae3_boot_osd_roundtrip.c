#include "ae3_boot_osd_roundtrip.h"
#include <stddef.h>

static uint64_t rd(const AE3_BootPrefix *s,unsigned r){return r?s->gpr[r][0]:0;}
static void wr(AE3_BootPrefix *s,unsigned r,uint64_t v){if(r)s->gpr[r][0]=v;}
static uint64_t sx(uint32_t v){return (uint64_t)(int64_t)(int32_t)v;}
/* Original retail SCUS_975.01 instructions confirmed against the owner's
 * independently supplied 32-MiB PCSX2 EE RAM image. */
static const struct {uint32_t pc,word;} code[]={
 {0x00435B98,0x03E00008},{0x00435B9C,0x00000000},
 {0x00435B80,0x2403004A},{0x00435B88,0x03E00008},
 {0x00435B8C,0x00000000},{0x00435B90,0x2403004B},
 {0x0043FF2C,0x8FA30000},{0x0043FF30,0x3C02FFFF},
 {0x0043FF34,0x34421FFF},{0x0043FF38,0x27B00004},
 {0x0043FF3C,0x00621824},{0x0043FF40,0x0200202D},
 {0x0043FF44,0x34632000},{0x0043FF48,0x0C10D6E0},
 {0x0043FF4C,0xAFA30004},{0x0043FF50,0x0C10D6E4},
 {0x0043FF54,0x0200202D},{0x0043FF58,0x0C10D6E0},
 {0x0043FF5C,0x03A0202D},{0x0043FF60,0x8FA20004},
 {0x0043FF64,0xDFBF0020},{0x0043FF68,0x00021342},
 {0x0043FF6C,0xDFB00010},{0x0043FF70,0x30420007},
 {0x0043FF74,0x2C420001},{0x0043FF78,0x03E00008},
 {0x0043FF7C,0x27BD0030},{0x0043FF98,0x1040001E},
 {0x0043FF9C,0x3C02006A},{0x00440014,0xDFBF0030},
 {0x00440018,0xDFB20020},{0x0044001C,0xDFB10010},
 {0x00440020,0xDFB00000},{0x00440024,0x03E00008},
 {0x00440028,0x27BD0040}
};
static AE3_OsdRoundtripResult boundary(const AE3_BootOsdRoundtrip *s){
 if(s->cpu.in_delay_slot)return AE3_OSDR_YIELDED;
 if(s->cpu.pc==AE3_OSD_GET_PC)
  return s->read_count==s->write_count && s->read_count<2?
   AE3_OSDR_GET_PENDING:AE3_OSDR_INVALID;
 if(s->cpu.pc==AE3_OSD_SET_PC)
  return s->read_count==s->write_count+1 && s->write_count<2?
   AE3_OSDR_SET_PENDING:AE3_OSDR_INVALID;
 if(s->cpu.pc==AE3_OSD_PARENT_RETURN && s->read_count==2 && s->write_count==2)
  return AE3_OSDR_PARENT_RETURNED;
 return AE3_OSDR_YIELDED;
}
void ae3_ee_osd_reset(AE3_EEOsdConfig *o){if(o)*o=(AE3_EEOsdConfig){0};}
int ae3_ee_osd_bind(AE3_EEOsdConfig *o,uint32_t packed){
 if(!o||o->bound)return -1;
 o->packed=packed;o->bound=1;o->generation++;
 return 0;
}
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_enter(
 AE3_BootOsdRoundtrip *out,const AE3_BootOsdEntry *prior){
 if(!out||!prior||!prior->active||prior->cpu.in_delay_slot||
    prior->cpu.pc!=AE3_OSD_GET_PC||rd(&prior->cpu,3)!=0x4Bu)
  return AE3_OSDR_INVALID;
 *out=(AE3_BootOsdRoundtrip){0};out->cpu=prior->cpu;out->active=1;
 return AE3_OSDR_GET_PENDING;
}
static AE3_OsdRoundtripResult check_service(AE3_BootOsdRoundtrip *s,
 const AE3_EEMemoryBus *bus,uint32_t number){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot||
    (number==0x4Bu && boundary(s)!=AE3_OSDR_GET_PENDING)||
    (number==0x4Au && boundary(s)!=AE3_OSDR_SET_PENDING)||
    rd(&s->cpu,3)!=number||((uint32_t)rd(&s->cpu,4)&3u))
   return AE3_OSDR_INVALID;
 uint32_t op=0;
 if(ae3_ee_bus_read32(bus,s->cpu.pc,&op)!=AE3_EE_BUS_OK)
   return AE3_OSDR_MEMORY_FAULT;
 return op==12u?AE3_OSDR_YIELDED:AE3_OSDR_UNSUPPORTED;
}
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_get(
 AE3_BootOsdRoundtrip *s,AE3_EEMemoryBus *bus,const AE3_EEOsdConfig *o){
 AE3_OsdRoundtripResult v=check_service(s,bus,0x4Bu);
 if(v!=AE3_OSDR_YIELDED)return v;
 if(!o||!o->bound)return AE3_OSDR_UNBOUND;
 if(ae3_ee_bus_write32(bus,(uint32_t)rd(&s->cpu,4),o->packed)!=AE3_EE_BUS_OK)
  return AE3_OSDR_MEMORY_FAULT;
 s->read_count++;s->cpu.pc+=4;s->cpu.retired++;
 return AE3_OSDR_YIELDED;
}
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_set(
 AE3_BootOsdRoundtrip *s,const AE3_EEMemoryBus *bus,AE3_EEOsdConfig *o){
 AE3_OsdRoundtripResult v=check_service(s,bus,0x4Au);
 if(v!=AE3_OSDR_YIELDED)return v;
 if(!o||!o->bound)return AE3_OSDR_UNBOUND;
 uint32_t packed=0;
 if(ae3_ee_bus_read32(bus,(uint32_t)rd(&s->cpu,4),&packed)!=AE3_EE_BUS_OK)
  return AE3_OSDR_MEMORY_FAULT;
 o->packed=packed;o->generation++;
 s->write_count++;s->cpu.pc+=4;s->cpu.retired++;
 return AE3_OSDR_YIELDED;
}
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_slice(
 AE3_BootOsdRoundtrip *s,AE3_EEMemoryBus *bus,uint32_t budget){
 if(!s||!bus||!s->active||s->cpu.in_delay_slot>1)return AE3_OSDR_INVALID;
 AE3_OsdRoundtripResult k=boundary(s);
 if(k!=AE3_OSDR_YIELDED)return k;
 for(uint32_t i=0;i<budget;i++){
  uint32_t pc=s->cpu.pc,op=0;size_t j=0;
  for(;j<sizeof(code)/sizeof(code[0]);++j)if(code[j].pc==pc)break;
  if(j==sizeof(code)/sizeof(code[0]))return AE3_OSDR_UNSUPPORTED;
  if(ae3_ee_bus_read32(bus,pc,&op)!=AE3_EE_BUS_OK)
    return AE3_OSDR_MEMORY_FAULT;
  if(op!=code[j].word)return AE3_OSDR_UNSUPPORTED;
  AE3_BootOsdRoundtrip n=*s;
  const int delay=n.cpu.in_delay_slot;
  uint32_t dest=delay?n.cpu.branch_target:pc+4u;
  if(delay){n.cpu.in_delay_slot=0;n.cpu.branch_target=0;}
  uint32_t addr=0;unsigned reg=0,size=0;int store=0;
  switch(pc){
  case 0x00435B98:case 0x00435B88:
   if(delay || (pc==0x00435b98 && n.read_count==n.write_count) ||
      (pc==0x00435b88 && n.write_count!=n.read_count))
      return AE3_OSDR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)rd(&n.cpu,31);break;
  case 0x0043FF78:case 0x00440024:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;n.cpu.branch_target=(uint32_t)rd(&n.cpu,31);break;
  case 0x00435B9C:case 0x00435B8C:
   if(!delay)return AE3_OSDR_UNSUPPORTED;
   break;
  case 0x00435B80:case 0x00435B90:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,3,pc==0x00435b80?0x4au:0x4bu);break;
  case 0x0043FF48:case 0x0043FF50:case 0x0043FF58:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,31,pc+8u);n.cpu.in_delay_slot=1;
   n.cpu.branch_target=pc==0x0043ff50?0x00435b90u:0x00435b80u;break;
  case 0x0043FF4C:
   if(!delay||dest!=0x00435b80u)return AE3_OSDR_UNSUPPORTED;
   store=1;size=4;reg=3;addr=(uint32_t)rd(&n.cpu,29)+4u;break;
  case 0x0043FF54:case 0x0043FF5C:
   if(!delay ||dest!=(pc==0x0043ff54?0x00435b90u:0x00435b80u))
      return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,4,rd(&n.cpu,pc==0x0043ff54?16u:29u));break;
  case 0x0043FF2C:case 0x0043FF60:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   addr=(uint32_t)rd(&n.cpu,29)+(pc==0x0043ff2c?0u:4u);
   reg=pc==0x0043ff2c?3u:2u;size=4;break;
  case 0x0043FF64:case 0x0043FF6C:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   addr=(uint32_t)rd(&n.cpu,29)+(pc==0x0043ff64?0x20u:0x10u);
   reg=pc==0x0043ff64?31u:16u;size=8;break;
  case 0x00440014:case 0x00440018:case 0x0044001C:case 0x00440020:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   addr=(uint32_t)rd(&n.cpu,29)+(pc==0x00440014?0x30u:
      pc==0x00440018?0x20u:pc==0x0044001c?0x10u:0u);
   reg=pc==0x00440014?31u:pc==0x00440018?18u:
       pc==0x0044001c?17u:16u;size=8;break;
  case 0x0043FF30:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,2,sx(0xffff0000u));break;
  case 0x0043FF34:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,2,sx((uint32_t)rd(&n.cpu,2)|0x1fffu));break;
  case 0x0043FF38:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,16,sx((uint32_t)rd(&n.cpu,29)+4u));break;
  case 0x0043FF3C:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,3,rd(&n.cpu,3)&rd(&n.cpu,2));break;
  case 0x0043FF40:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,4,rd(&n.cpu,16));break;
  case 0x0043FF44:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,3,sx((uint32_t)rd(&n.cpu,3)|0x2000u));break;
  case 0x0043FF68:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,2,sx((uint32_t)rd(&n.cpu,2)>>13u));break;
  case 0x0043FF70:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,2,rd(&n.cpu,2)&7u);break;
  case 0x0043FF74:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,2,rd(&n.cpu,2)<1u?1u:0u);break;
  case 0x0043FF7C:
   if(!delay||dest!=0x0043ff98u)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,29,sx((uint32_t)rd(&n.cpu,29)+0x30u));break;
  case 0x0043FF98:
   if(delay)return AE3_OSDR_UNSUPPORTED;
   n.cpu.in_delay_slot=1;
   n.cpu.branch_target=rd(&n.cpu,2)==0u?0x00440014u:0x0043ffa0u;break;
  case 0x0043FF9C:
   if(!delay)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,2,sx(0x006a0000u));break;
  case 0x00440028:
   if(!delay||dest!=AE3_OSD_PARENT_RETURN)return AE3_OSDR_UNSUPPORTED;
   wr(&n.cpu,29,sx((uint32_t)rd(&n.cpu,29)+0x40u));break;
  default:return AE3_OSDR_UNSUPPORTED;
  }
  if(size || store){
   if(addr & (size-1u))return AE3_OSDR_MEMORY_FAULT;
   if(store){
    if(ae3_ee_bus_write32(bus,addr,(uint32_t)rd(&n.cpu,reg))!=AE3_EE_BUS_OK)
     return AE3_OSDR_MEMORY_FAULT;
   }else {
    uint8_t raw[8]={0};
    if(ae3_ee_bus_read(bus,addr,raw,size)!=AE3_EE_BUS_OK)
      return AE3_OSDR_MEMORY_FAULT;
    uint64_t v=0;
    for(unsigned q=0;q<size;q++)v|=(uint64_t)raw[q]<<(q*8u);
    wr(&n.cpu,reg,size==4?sx((uint32_t)v):v);
   }
  }
  n.cpu.pc=dest;n.cpu.retired++;*s=n;
  k=boundary(s);
  if(k!=AE3_OSDR_YIELDED)return k;
 }
 return AE3_OSDR_YIELDED;
}