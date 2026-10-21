#pragma once
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Original 0x57C8EC emits a low 64-bit packed vector in EE $v0.
// A full, authentic geometry implementation is still required for the
// fallback VU0 paths. Returning 64-bit SOURCE BITS preserves identity.
enum class OriginalActorTargetBranch_57c8ec {
    OriginalThreeVertexTransform_57c98c,
    OriginalVariantVectorPassThrough_57c944,
    OriginalFallbackGeometry_57ca68,
};
struct OriginalActorTargetResult_57c8ec {
    OriginalActorTargetBranch_57c8ec branch;
    std::uint64_t originalVectorLow64 = 0;
};

class OriginalActorTargetGeometryCalls_57c8ec {
public:
    virtual ~OriginalActorTargetGeometryCalls_57c8ec()=default;
    // Retail 0x57C930: original 0x21B788 with f12 from 0x708A60.
    virtual bool call_original_21b788_eligibility(
        const OriginalSpatialRecord70& target,float originalF12)=0;
    // Original 0x57C97C: geometry fallback after failed 0x21B788.
    virtual std::uint64_t call_original_57ca68_geometry(
        const OriginalSpatialRecord70& target)=0;
    // Original 0x57C98C..0x57CA60: three 0x21B458 vectors feed its
    // FPU/VU0 calculations; unresolved transform must remain explicit.
    virtual std::uint64_t call_original_57c98c_three_vertex_transform(
        const OriginalSpatialRecord70& target,
        const std::array<std::array<std::byte,16>,3>& vertices)=0;
};

OriginalActorTargetResult_57c8ec originalActorTargetVector_57c8ec(
    const OriginalSpatialRecord70& target,
    bool originalControllerVariantByte30,
    const std::array<std::byte,16>& originalControllerVector20,
    OriginalActorTargetGeometryCalls_57c8ec& original);
} // namespace ae3
