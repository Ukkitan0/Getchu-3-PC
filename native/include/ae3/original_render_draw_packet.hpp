#pragma once
#include "ae3/original_render_context.hpp"
#include "ae3/original_render_resource.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>

namespace ae3 {
// Native scratch descriptor for the real retail 0x280364 and 0x2803B4
// render-resource paths. 0x390 bytes is the retail stack allocation;
// +0x330 is the descriptor passed to 0x3AF8F0. This is NOT a generic
// mesh, emulator stack, or a PS2 rendering packet implementation.
struct OriginalDrawScratch {
    static constexpr std::size_t bytes = 0x390;
    static constexpr std::size_t packetDescriptorOffset = 0x330;
    std::array<std::uint8_t,bytes> data{};
    std::span<std::uint8_t> packetDescriptor() noexcept {
        return std::span<std::uint8_t>(data).subspan(packetDescriptorOffset);
    }
    std::span<const std::uint8_t> packetDescriptor() const noexcept {
        return std::span<const std::uint8_t>(data).subspan(packetDescriptorOffset);
    }
};
struct OriginalDrawPacketDependencies {
    // Retail descriptor initialization (VU-specific stores pending).
    std::function<void(OriginalDrawScratch&)> initialize51c9b0;
    std::function<void(std::uintptr_t,OriginalDrawScratch&)> convertModel27c614;
    std::function<void(std::uintptr_t,OriginalDrawScratch&)> convertModel27c520;
    // Actor flag00 true: original 0x26A8F4 and 0x28040C–0x2804C8
    // VU-assisted path is mandatory and is not substituted.
    std::function<void(OriginalRenderResourceState&,OriginalDrawScratch&)> fillSpecialModelDescriptor;
    // Original 0x3AF8F0(globalGraphics, scratch+0x330).
    std::function<std::uintptr_t(std::uintptr_t,std::span<const std::uint8_t>)> createPacket3af8f0;
};
// Retail 0x280328: create packet, then set context+0x82C via 0x3B76E8.
std::uintptr_t originalBuildDrawPacket280328(
    OriginalRenderContextSlots&,std::uintptr_t globalGraphics,
    const OriginalDrawScratch&,const OriginalDrawPacketDependencies&);
// Retail 0x280364: initialize, original 0x27C614, then 0x280328.
std::uintptr_t originalBuildStaticDrawPacket280364(
    OriginalRenderContextSlots&,std::uintptr_t originalModelRoot28,
    std::uintptr_t globalGraphics,const OriginalDrawPacketDependencies&);
// Retail 0x2803B4 call graph, with explicitly unported VU inner body.
std::uintptr_t originalBuildActorDrawPacket2803b4(
    OriginalRenderContextSlots&,OriginalRenderResourceOwner&,
    std::uintptr_t originalModelRoot28,std::uint32_t originalActorWord118,
    std::uintptr_t globalGraphics,const OriginalDrawPacketDependencies&);
} // namespace ae3
