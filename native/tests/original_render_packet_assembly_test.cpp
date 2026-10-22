#include "ae3/original_render_packet_assembly.hpp"
#include "ae3/original_render_draw_packet.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
namespace {
void require(bool v,const char* s){if(!v)throw std::runtime_error(s);}
void store32(std::span<std::uint8_t> b,std::size_t offset,std::uint32_t v){
    for(unsigned k=0;k<4;++k)b[offset+k]=std::uint8_t(v>>(8*k));
}
void run(){
    OriginalDrawScratch scratch{};
    auto desc=scratch.packetDescriptor();
    for(unsigned i=0;i<5;++i){
        store32(desc,0x10+i*8,0x1000+i*0x100);
        store32(desc,0x14+i*8,i%2==0?i+1:0);
    }
    std::string order;
    std::vector<OriginalPacketAttachment> actual;
    OriginalPacketAssemblyDependencies deps;
    deps.allocate3b3ce0=[&](std::uintptr_t gfx,std::uint32_t n){
        require(gfx==0x6789&&n==4,"packet allocation source arguments not preserved");
        order+='A';return std::uintptr_t(0xA123);
    };
    deps.reset3af8b8=[&](std::uintptr_t handle){
        require(handle==0xA123,"wrong packet to original initialization");order+='R';
    };
    deps.bindGraphicsOwner3af920=[&](std::uintptr_t packet,std::uintptr_t gfx){
        require(packet==0xA123&&gfx==0x6789,"original packet owner source argument changed");
        order+='B';
    };
    deps.setup3af8a8=[&](std::uintptr_t h,std::span<const std::uint8_t> span){
        require(h==0xA123&&span.data()==desc.data(),"source descriptor pointer not forwarded");
        order+='S';
    };
    deps.appendOriginal=[&](std::uintptr_t h,const OriginalPacketAttachment& a){
        require(h==0xA123,"original packet changed between material segments");
        order+='D';actual.push_back(a);
    };
    auto result=originalAssemblePacket3af8f0(0x6789,desc,deps);
    require(result==0xA123&&order=="ARBSDDD"&&actual.size()==3,
            "retail 3AF8F0 packet assembly or conditional attachments wrong");
    require(actual[0].kind==0&&actual[0].sourceVa==0x3af478&&
            actual[0].dataToken==0x1000&&actual[0].count==1,
            "first original packet segment data incorrect");
    require(actual[1].kind==2&&actual[1].sourceVa==0x3af6f8&&
            actual[1].dataToken==0x1200&&actual[1].count==3,
            "third original packet segment data incorrect");
    require(actual[2].kind==4&&actual[2].sourceVa==0x3af600&&
            actual[2].dataToken==0x1400&&actual[2].count==5,
            "fifth original packet segment data incorrect");
    OriginalRenderContextSlots context{};
    OriginalDrawPacketDependencies wrapper;
    wrapper.createPacket3af8f0=[&](auto gfx,auto descriptor){
        return originalAssemblePacket3af8f0(gfx,descriptor,deps);
    };
    order.clear();actual.clear();
    auto built=originalBuildDrawPacket280328(context,0x6789,scratch,wrapper);
    require(built==0xA123&&context.source82c==built&&order=="ARBSDDD",
            "original packet builder did not connect to native context+0x82C");
    order.clear();actual.clear();
    for(unsigned i=0;i<5;++i)store32(desc,0x14+i*8,0);
    originalAssemblePacket3af8f0(0x6789,desc,deps);
    require(order=="ARBS"&&actual.empty(),"zero-count retail segments must skip append calls");
    bool threw=false;
    try{originalAssemblePacket3af8f0(0x6789,desc,{});}
    catch(const std::runtime_error&){threw=true;}
    require(threw,"absent original packet callbacks must fail explicitly");
    threw=false;
    try{originalAssemblePacket3af8f0(0x6789,desc.first(0x20),deps);}
    catch(const std::invalid_argument&){threw=true;}
    require(threw,"truncated original render descriptor accepted");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_PACKET_ASSEMBLY_PASS: genuine_3AF8F0 five_conditional_segments original_order and_context_binding\n";return 0;}
    catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
