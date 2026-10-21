#pragma once
#include "ae3/original_attribute_loaded_text.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace ae3 {
// Source 0x2BB0D0 loads original global owner+0x20 as record count.
// Source 0x2BB0F4 reads owner+0x24 stride and owner+0x04 rebased
// pointer. R5900 MULT a0,t6,a0 produces the low 32-bit index*stride
// and the JR delay slot returns base+that product (ADDU).
struct OriginalAttributeRecordIndex_2bb0f4 {
    std::uint32_t sourceRecordBaseAt04=0;
    std::int32_t sourceCountAt20=0;
    std::int32_t sourceStrideAt24=0;
};
inline OriginalAttributeRecordIndex_2bb0f4 originalAttributeRecordIndexFromParser_2bb0f4(
        const OriginalAttributeMaterializedFields_2ba584& input) noexcept {
    return {input.trailer.originalRebasedPointerAt04,
        input.body.sourceHeader.sourceFourthWordAt20,
        input.trailer.sourcePenultimateWordAt24};
}
inline std::uint32_t originalAttributeRecordGuestAddress_2bb0f4(
        const OriginalAttributeRecordIndex_2bb0f4& index,
        std::uint32_t sourceRecordIndex) noexcept {
    return index.sourceRecordBaseAt04+sourceRecordIndex*
        std::bit_cast<std::uint32_t>(index.sourceStrideAt24);
}
// The original accessor does not check bounds. This separate native
// range-checked adapter operates on an externally provided loaded
// allocation and its caller-supplied guest-base address; never casts
// an original 32-bit PS2 pointer to a host pointer.
inline std::span<const std::byte> originalAttributeRecordBytesChecked_2bb0f4(
        std::span<const std::byte> originalAllocatedBytes,
        std::uint32_t originalAllocationGuestBase,
        const OriginalAttributeRecordIndex_2bb0f4& index,
        std::uint32_t sourceRecordIndex,
        std::size_t sourceRecordBytes) {
    if(index.sourceCountAt20<=0 ||
       sourceRecordIndex>=static_cast<std::uint32_t>(index.sourceCountAt20))
        throw std::out_of_range("Original attribute record index beyond parsed count");
    const auto address=originalAttributeRecordGuestAddress_2bb0f4(index,sourceRecordIndex);
    if(address<originalAllocationGuestBase)
        throw std::out_of_range("Original record points before loaded source allocation");
    const std::uint64_t offset=std::uint64_t(address)-originalAllocationGuestBase;
    if(offset>originalAllocatedBytes.size() ||
       sourceRecordBytes>originalAllocatedBytes.size()-static_cast<std::size_t>(offset))
        throw std::out_of_range("Original attribute record extends beyond loaded allocation");
    return originalAllocatedBytes.subspan(static_cast<std::size_t>(offset),sourceRecordBytes);
}
} // namespace ae3
