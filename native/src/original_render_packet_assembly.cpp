#include "ae3/original_render_packet_assembly.hpp"
#include <stdexcept>

namespace ae3 {
namespace {
std::uint32_t u32(std::span<const std::uint8_t> bytes,std::size_t offset){
    return std::uint32_t(bytes[offset])|
        (std::uint32_t(bytes[offset+1])<<8)|
        (std::uint32_t(bytes[offset+2])<<16)|
        (std::uint32_t(bytes[offset+3])<<24);
}
constexpr std::uint32_t appendVas[5]{0x3af478,0x3af680,0x3af6f8,0x3af530,0x3af600};
}
std::uintptr_t originalAssemblePacket3af8f0(
    std::uintptr_t gfx,std::span<const std::uint8_t> desc,
    const OriginalPacketAssemblyDependencies& d){
    if(!gfx)throw std::invalid_argument("original retail graphics object not bound");
    if(desc.size()<0x38)
        throw std::invalid_argument("retail render descriptor too short for five packet segments");
    if(!d.allocate3b3ce0||!d.reset3af8b8||!d.bindGraphicsOwner3af920||
       !d.setup3af8a8||!d.appendOriginal)
        throw std::runtime_error("original packet allocator/setup or segment writers unavailable");
    // Retail 0x3AF90C: allocation count4; 0x3AF918: initialize.
    const auto packet=d.allocate3b3ce0(gfx,4);
    if(!packet)throw std::runtime_error("original graphics packet allocation returned null");
    d.reset3af8b8(packet);
    // Retail 0x3AF920 stores the original graphics owner at +0x38.
    d.bindGraphicsOwner3af920(packet,gfx);
    // Retail 0x3AF928 calls 0x3AF8A8(packet,descriptor).
    d.setup3af8a8(packet,desc);
    // Original beqzl on each count skips the corresponding handler.
    for(std::uint8_t i=0;i<5;++i){
        const std::size_t at=0x10+std::size_t(i)*8;
        const auto count=u32(desc,at+4);
        if(!count)continue;
        d.appendOriginal(packet,{appendVas[i],i,u32(desc,at),count});
    }
    return packet;
}
} // namespace ae3
