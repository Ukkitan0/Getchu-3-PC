#pragma once
#include "ae3/original_resource_bucket_owner.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <stdexcept>

namespace ae3 {
// Original 0x1BB458 populates 0xB0-byte records; 0x1BB3EC walks
// them via 0x1BB4E4 and inserts each record's first 0x20-byte key
// into the already recovered 0x36D610 bucket owner.
inline constexpr std::size_t OriginalResourceRecordStride_1bb4e4=0xb0;
struct OriginalResourceRecordB0 {
    std::span<const std::byte> originalBytes;
    std::size_t sourceIndex=0;
};
// SOURCE-OWNED, already populated original record buffer. Caller
// must keep source allocation alive and unmoved while owner is used.
// No fabricated ISO or resource-descriptor decoding.
class OriginalResourceRecordBatchOwner {
public:
    OriginalResourceRecordBatchOwner()=default;
    OriginalResourceRecordBatchOwner(const OriginalResourceRecordBatchOwner&)=delete;
    OriginalResourceRecordBatchOwner& operator=(const OriginalResourceRecordBatchOwner&)=delete;
    OriginalResourceRecordBatchOwner(OriginalResourceRecordBatchOwner&&)=delete;
    OriginalResourceRecordBatchOwner& operator=(OriginalResourceRecordBatchOwner&&)=delete;

    std::size_t originalAppendPopulatedRecords_1bb3ec(
        std::span<const std::byte> originalPopulatedBuffer,
        std::int32_t originalGlobalCount_2bb0d0) {
        // Retail BLEZ at 0x1BB408 precedes source dereferences.
        if(originalGlobalCount_2bb0d0<=0)return 0;
        const auto count=static_cast<std::size_t>(originalGlobalCount_2bb0d0);
        if(count>originalPopulatedBuffer.size()/OriginalResourceRecordStride_1bb4e4)
            throw std::out_of_range("Original 0xB0-byte record pool is truncated");
        for(std::size_t i=0;i<count;++i) {
            const auto bytes=originalPopulatedBuffer.subspan(
                i*OriginalResourceRecordStride_1bb4e4,
                OriginalResourceRecordStride_1bb4e4);
            OriginalResourceKey32 key{};
            std::copy_n(bytes.begin(),key.size(),key.begin());
            stableRecords_.push_back({bytes,i});
            try {
                originalBuckets_.originalInsert_36d610(key,stableRecords_.back());
            } catch(...) {
                stableRecords_.pop_back();
                throw;
            }
        }
        return count;
    }
    const OriginalResourceBucketHeads<OriginalResourceRecordB0>& originalBuckets() const noexcept {
        return originalBuckets_.originalBuckets();
    }
    std::size_t size() const noexcept{return stableRecords_.size();}
    std::size_t originalBroadcast_36d844(
        OriginalResourceEventSink<OriginalResourceRecordB0>& sink) const {
        return originalBuckets_.originalBroadcast_36d844(sink);
    }
    void originalClear() noexcept{
        originalBuckets_.originalClear();
        stableRecords_.clear();
    }
private:
    // deque references stay stable across further push_back calls.
    std::deque<OriginalResourceRecordB0> stableRecords_{};
    OriginalResourceBucketOwner<OriginalResourceRecordB0> originalBuckets_{};
};
} // namespace ae3
