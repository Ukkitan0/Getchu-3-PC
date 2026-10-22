#pragma once
#include "ae3/original_render_packet_arena.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
namespace ae3 {
constexpr std::size_t originalRetailPacketHeaderBytes=0x40;
void originalResetPacket3af8b8(std::span<std::uint8_t> header);
void originalSetupPacket3af8a8(std::span<std::uint8_t> header,
                               std::span<const std::uint8_t> descriptor);
void originalBindPacketGraphicsOwner(std::span<std::uint8_t> header,
                                     std::uint32_t originalGraphicsToken);
class OriginalNativeRenderPacketStorage {
public:
    explicit OriginalNativeRenderPacketStorage(std::uint32_t byteCapacity);
    std::size_t allocatePacketHeader(std::uint32_t originalGraphicsToken,
                                     std::span<const std::uint8_t> originalDescriptor);
    std::span<std::uint8_t> packetHeader(std::size_t offset);
    std::span<const std::uint8_t> packetHeader(std::size_t offset) const;
    const OriginalRenderArenaCursors& cursors() const noexcept{return cursors_;}
private:
    std::vector<std::uint8_t> bytes_;
    OriginalRenderArenaCursors cursors_;
    std::vector<std::size_t> allocatedHeaders_;
};
} // namespace ae3
