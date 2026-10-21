#pragma once
#include "ae3/original_resource_broadcast.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace ae3 {
// Retail 0x36CFD0 reduces both halves of a 32-byte key to 8-bit
// byte sums. 0x36D69C/0x36D6A0 combines them and masks to 7 bits.
// Closed-form native C++; no original EE instruction execution.
using OriginalResourceKey32 = std::array<std::byte,32>;
inline std::uint8_t originalResourceBucketIndex_36cfd0_36d6a0(
    const OriginalResourceKey32& originalKey) noexcept {
    unsigned sum=0;
    for(const std::byte b:originalKey)
        sum+=std::to_integer<unsigned char>(b);
    return static_cast<std::uint8_t>(sum & 0x7fu);
}

// Original 0x36D610 copies the 0x20-byte source key to node+0,
// stores the source pointer at +0x20, clears +0x24 next, then
// appends to the selected bucket without deduplication.
// Native pointers replace original guest addresses and resource
// ownership stays external to the bucket node.
template<class Resource> class OriginalResourceBucketOwner {
public:
    struct Entry {
        OriginalResourceKey32 originalKey{};
        OriginalResourceBucketNode<Resource> node{};
        std::uint8_t originalBucket=0;
    };
    OriginalResourceBucketOwner()=default;
    OriginalResourceBucketOwner(const OriginalResourceBucketOwner&)=delete;
    OriginalResourceBucketOwner& operator=(const OriginalResourceBucketOwner&)=delete;
    OriginalResourceBucketOwner(OriginalResourceBucketOwner&&)=delete;
    OriginalResourceBucketOwner& operator=(OriginalResourceBucketOwner&&)=delete;

    const Entry& originalInsert_36d610(
        const OriginalResourceKey32& originalKey,const Resource& resource) {
        const std::uint8_t bucket=originalResourceBucketIndex_36cfd0_36d6a0(originalKey);
        auto item=std::make_unique<Entry>();
        item->originalKey=originalKey;
        item->node.resource=&resource;
        item->node.next=nullptr;
        item->originalBucket=bucket;
        // Establish ownership before exposing a linked node.
        owned_.push_back(std::move(item));
        auto* inserted=&owned_.back()->node;
        auto** position=&heads_[bucket];
        while(*position!=nullptr)position=&(*position)->next;
        *position=inserted;
        return *owned_.back();
    }
    const OriginalResourceBucketHeads<Resource>& originalBuckets() const noexcept {
        return heads_;
    }
    std::size_t size() const noexcept {return owned_.size();}
    void originalClear() noexcept {
        heads_.fill(nullptr);
        owned_.clear();
    }
    std::size_t originalBroadcast_36d844(OriginalResourceEventSink<Resource>& sink) const {
        return originalResourceBroadcast_36d844(heads_,sink);
    }
private:
    OriginalResourceBucketHeads<Resource> heads_{};
    std::vector<std::unique_ptr<Entry>> owned_{};
};
} // namespace ae3
