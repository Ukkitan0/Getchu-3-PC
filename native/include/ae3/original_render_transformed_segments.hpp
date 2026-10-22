#pragma once
#include "ae3/original_render_packet_assembly.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace ae3 {
// Source-verified original 0x3AF478 / 0x3AF6F8 / 0x3AF530 packet
// field offsets, allocation units and original record strides.
// Group 1/4 pure-copies belong to original_render_packet_copies.hpp.
struct OriginalTransformGroupLayout {
    std::uint8_t kind=0;
    std::uint32_t sourceVa=0;
    std::uint8_t destinationCountOffset=0;
    std::uint8_t destinationDataOffset=0;
    std::uint8_t allocatorUnitsPerRecord=0;
    std::uint8_t bytesPerRecord=0;
};
OriginalTransformGroupLayout originalTransformGroupLayout(std::uint8_t kind);

struct OriginalTransformedPacketSegment {
    OriginalTransformGroupLayout layout;
    std::uint32_t originalRecordCount=0;
    std::vector<std::uint8_t> bytes; // native-owned transformed records
};

struct OriginalTransformDependencies {
    // Actual 0x3BDF08/VU quadword copy and subsequent packed/VU writes
    // MUST be source-equivalent; never substitute a memcpy approximation.
    std::function<void(std::uint8_t kind,std::uint32_t recordIndex,
                       std::span<const std::uint8_t> source,
                       std::span<std::uint8_t> destination)> originalVuTransform;
    // Retail 0x41D3D8 and 0x41D490; these can be supplied by source-
    // recovered native math when available. Required for group 2.
    std::function<float(float)> retailFunction41d3d8;
    std::function<float(float)> retailFunction41d490;
};

// Group 2: source 0x3AF7AC–0x3AF858 scalar instructions are ported
// over the VU-produced record. The authentic libm and VU operations
// remain mandatory callbacks. Groups 0/3 are source-layout accurate
// and require their complete original VU transforms.
OriginalTransformedPacketSegment originalTransformPacketSegment(
    const OriginalPacketAttachment& call,
    std::span<const std::uint8_t> originalSource,
    const OriginalTransformDependencies& functions);
} // namespace ae3
