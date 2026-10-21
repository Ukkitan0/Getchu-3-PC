#pragma once
#include "ae3/original_stage_spatial_loader.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace ae3 {
// Original descriptor tables consumed at 0x21B3F8/0x21B404.
// All original retail adjacency arrays have 0..3 neighbor indices.
class OriginalNavGraphTopology {
public:
    explicit OriginalNavGraphTopology(std::span<const std::byte> originalNavBytes);
    std::size_t size() const noexcept {return links_.size();}
    std::span<const std::uint16_t> links(std::size_t index) const;
    std::uint16_t degree_21b3f8(std::size_t recordIndex) const;
    const OriginalSpatialRecord70* neighbor_21b404(
        std::span<const OriginalSpatialRecord70> originalRecords,
        std::size_t fromRecord,std::size_t linkIndex) const;
private:
    std::vector<std::vector<std::uint16_t>> links_;
};

// Represents the original NavGraph resource plus its decoded 0x70 records.
// Runtime source/graph original VAs MUST be supplied by original game's
// native owner; this component does not fabricate virtual addresses.
struct OriginalNavGraphOwned {
    OriginalStageSpatialLoaded records;
    OriginalNavGraphTopology topology;
};
OriginalNavGraphOwned originalNavGraphConstruct_4f0b14(
    std::span<const std::uint8_t> originalNavBytes,
    std::uint32_t originalSourceRuntimeVA,
    std::uint32_t originalGraphOwnerVA);

class OriginalNavGraphWalkCalls {
public:
    virtual ~OriginalNavGraphWalkCalls()=default;
    // This genuine downstream original routine is NOT decompiled yet.
    virtual void call_original_21db64(
        const OriginalSpatialRecord70& originalCurrentRecord)=0;
};

// Original 0x21E648: attempt grid nearest 0x21D898; if not found,
// fall back to entire-registry nearest 0x21DA2C. For the result,
// 0x21DB64 is called once for the selected polygon and once per link,
// as each subsequent 0x21B404 returns that neighbor's record.
// Returns selected record only as a host API convenience; the actual
// retail method's return register has no documented return contract.
const OriginalSpatialRecord70* originalNavVisitAdjacent_21e648(
    const OriginalNavGraphOwned& originalGraph,
    const OriginalSpatialGridFields& originalGrid,
    float originalGridX,float originalGridY,
    const OriginalVU0Vec4& originalQuery,
    OriginalNavGraphWalkCalls& originalDependency);

// Three original pointer slots at 0x647B80, indexed by 0x4F0FBC.
// Ownership of pointers remains with caller's real native stage lifetime.
struct OriginalNavGraphSlots {
    std::array<const OriginalNavGraphOwned*,3> originalSlots_647b80{};
};
std::size_t originalNavSlotFromName_4f0fbc(std::string_view) noexcept;
void originalNavBindSlot_4f0b00(
    OriginalNavGraphSlots&,std::string_view originalName,
    const OriginalNavGraphOwned& originalGraph) noexcept;
const OriginalNavGraphOwned* originalNavLookupSlot_647b80(
    const OriginalNavGraphSlots&,std::size_t originalSlot) noexcept;
} // namespace ae3
