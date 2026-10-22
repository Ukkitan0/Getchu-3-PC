#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>

namespace ae3 {
// Recovered 0x3AF8F0: five original descriptor (data,count) pairs
// at (+0x10,+0x14) through (+0x30,+0x34).
// 32-bit data tokens need original-resource resolution before GPU use.
struct OriginalPacketAttachment {
    std::uint32_t sourceVa=0;
    std::uint8_t kind=0;
    std::uint32_t dataToken=0;
    std::uint32_t count=0;
};
struct OriginalPacketAssemblyDependencies {
    // 0x3B3CE0(gfx,4) -> new original packet handle.
    std::function<std::uintptr_t(std::uintptr_t,std::uint32_t)> allocate3b3ce0;
    // 0x3AF8B8(packet) reset original packet storage.
    std::function<void(std::uintptr_t)> reset3af8b8;
    // 0x3AF8A8(packet, descriptor) descriptor initialization.
    std::function<void(std::uintptr_t,std::span<const std::uint8_t>)> setup3af8a8;
    // Original 0x3AF478/0x3AF680/0x3AF6F8/0x3AF530/0x3AF600
    // require real source-recovered implementations.
    std::function<void(std::uintptr_t,const OriginalPacketAttachment&)> appendOriginal;
};
// Original packet creation, branch and group order 0x3AF8F0–0x3AF9C4.
std::uintptr_t originalAssemblePacket3af8f0(
    std::uintptr_t originalGraphics,std::span<const std::uint8_t> sourceDescriptor,
    const OriginalPacketAssemblyDependencies& dependencies);
} // namespace ae3
