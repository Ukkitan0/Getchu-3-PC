#include "ae3/original_render_draw_packet.hpp"
#include <stdexcept>

namespace ae3 {
namespace {
void ensure(bool condition,const char* explanation) {
    if(!condition)throw std::runtime_error(explanation);
}
void storeWordLittleEndian(OriginalDrawScratch& scratch,std::uint32_t word) {
    // 0x2803F0: sw word118,0(sp), in the jal delay slot.
    for(unsigned i=0;i!=4;++i)
        scratch.data[i]=static_cast<std::uint8_t>(word>>(i*8));
}
}
std::uintptr_t originalBuildDrawPacket280328(
    OriginalRenderContextSlots& destination,std::uintptr_t globalGraphics,
    const OriginalDrawScratch& scratch,const OriginalDrawPacketDependencies& deps) {
    ensure(static_cast<bool>(deps.createPacket3af8f0),"retail 0x3AF8F0 graphics packet builder missing");
    ensure(globalGraphics!=0,"retail global graphics context pointer not bound");
    // 0x280334 adds 0x330 to a2, 0x280344 calls 0x3AF8F0,
    // then 0x28035C tail-jumps to context setter 0x3B76E8.
    const auto packet=deps.createPacket3af8f0(globalGraphics,scratch.packetDescriptor());
    originalSetRenderSource82c(destination,packet);
    return packet;
}
std::uintptr_t originalBuildStaticDrawPacket280364(
    OriginalRenderContextSlots& destination,std::uintptr_t originalModelRoot28,
    std::uintptr_t globalGraphics,const OriginalDrawPacketDependencies& deps) {
    ensure(static_cast<bool>(deps.initialize51c9b0),"retail 0x51C9B0 model descriptor initializer missing");
    ensure(static_cast<bool>(deps.convertModel27c614),"retail 0x27C614 model data converter missing");
    ensure(static_cast<bool>(deps.createPacket3af8f0),"retail 0x3AF8F0 packet creation missing");
    ensure(originalModelRoot28!=0,"retail stage model root +0x28 unavailable");
    OriginalDrawScratch scratch;
    deps.initialize51c9b0(scratch);
    // 0x2802F8 loads original model root+0x28 and calls 0x27C614.
    deps.convertModel27c614(originalModelRoot28,scratch);
    return originalBuildDrawPacket280328(destination,globalGraphics,scratch,deps);
}
std::uintptr_t originalBuildActorDrawPacket2803b4(
    OriginalRenderContextSlots& destination,OriginalRenderResourceOwner& actor,
    std::uintptr_t originalModelRoot28,std::uint32_t originalActorWord118,
    std::uintptr_t globalGraphics,const OriginalDrawPacketDependencies& deps) {
    ensure(static_cast<bool>(deps.initialize51c9b0),"retail 0x51C9B0 actor descriptor initializer missing");
    ensure(static_cast<bool>(deps.convertModel27c520),"retail 0x27C520 actor model converter missing");
    ensure(static_cast<bool>(deps.createPacket3af8f0),"retail 0x3AF8F0 actor packet creation missing");
    ensure(originalModelRoot28!=0,"retail actor model root +0x28 unavailable");
    OriginalDrawScratch scratch;
    deps.initialize51c9b0(scratch);
    // 0x2803D4 initializes scratch; 0x2803F0 stores actor+0x118 to sp.
    storeWordLittleEndian(scratch,originalActorWord118);
    deps.convertModel27c520(originalModelRoot28,scratch);
    if(originalReadActorFlag00(actor)) {
        ensure(static_cast<bool>(deps.fillSpecialModelDescriptor),
               "original 0x26A8F4 / 0x28040C VU descriptor dependency missing");
        auto& inner=originalEnsureRenderActorResource(actor);
        deps.fillSpecialModelDescriptor(inner,scratch);
    }
    return originalBuildDrawPacket280328(destination,globalGraphics,scratch,deps);
}
} // namespace ae3
