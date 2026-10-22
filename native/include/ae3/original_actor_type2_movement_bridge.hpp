#pragma once
#include "ae3/original_actor_distance_gate.hpp"
#include "ae3/original_actor_type2_vu0_target.hpp"
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
namespace ae3 {
// Only the source-proven 0x57BCDC/57BD34/57BD80/57BDA4 type-2
// branch is recovered. Other original actor movement paths remain pending.
enum class OriginalActorType2BridgeOutcome_57bcb8 {
    OriginalDistanceGateReset_57bd3c,
    OriginalType2TargetWritten_57bed4,
    OriginalOtherBranchUnrecovered
};
struct OriginalActorType2BridgeResult_57bcb8 {
    OriginalActorType2BridgeOutcome_57bcb8 outcome;
    OriginalActorDistanceGate_57bcdc originalGate;
    const OriginalSpatialRecord70* originalTargetPolygonAt50=nullptr;
};
OriginalActorType2BridgeResult_57bcb8 originalActorType2MovementBridge_57bcb8(
    bool originalControllerVariant30,
    bool originalHasMultipleAt1c,
    std::uint16_t originalSourceActionKind,
    const std::array<std::byte,16>& originalControllerVector20,
    const std::array<std::byte,16>& originalOwner14Vector40,
    std::span<const OriginalSpatialRecord70> originalSourceRecords,
    std::span<const std::uint32_t> originalRoutePolygonIndices,
    OriginalGameRandom_36f3c0& originalSharedGameRng,
    std::uint32_t& originalControllerField08,
    std::uint32_t& originalControllerField0c,
    std::array<std::byte,16>& originalControllerVector40,
    const OriginalSpatialRecord70*& originalControllerTarget50);
} // namespace ae3
