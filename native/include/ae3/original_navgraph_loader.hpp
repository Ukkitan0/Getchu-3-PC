#pragma once
#include "ae3/archive.hpp"
#include "ae3/original_stage_spatial_loader.hpp"
#include "ae3/original_nav_type1_route.hpp"
#include "ae3/original_nav_graph_search.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// Original source identifiers at 0x6E7470 / 0x6E7480 / 0x6E7490.
// Original 0x4F0FBC maps first name -> slot 0, second -> slot 1,
// and all other strings to slot 2 (last strcmp result is not tested).
std::size_t originalNavGraphSlot_4f0fbc(std::string_view name) noexcept;

// Host-native asset codec: SZ is u32 uncompressed size followed by
// raw DEFLATE (the original disc has optional trailing bytes).
// Uses zlib as an ordinary host library, NOT the game CPU runtime.
std::vector<std::uint8_t> originalNavInflateSZ(
    std::span<const std::uint8_t> originalCompressed);

// Original file handles: real raw .nav data as stored in VFI DATA.BIN,
// after SZ and PCK container decoding; there are no invented vertices.
struct OriginalNavSource {
    std::string originalFilename;
    std::size_t originalSlot=0;
    std::vector<std::uint8_t> originalNavBytes;
};

// Parses the retail navigation.pck.sz from an existing native Archive
// (DATA.BIN or owner-provided original ISO). No gameplay replacement.
std::vector<OriginalNavSource> originalNavReadPackage(
    std::span<const std::uint8_t> originalSZ);
std::vector<OriginalNavSource> originalNavReadPackage(
    Archive& originalArchive,const VfiEntry& originalEntry);

// Requires the original source-allocation VA and owning NavGraph address.
// No fabricated guest pointer is generated. Uses already recovered
// original 0x21D46C / 0x21B254 stage->nav record constructors.
OriginalStageSpatialLoaded originalNavLoadGraph_4f0a5c(
    const OriginalNavSource& source,
    std::uint32_t originalSourceAllocationVA,
    std::uint32_t originalGraphOwnerVA);
} // namespace ae3
