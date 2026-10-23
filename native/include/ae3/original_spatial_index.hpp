#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// SCUS_975.01 uses 0x70-byte records (0x21D884, 0x21D90C).
// Unknown fields stay opaque; no emulated PlayStation CPU/memory ABI.
struct alignas(16) OriginalSpatialRecord70 {
    std::array<std::byte,0x70> originalFields{};
};
static_assert(sizeof(OriginalSpatialRecord70)==0x70);

const OriginalSpatialRecord70* originalSpatialRecord_21d884(
    std::span<const OriginalSpatialRecord70> records,std::uint16_t index);
std::array<std::byte,16> originalSpatialVector_21b458(
    const OriginalSpatialRecord70&,std::size_t index);
const OriginalSpatialRecord70* originalSpatialByEntry_21b404(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::uint16_t> headerEntries,std::size_t index);
std::uint16_t originalSpatialHeaderHalfword_21b444(
    std::span<const std::uint16_t> headerAt0A,std::size_t index);

struct OriginalSpatialWordData {
    std::array<std::uint32_t,8> originalWords{};
};
std::uint32_t originalSpatialWordAt04_21b4e0(
    const OriginalSpatialWordData&,std::size_t index);
std::uint32_t originalSpatialWordAt10_21b4f0(
    const OriginalSpatialWordData&,std::size_t index);
std::uint32_t originalSpatialWordAt1C_21b500(
    const OriginalSpatialWordData&) noexcept;

// 0x4E3C14 still needs porting. This is its already-resolved u16 hit list.
struct OriginalSpatialGridHits {
    std::span<const std::uint16_t> originalRecordIndices;
};
class OriginalSpatialEngineCalls {
public:
    virtual ~OriginalSpatialEngineCalls()=default;
    // Original 0x21B788 VU0 eligibility now has a native translation
    // in original_vu0_geometry.cpp; this interface also supports callers
    // that must obtain the source decision from an external game owner.
    virtual bool call_original_21b788_eligibility(
        const OriginalSpatialRecord70& record)=0;
    // Source 0x21B468 centroid and 0x21B234 VU0 XYZ distance are
    // recovered in original_vu0_geometry.cpp. The caller must supply
    // genuine source polygon vertices, never synthetic host geometry.
    virtual float call_original_21b468_and_VU_distance(
        const OriginalSpatialRecord70& record)=0;
};
// Real original 0x21D898 grid-hit nearest-selection control flow.
const OriginalSpatialRecord70* originalSpatialNearest_21d898(
    std::span<const OriginalSpatialRecord70> records,
    OriginalSpatialGridHits gridHits,
    OriginalSpatialEngineCalls& original);
// Real 0x21DA2C full contiguous-registry nearest-selection control flow.
const OriginalSpatialRecord70* originalSpatialNearestAll_21da2c(
    std::span<const OriginalSpatialRecord70> registry,
    OriginalSpatialEngineCalls& original);

// 0x21D7D8 destroys the same original 0x70 record registry.
// Native pointer ownership and original field offsets are separated.
struct OriginalSpatialRegistryOwner {
    void* ownedBlock00=nullptr;
    void* originalRecordAllocation04=nullptr;
    std::int32_t selectionIndex0c=0;
    void* auxiliaryAllocation10=nullptr;
    std::int32_t recordCount14=0;
    std::span<OriginalSpatialRecord70> records;
};
class OriginalSpatialDestroyCalls {
public:
    virtual ~OriginalSpatialDestroyCalls()=default;
    virtual void call_original_41a3b0_free(void*)=0;
    virtual void call_original_21b384_record_destructor(OriginalSpatialRecord70&)=0;
    virtual void call_original_36ede8_release(void*)=0;
};
void originalSpatialRegistryDestroy_21d7d8(
    OriginalSpatialRegistryOwner&,OriginalSpatialDestroyCalls&);
} // namespace ae3
