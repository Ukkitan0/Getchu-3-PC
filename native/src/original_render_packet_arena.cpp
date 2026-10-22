#include "ae3/original_render_packet_arena.hpp"
#include <stdexcept>

namespace ae3 {
namespace {
void verify(const OriginalRenderArenaCursors& a){
    if(a.front14>a.back18||a.back18>a.capacity)
        throw std::invalid_argument("original render allocator cursors corrupted");
}
}
OriginalRenderArenaCursors originalRenderArenaAtCapacity(std::uint32_t n){
    if(n==0||n%16)
        throw std::invalid_argument("original renderer capacity must be 16-byte aligned");
    return {0,n,n};
}
std::uint32_t originalReserveRenderPacketFromBack3b3ce0(
    OriginalRenderArenaCursors& a,std::uint32_t units){
    verify(a);
    const std::uint64_t requested=std::uint64_t(units)*16;
    if(requested>a.back18)
        throw std::length_error("original 0x3B3CE0 packet arena underflow");
    // Original subu then AND -16.
    const std::uint32_t next=(a.back18-static_cast<std::uint32_t>(requested))&~std::uint32_t(15);
    if(next<a.front14)
        throw std::length_error("original render packet rear allocation collided with front");
    a.back18=next;
    return next;
}
std::uint32_t originalReserveRenderPacketFromFront3b3d00(
    OriginalRenderArenaCursors& a,std::uint32_t units){
    verify(a);
    const std::uint64_t next64=std::uint64_t(a.front14)+std::uint64_t(units)*16+15;
    const auto next=next64&~std::uint64_t(15);
    if(next>a.back18)
        throw std::length_error("original 0x3B3D00 packet arena overflow or collision");
    const auto old=a.front14;
    a.front14=static_cast<std::uint32_t>(next);
    return old;
}
} // namespace ae3
