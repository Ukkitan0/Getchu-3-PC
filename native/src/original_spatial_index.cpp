#include "ae3/original_spatial_index.hpp"
#include <cfloat>
#include <stdexcept>

namespace ae3 {
const OriginalSpatialRecord70* originalSpatialRecord_21d884(
    std::span<const OriginalSpatialRecord70> records,std::uint16_t index) {
    // 0x21D884: original load base+4, mul index by 0x70, add.
    if(index>=records.size())throw std::out_of_range("Original 0x21D884 index outside registry");
    return &records[index];
}
std::array<std::byte,16> originalSpatialVector_21b458(
    const OriginalSpatialRecord70& record,std::size_t index) {
    // 0x21B458: read original LQ 16 bytes at +0x40 + 16*index.
    if(index>=3)throw std::out_of_range("Original 0x21B458 vector index out of range");
    std::array<std::byte,16> raw{};
    const std::size_t offset=0x40+16*index;
    for(std::size_t i=0;i<16;i++)raw[i]=record.originalFields[offset+i];
    return raw;
}
const OriginalSpatialRecord70* originalSpatialByEntry_21b404(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::uint16_t> entries,std::size_t index) {
    // 0x21B404: count at header+2, entry index at header+4+2*i,
    // tail J 0x21D884. Caller supplies valid source-game hit span.
    if(index>=entries.size())return nullptr;
    return originalSpatialRecord_21d884(records,entries[index]);
}
std::uint16_t originalSpatialHeaderHalfword_21b444(
    std::span<const std::uint16_t> at0a,std::size_t index) {
    if(index>=at0a.size())throw std::out_of_range("Original header+0xA out of range");
    return at0a[index];
}
std::uint32_t originalSpatialWordAt04_21b4e0(
    const OriginalSpatialWordData& data,std::size_t index) {
    if(index>=7)throw std::out_of_range("Original data+4 outside record");
    return data.originalWords[1+index];
}
std::uint32_t originalSpatialWordAt10_21b4f0(
    const OriginalSpatialWordData& data,std::size_t index) {
    if(index>=4)throw std::out_of_range("Original data+0x10 outside record");
    return data.originalWords[4+index];
}
std::uint32_t originalSpatialWordAt1C_21b500(
    const OriginalSpatialWordData& data) noexcept {
    return data.originalWords[7];
}
const OriginalSpatialRecord70* originalSpatialNearest_21d898(
    std::span<const OriginalSpatialRecord70> records,
    OriginalSpatialGridHits hits,OriginalSpatialEngineCalls& original) {
    // Original 0x6E3E7C = FLT_MAX; original first accepted initializes
    // minimum, later candidates win only with strict less-than. No
    // synthetic VU0 vector distance or eligibility implementation.
    const OriginalSpatialRecord70* best=nullptr;
    float minDistance=FLT_MAX;
    for(const auto index:hits.originalRecordIndices) {
        const auto* record=originalSpatialRecord_21d884(records,index);
        if(!original.call_original_21b788_eligibility(*record))continue;
        const float d=original.call_original_21b468_and_VU_distance(*record);
        if(best==nullptr||d<minDistance){best=record;minDistance=d;}
    }
    return best;
}
const OriginalSpatialRecord70* originalSpatialNearestAll_21da2c(
    std::span<const OriginalSpatialRecord70> records,
    OriginalSpatialEngineCalls& original) {
    // 0x21DA2C scans original registry count at vtable[1]+0x24,
    // uses 0x70 stride, and does not call the eligibility predicate.
    const OriginalSpatialRecord70* best=nullptr;
    float minDistance=FLT_MAX; // Original global 0x6E3E80.
    for(const auto& record:records) {
        const float d=original.call_original_21b468_and_VU_distance(record);
        if(best==nullptr||d<minDistance){best=&record;minDistance=d;}
    }
    return best;
}
void originalSpatialRegistryDestroy_21d7d8(
    OriginalSpatialRegistryOwner& state,OriginalSpatialDestroyCalls& original) {
    // Source code 0x21D7D8. Original calls 0x41A3B0 on block+0,
    // then 0x21B384 on each 0x70 record, then 0x36EDE8 release.
    if(state.ownedBlock00) {
        original.call_original_41a3b0_free(state.ownedBlock00);
        state.ownedBlock00=nullptr;
    }
    if(state.originalRecordAllocation04) {
        for(std::int32_t i=0;i<state.recordCount14;++i) {
            if(static_cast<std::size_t>(i)>=state.records.size())
                throw std::out_of_range("Original record count exceeds loaded native registry");
            original.call_original_21b384_record_destructor(state.records[static_cast<std::size_t>(i)]);
        }
        original.call_original_36ede8_release(state.originalRecordAllocation04);
        state.originalRecordAllocation04=nullptr;
    }
    if(state.auxiliaryAllocation10) {
        original.call_original_36ede8_release(state.auxiliaryAllocation10);
        state.auxiliaryAllocation10=nullptr;
        // Original field +0x0C reset to -1 ONLY when +0x10 is nonnull.
        state.selectionIndex0c=-1;
    }
}
} // namespace ae3
