#include "ae3/original_render_model_streams.hpp"
#include "ae3/original_render_draw_packet.hpp"
#include "ae3/original_render_packet_assembly.hpp"
#include "ae3/original_render_packet_copies.hpp"
#include "ae3/original_render_packet_storage.hpp"
#include "ae3/original_render_transformed_segments.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
namespace {
void req(bool v,const char* msg){if(!v)throw std::runtime_error(msg);}
void put32(std::span<std::uint8_t> bytes,std::size_t offset,std::uint32_t w){
    for(unsigned i=0;i<4;++i)bytes[offset+i]=std::uint8_t(w>>(8*i));
}
std::uint32_t get32(std::span<const std::uint8_t> bytes,std::size_t offset){
    return std::uint32_t(bytes[offset])|std::uint32_t(bytes[offset+1])<<8|
           std::uint32_t(bytes[offset+2])<<16|std::uint32_t(bytes[offset+3])<<24;
}
void run(){
    // Fixtures represent decoded original source structures only, not game art.
    std::array<std::uint8_t,0x80> root{};
    for(const auto& entry:std::array<std::array<std::uint32_t,3>,5>{{
         {{0x04,0x10000,96}},{{0x14,0x11000,96}},{{0x24,0x12000,112}},
         {{0x34,0x13000,80}},{{0x44,0x14000,128}}
    }}){
        put32(root,entry[0],entry[1]);put32(root,entry[0]+4,entry[1]+entry[2]);
    }
    const auto streams=originalDecodeModelStreams27c614(root);
    for(const auto& s:streams.streams)req(s.recordCount==1,"original source model stream lost");
    std::vector<std::uint8_t> sourceGroup2(64),sourceGroup4(48);
    for(std::size_t i=0;i<64;++i)sourceGroup2[i]=std::uint8_t(i*7);
    put32(sourceGroup2,0x30,std::bit_cast<std::uint32_t>(0.7f));
    put32(sourceGroup2,0x34,std::bit_cast<std::uint32_t>(1.2f));
    for(std::size_t i=0;i<48;++i)sourceGroup4[i]=std::uint8_t(i*3);
    OriginalTransformDependencies vu;
    vu.originalVuTransform=[](std::uint8_t kind,std::uint32_t,
                              std::span<const std::uint8_t> src,std::span<std::uint8_t> out){
        req(kind==2,"bridge invoked unknown source VU packet group");
        for(std::size_t i=0;i<src.size();++i)out[i]=src[i];
    };
    // Deterministic test stubs only. Original 0x41D3D8 and 0x41D490
    // must be decompiled before a real game render.
    vu.retailFunction41d3d8=[](float n){return n/2.f;};
    vu.retailFunction41d490=[](float n){return n+2.f;};
    OriginalDrawScratch scratch{};
    auto desc=scratch.packetDescriptor();
    for(unsigned i=0;i<16;++i)desc[i]=std::uint8_t(i+1);
    put32(desc,0x20,0x22000);put32(desc,0x24,1);
    put32(desc,0x30,0x44000);put32(desc,0x34,1);
    OriginalNativeRenderPacketStorage memory(0x1000);
    OriginalRenderContextSlots context{};
    std::size_t nativeOffset=0;
    std::vector<OriginalTransformedPacketSegment> transformed;
    std::vector<OriginalCopiedPacketSegment> copied;
    std::string sequence;
    OriginalPacketAssemblyDependencies assemble;
    assemble.allocate3b3ce0=[&](std::uintptr_t gfx,std::uint32_t count){
        req(gfx==0x7788&&count==4,"retail graphics allocator args wrong");
        sequence+='A';
        nativeOffset=memory.reservePacketHeader();
        return static_cast<std::uintptr_t>(nativeOffset+1);
    };
    auto header=[&](std::uintptr_t id){
        req(id==nativeOffset+1,"retail packet callback used wrong handle");
        return memory.packetHeader(nativeOffset);
    };
    assemble.reset3af8b8=[&](std::uintptr_t id){
        sequence+='R';originalResetPacket3af8b8(header(id));
    };
    assemble.bindGraphicsOwner3af920=[&](std::uintptr_t id,std::uintptr_t gfx){
        sequence+='B';req(gfx==0x7788,"retail graphics owner token truncated");
        originalBindPacketGraphicsOwner(header(id),static_cast<std::uint32_t>(gfx));
    };
    assemble.setup3af8a8=[&](std::uintptr_t id,std::span<const std::uint8_t> descriptor){
        sequence+='S';originalSetupPacket3af8a8(header(id),descriptor);
    };
    assemble.appendOriginal=[&](std::uintptr_t id,const OriginalPacketAttachment& a){
        req(id==nativeOffset+1,"original native packet group attached to wrong header");
        if(a.kind==2){
            req(a.dataToken==0x22000&&a.count==1,"original group2 source ownership wrong");
            transformed.push_back(originalTransformPacketSegment(a,sourceGroup2,vu));
            sequence+='T';
        }else if(a.kind==4){
            req(a.dataToken==0x44000&&a.count==1,"original group4 source ownership wrong");
            copied.push_back(originalCopyRenderPacketRecords(a,sourceGroup4));
            sequence+='C';
        }else throw std::runtime_error("unexpected original packet group in bridge");
    };
    OriginalDrawPacketDependencies draw;
    draw.createPacket3af8f0=[&](auto gfx,auto descriptor){
        return originalAssemblePacket3af8f0(gfx,descriptor,assemble);
    };
    const auto packet=originalBuildDrawPacket280328(context,0x7788,scratch,draw);
    req(packet==nativeOffset+1&&context.source82c==packet,"original render state lost native packet");
    req(sequence=="ARBSTC","original packet state and original attachment order changed");
    const auto finalHeader=memory.packetHeader(nativeOffset);
    req(get32(finalHeader,0x38)==0x7788,"original +0x38 renderer owner missing");
    for(unsigned i=0;i<16;++i)req(finalHeader[i]==i+1,"source first descriptor quadword wrong");
    req(transformed.size()==1&&transformed[0].bytes.size()==64&&
        copied.size()==1&&copied[0].bytes.size()==48,
        "source authentic group segments failed downstream native ownership");
}
}
int main(){try{run();std::cout<<"ORIGINAL_RENDER_MODEL_PACKET_BRIDGE_PASS\n";return 0;}
catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
