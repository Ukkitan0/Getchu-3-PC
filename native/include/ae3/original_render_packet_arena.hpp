#pragma once
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Native cursor view for original renderer scratch allocator state.
// Source +0x14 is a forward cursor and +0x18 is a backward cursor.
// Both original functions account in 16-byte allocation units.
// Values are offsets into HOST-owned graphics buffers, not PS2 addresses.
struct OriginalRenderArenaCursors {
    std::uint32_t front14=0;
    std::uint32_t back18=0;
    std::uint32_t capacity=0;
};
OriginalRenderArenaCursors originalRenderArenaAtCapacity(std::uint32_t bytes);
// 0x3B3CE0: back=(back-units*16)&~15; return new back.
// Guards reject overflow/overlap rather than corrupting native memory.
std::uint32_t originalReserveRenderPacketFromBack3b3ce0(
    OriginalRenderArenaCursors& arena,std::uint32_t units);
// 0x3B3D00: old=front; front=(front+units*16+15)&~15;
// Returns original OLD front pointer.
std::uint32_t originalReserveRenderPacketFromFront3b3d00(
    OriginalRenderArenaCursors& arena,std::uint32_t units);
} // namespace ae3
