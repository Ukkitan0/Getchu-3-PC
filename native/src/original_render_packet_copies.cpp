#include "ae3/original_render_packet_copies.hpp"
#include <stdexcept>

namespace ae3 {
OriginalCopiedPacketSegment originalCopyRenderPacketRecords(
    const OriginalPacketAttachment& call,
    std::span<const std::uint8_t> originalBytes){
    const std::size_t stride=call.kind==1&&call.sourceVa==0x3af680?0x20:
                             call.kind==4&&call.sourceVa==0x3af600?0x30:0;
    if(!stride)throw std::invalid_argument("packet group requires original transform, not raw copy");
    constexpr std::size_t cap=64ull*1024*1024;
    if(call.count>cap/stride)
        throw std::length_error("original packet allocation exceeds safe native budget");
    const auto size=std::size_t(call.count)*stride;
    if(originalBytes.size()<size)
        throw std::invalid_argument("authentic original packet source is truncated");
    OriginalCopiedPacketSegment out;
    out.kind=call.kind;
    out.sourceVa=call.sourceVa;
    out.originalRecordCount=call.count;
    out.originalAllocator16ByteUnits=size/16;
    out.bytes.assign(originalBytes.begin(),originalBytes.begin()+size);
    return out;
}
} // namespace ae3
