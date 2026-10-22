#include "ae3/original_render_packet_storage.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <span>
namespace {
void req(bool a,const char* b){if(!a)throw std::runtime_error(b);}
std::uint32_t get32(std::span<const std::uint8_t> b,std::size_t p){
    return std::uint32_t(b[p]) | std::uint32_t(b[p+1])<<8|
           std::uint32_t(b[p+2])<<16|std::uint32_t(b[p+3])<<24;
}
void run(){
    std::array<std::uint8_t,64> bytes{};bytes.fill(0x77);
    ae3::originalResetPacket3af8b8(bytes);
    for(std::size_t i=0;i<60;++i)req(bytes[i]==0,"retail reset did not clear exact quadword/field span");
    for(std::size_t i=60;i<64;++i)req(bytes[i]==0x77,"retail source did not initialize +0x3C word");
    std::array<std::uint8_t,16> descriptor{};
    for(std::size_t i=0;i<descriptor.size();++i)descriptor[i]=std::uint8_t(i+9);
    ae3::originalSetupPacket3af8a8(bytes,descriptor);
    for(std::size_t i=0;i<16;++i)req(bytes[i]==descriptor[i],"source LQ/SQ first quadword mis-copied");
    ae3::originalBindPacketGraphicsOwner(bytes,0x12345678);
    req(get32(bytes,0x38)==0x12345678,"original packet owner +0x38 set incorrectly");
    ae3::OriginalNativeRenderPacketStorage arena(0x200);
    const auto first=arena.allocatePacketHeader(0x12345678,descriptor);
    req(first==0x1c0&&arena.cursors().back18==0x1c0,
        "original 0x3B3CE0 arena packet header allocation incorrect");
    auto a=arena.packetHeader(first);
    req(a[0]==9&&a[15]==24&&get32(a,0x38)==0x12345678,
        "original graphics packet header initial content incorrect");
    const auto second=arena.allocatePacketHeader(0xAABBCCDD,descriptor);
    req(second==0x180&&arena.packetHeader(first)[0]==9&&
        get32(arena.packetHeader(second),0x38)==0xAABBCCDD,
        "second source packet overwritten first or wrong owner token");
    bool rejected=false;
    try{arena.packetHeader(first+0x10);}catch(const std::out_of_range&){rejected=true;}
    req(rejected,"original packet storage accepted a misaligned interior packet handle");
    rejected=false;
    try{arena.packetHeader(0x140);}catch(const std::out_of_range&){rejected=true;}
    req(rejected,"native original packet storage accepted unallocated address");
    rejected=false;
    try{arena.allocatePacketHeader(0,std::span(descriptor).first(12));}
    catch(const std::invalid_argument&){rejected=true;}
    req(rejected,"original 16-byte LQ input underrun accepted");
    rejected=false;
    try{ae3::OriginalNativeRenderPacketStorage(7);}catch(const std::invalid_argument&){rejected=true;}
    req(rejected,"unaligned source packet arena accepted");
}
}
int main(){try{run();std::cout<<"ORIGINAL_RENDER_PACKET_STORAGE_PASS\n";return 0;}
catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
