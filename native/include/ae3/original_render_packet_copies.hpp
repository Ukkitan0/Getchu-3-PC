#pragma once
#include "ae3/original_render_packet_assembly.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Native transfer of original renderer packet data. NOT a model decoder;
// authentic bytes must be supplied by the Assets subsystem.
struct OriginalCopiedPacketSegment {
    std::uint8_t kind=0;
    std::uint32_t sourceVa=0;
    std::uint32_t originalRecordCount=0;
    std::size_t originalAllocator16ByteUnits=0;
    std::vector<std::uint8_t> bytes;
};
// Retail 0x3AF680 (kind 1) allocates 2*count 16-byte units and
// copies 0x20*count bytes through 0x42B16C (native memcpy equivalent).
// Retail 0x3AF600 (kind 4) allocates 3*count units, copies 0x30*count.
OriginalCopiedPacketSegment originalCopyRenderPacketRecords(
    const OriginalPacketAttachment& originalCall,
    std::span<const std::uint8_t> originalResourceBytes);
} // namespace ae3
