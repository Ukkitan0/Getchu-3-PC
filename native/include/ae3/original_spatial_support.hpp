#pragma once
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstdint>
#include <span>

namespace ae3 {
// Source +4 grid metadata and original u16 header table at +0x50.
// VU0 packed-vector extraction occurs before these resolved inputs.
struct OriginalSpatialGridFields {
    float originX00=0,originY04=0;
    float stepX10=1,stepY14=1;
    std::int32_t dimensionX18=0,dimensionY1c=0;
    std::span<const std::uint16_t> cellOffsets50;
    std::span<const std::uint16_t> hitBuffer08;
};
OriginalSpatialGridHits originalSpatialGridLookup_4e3c14(
    const OriginalSpatialGridFields&,float resolvedX,float resolvedY);
const OriginalSpatialRecord70* originalSpatialNearestFromGrid_21d898(
    std::span<const OriginalSpatialRecord70>,
    const OriginalSpatialGridFields&,float resolvedX,float resolvedY,
    OriginalSpatialEngineCalls&);

// Actual VU0-produced f1/f0 per plane are inputs, not invented geometry.
struct OriginalSpatialPlaneComparison {
    float originalVU_f1=0;
    float originalVU_f0=0;
};
bool originalSpatialPlaneEligible_21b788(
    std::span<const OriginalSpatialPlaneComparison>,float originalF12=0.0f) noexcept;
float originalSpatialReciprocalPlaneCount_21b468(std::uint16_t count);

// Original 0x21B178 / 0x21B180 child record point array.
struct OriginalSpatialPointArray {
    std::span<const std::array<std::byte,16>> sourceVectors;
};
std::uint32_t originalSpatialPointCount_21b178(const OriginalSpatialPointArray&) noexcept;
std::array<std::byte,16> originalSpatialPoint_21b180(
    const OriginalSpatialPointArray&,std::size_t);
class OriginalPolylineMetric {
public:
    virtual ~OriginalPolylineMetric()=default;
    virtual float call_original_VU0_consecutive_distance(
        const std::array<std::byte,16>& first,
        const std::array<std::byte,16>& next)=0;
};
float originalSpatialPolylineLength_21b194(
    const OriginalSpatialPointArray&,OriginalPolylineMetric&);

// Native ownership of actual game subobjects, not 32-bit guest memory.
struct OriginalSpatialChildOwnership {
    void* originalAllocatedField04=nullptr;
};
struct OriginalSpatialRecordOwnership {
    std::array<OriginalSpatialChildOwnership*,3> childPointers_04_08_0c{};
    void* subobjectAt20=nullptr;
};
class OriginalSpatialRecordReleaseCalls {
public:
    virtual ~OriginalSpatialRecordReleaseCalls()=default;
    virtual void call_original_41a3d8(void*)=0;
    virtual void call_original_41a3b0(void*)=0;
};
void originalSpatialChildDispose_21b14c(
    OriginalSpatialChildOwnership&,OriginalSpatialRecordReleaseCalls&);
void originalSpatialRecordDestroy_21b384(
    OriginalSpatialRecordOwnership&,OriginalSpatialRecordReleaseCalls&);
void originalSpatialSubobjectNoop_21afb0(void*) noexcept;
} // namespace ae3
