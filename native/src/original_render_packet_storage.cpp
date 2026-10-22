#include "ae3/original_render_packet_storage.hpp"
#include <stdexcept>
#include <algorithm>

namespace ae3 {
namespace {
void put32(std::span<std::uint8_t> out,std::size_t pos,std::uint32_t v){
    for(unsigned k=0;k<4;++k)out[pos+k]=std::uint8_t(v>>(8*k));
}
void headerSize(std::span<const std::uint8_t> bytes){
    if(bytes.size()<originalRetailPacketHeaderBytes)
        throw std::invalid_argument("retail packet header below four 16-byte allocation units");
}
}
void originalResetPacket3af8b8(std::span<std::uint8_t> header){
    headerSize(header);
    // Original 0x3AF8B8: sq zero,0(a0) clears first 16 bytes.
    for(std::size_t i=0;i<0x10;++i)header[i]=0;
    // Original 0x3AF8BC–0x3AF8E8: clear 0x38,0x10,0x14,...0x34.
    for(std::size_t i=0x10;i<=0x38;i+=4)put32(header,i,0);
    // Original source DOES NOT initialize +0x3C here.
}
void originalSetupPacket3af8a8(std::span<std::uint8_t> header,
                               std::span<const std::uint8_t> descriptor){
    headerSize(header);
    if(descriptor.size()<0x10)
        throw std::invalid_argument("retail packet descriptor lacks first 16-byte quadword");
    // Retail 0x3AF8A8 lq t7,0(a1); 0x3AF8B4 sq t7,0(a0)
    for(std::size_t i=0;i<0x10;++i)header[i]=descriptor[i];
}
void originalBindPacketGraphicsOwner(std::span<std::uint8_t> header,
                                     std::uint32_t originalGraphicsToken){
    headerSize(header);
    // Retail 0x3AF920: sw s0,0x38(s2); value is original 32-bit
    // context reference, not a truncated host pointer.
    put32(header,0x38,originalGraphicsToken);
}
OriginalNativeRenderPacketStorage::OriginalNativeRenderPacketStorage(std::uint32_t capacity)
  : bytes_(capacity),cursors_(originalRenderArenaAtCapacity(capacity)) {}
std::size_t OriginalNativeRenderPacketStorage::allocatePacketHeader(
    std::uint32_t graphics,std::span<const std::uint8_t> descriptor){
    if(descriptor.size()<16)
        throw std::invalid_argument("invalid original packet descriptor for arena allocation");
    const auto offset=originalReserveRenderPacketFromBack3b3ce0(cursors_,4);
    auto header=std::span<std::uint8_t>(bytes_).subspan(offset,originalRetailPacketHeaderBytes);
    originalResetPacket3af8b8(header);
    originalBindPacketGraphicsOwner(header,graphics);
    originalSetupPacket3af8a8(header,descriptor);
    allocatedHeaders_.push_back(offset);
    return offset;
}
std::span<std::uint8_t> OriginalNativeRenderPacketStorage::packetHeader(std::size_t offset){
    if(std::find(allocatedHeaders_.begin(),allocatedHeaders_.end(),offset)==allocatedHeaders_.end()||
       offset<static_cast<std::size_t>(cursors_.back18)||
       offset>bytes_.size()||bytes_.size()-offset<originalRetailPacketHeaderBytes||
       offset%16)
        throw std::out_of_range("packet header offset not allocated from original rear arena");
    return std::span<std::uint8_t>(bytes_).subspan(offset,originalRetailPacketHeaderBytes);
}
std::span<const std::uint8_t> OriginalNativeRenderPacketStorage::packetHeader(std::size_t offset) const{
    if(std::find(allocatedHeaders_.begin(),allocatedHeaders_.end(),offset)==allocatedHeaders_.end()||
       offset<static_cast<std::size_t>(cursors_.back18)||
       offset>bytes_.size()||bytes_.size()-offset<originalRetailPacketHeaderBytes||
       offset%16)
        throw std::out_of_range("packet header offset not allocated from original rear arena");
    return std::span<const std::uint8_t>(bytes_).subspan(offset,originalRetailPacketHeaderBytes);
}
} // namespace ae3
