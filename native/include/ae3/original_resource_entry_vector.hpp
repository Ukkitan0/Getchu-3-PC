#pragma once
#include "ae3/original_exdb_package.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// Original 0x26D138 uses four original 32-bit words per source entry.
// Original 0x26D24C constructs the entry as name pointer at
// +0x00, type string pointer at +0x04, payload pointer at +0x08
// and payload byte count at +0x0C. This host representation owns the character/payload bytes;
// it is deliberately NOT binary-compatible with EE pointer structures.
struct OriginalResourceEntry16_26d138 {
    std::string sourceName;
    std::string sourceType;
    std::vector<std::byte> sourcePayload;
    std::uint32_t originalPckPayloadOffset=0; // PCK metadata, not EE +0x04
    std::span<const std::byte> data() const noexcept {return sourcePayload;}
};

// Source 0x26D120: (sourceEnd-sourceBegin)/16.
// Source 0x26D214: sourceBegin+sourceIndex*16, no bounds validation.
// Source 0x26D138: append 16-byte source entry; the original calls
// 0x516F08 when the preallocated source vector is full. We reject it
// with an exception, rather than implicitly growing or corrupting data.
class OriginalResourceEntryVector_26d138 {
public:
    explicit OriginalResourceEntryVector_26d138(std::size_t originalCapacity)
        : originalCapacity_(originalCapacity) {
        if(originalCapacity_>std::numeric_limits<std::uint32_t>::max()/16u)
            throw std::length_error("Original resource vector exceeds 32-bit addressable length");
        entries_.reserve(originalCapacity_);
    }
    OriginalResourceEntryVector_26d138(const OriginalResourceEntryVector_26d138&)=delete;
    OriginalResourceEntryVector_26d138& operator=(const OriginalResourceEntryVector_26d138&)=delete;
    OriginalResourceEntryVector_26d138(OriginalResourceEntryVector_26d138&&)=default;
    OriginalResourceEntryVector_26d138& operator=(OriginalResourceEntryVector_26d138&&)=default;

    void originalAppend_26d138(OriginalResourceEntry16_26d138 item) {
        if(entries_.size()==originalCapacity_)
            throw std::length_error("Original resource entry vector is full (0x516F08)");
        entries_.push_back(std::move(item));
    }
    // Original 0x26D1B0 resets the existing vector range before each
    // 0x26D24C request. Keep the original allocation capacity.
    void originalClear_26d1b0() noexcept {entries_.clear();}
    std::size_t originalCount_26d120() const noexcept {return entries_.size();}
    const OriginalResourceEntry16_26d138& originalAt_26d214(std::size_t n) const {
        return entries_.at(n); // native safety beyond source unchecked access
    }
    std::span<const OriginalResourceEntry16_26d138> entries() const noexcept {
        return entries_;
    }
    std::size_t originalCapacity() const noexcept{return originalCapacity_;}
private:
    std::size_t originalCapacity_=0;
    std::vector<OriginalResourceEntry16_26d138> entries_;
};

// Data-adapter, not a claim that the original PCK archive *reader* has
// been decompiled: entries come from authenticated original PCK bytes
// using previously committed originalExdbPackageEntriesFromPck.
// Unlike a borrowed span, returned entries own their payload bytes,
// and remain valid after the original PCK allocation is released.
inline OriginalResourceEntryVector_26d138 originalResourceEntryVectorFromPck(
        std::span<const std::byte> originalPckBytes) {
    auto originals=originalExdbPackageEntriesFromPck(originalPckBytes);
    OriginalResourceEntryVector_26d138 result(originals.size());
    for(const auto& original:originals){
        OriginalResourceEntry16_26d138 owned;
        owned.sourceName=original.name;
        owned.sourceType=original.type;
        owned.originalPckPayloadOffset=original.packageDataOffset;
        owned.sourcePayload.assign(original.originalBytes.begin(),original.originalBytes.end());
        result.originalAppend_26d138(std::move(owned));
    }
    return result;
}

// Original 0x26D24C consults the PCK source table at +0x10,
// with count +0x0C, string base +0x08 and payload base +0x00.
// The name/type/payload/size entries are 16 bytes each. It compares
// source type against requested kind via 0x42D148 for strlen(kind)
// bytes, and then permits only NUL or ASCII SPACE after the match
// (0x26D2CC..0x26D2E0). Not a generic prefix search.
inline bool originalResourceTypeMatch_26d2b8(std::string_view originalType,
                                             std::string_view requestedKind) noexcept {
    if(!originalType.starts_with(requestedKind))return false;
    return originalType.size()==requestedKind.size() ||
           originalType[requestedKind.size()]==' ' ||
           originalType[requestedKind.size()]=='\0';
}
inline OriginalResourceEntryVector_26d138 originalFilterResourceEntriesFromPck_26d24c(
        std::span<const std::byte> originalPckBytes,std::string_view requestedKind) {
    const auto originalEntries=originalExdbPackageEntriesFromPck(originalPckBytes);
    OriginalResourceEntryVector_26d138 selected(originalEntries.size());
    for(const auto& original:originalEntries) {
        if(!originalResourceTypeMatch_26d2b8(original.type,requestedKind))continue;
        OriginalResourceEntry16_26d138 entry;
        entry.sourceName=original.name;
        entry.sourceType=original.type;
        entry.originalPckPayloadOffset=original.packageDataOffset;
        entry.sourcePayload.assign(original.originalBytes.begin(),original.originalBytes.end());
        selected.originalAppend_26d138(std::move(entry));
    }
    return selected;
}

// Source 0x35FCBC chooses 'exdb' and 0x35FD20 selects exactly the
// 'bgm_desc.exdb' source name. A data adapter does not substitute the
// original audio consumer at 0x1BB63C (its native core is recovered
// separately) or invent a sound backend.
inline const OriginalResourceEntry16_26d138* originalBgmDescriptorResource_35fd20(
        const OriginalResourceEntryVector_26d138& resources) noexcept {
    for(const auto& item:resources.entries())
        if(item.sourceType==OriginalBgmExdbType_35fcbc &&
           item.sourceName==OriginalBgmExdbName_35fd20)
            return &item;
    return nullptr;
}
} // namespace ae3
