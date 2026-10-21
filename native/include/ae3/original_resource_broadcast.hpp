#pragma once
#include "ae3/original_navrange_registration.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
// Original EE method addresses: static identifiers, never executable host pointers.
// 0x27242C stacks descriptor 0x6AF610 -> 0x6AF628 -> base 0x6AE290.
inline constexpr std::uint32_t OriginalResourceActiveDescriptorVA=0x6af610u;
inline constexpr std::uint32_t OriginalResourceTransitionDescriptorVA=0x6af628u;
inline constexpr std::uint32_t OriginalResourceBaseDescriptorVA=0x6ae290u;
inline constexpr std::array<std::uint32_t,4> OriginalResourceActiveMethods{
    0x518e4cu,0x518e14u,0x518df4u,0x518e24u};
inline constexpr std::array<std::uint32_t,4> OriginalResourceTransitionMethods{
    0x518e4cu,0x518dc8u,0x518da8u,0x41b8e8u};

template<class Resource> struct OriginalResourceBucketNode {
    // Source node+0x20 resource pointer and +0x24 next link.
    // Native pointers are externally owned; no guest EE memory is executed.
    const Resource* resource=nullptr;
    OriginalResourceBucketNode* next=nullptr;
};
template<class Resource> using OriginalResourceBucketHeads=
    std::array<OriginalResourceBucketNode<Resource>*,128>;

template<class Resource> class OriginalResourceEventSink {
public:
    virtual ~OriginalResourceEventSink()=default;
    // 0x36D844 descriptor[0] -> 0x518E4C -> 0x518E24:
    // underlying receiver's first method gets original node+0x20 payload.
    virtual void originalOnResource(const Resource&)=0;
};

// Source-level 0x36D844..0x36D8C4; bucket heads 0..127, then +0x24
// linked records in order. Reads the next link AFTER each callback, as
// at retail 0x36D890; callers must keep original nodes alive/acyclic.
template<class Resource>
inline std::size_t originalResourceBroadcast_36d844(
    const OriginalResourceBucketHeads<Resource>& buckets,
    OriginalResourceEventSink<Resource>& callback) {
    std::size_t delivered=0;
    for(std::size_t bucket=0;bucket<128;++bucket) {
        for(auto* node=buckets[bucket];node!=nullptr;node=node->next) {
            if(!node->resource)
                throw std::invalid_argument("Original resource dispatcher node has null payload");
            callback.originalOnResource(*node->resource);
            ++delivered;
        }
    }
    return delivered;
}

template<class Resource>
inline std::size_t originalResourceDispatch_27242c(
    const OriginalResourceBucketHeads<Resource>& buckets,
    OriginalResourceEventSink<Resource>& originalTarget) {
    return originalResourceBroadcast_36d844(buckets,originalTarget);
}

struct OriginalNavrangeResourceDelivery {
    OriginalNavGroupSource source;
    std::span<const OriginalSpatialRecord70> polygons;
    std::span<const std::byte> navigation;
};
struct OriginalNavrangeDispatchSummary {
    std::size_t resourcesVisited=0;
    OriginalNavGroupBuildResult navrange{};
};

// Explicit opt-in dispatch from decoded source resources to the recovered
// original keyed owner/eligibility. Real disc allocation remains separate.
inline OriginalNavrangeDispatchSummary originalNavrangeDispatchRegistered_27242c(
    OriginalNavrangeRegisteredOwner& owner,
    const OriginalResourceBucketHeads<OriginalNavrangeResourceDelivery>& buckets) {
    class Sink final:public OriginalResourceEventSink<OriginalNavrangeResourceDelivery> {
    public:
        explicit Sink(OriginalNavrangeRegisteredOwner& instance):owner(instance){}
        void originalOnResource(const OriginalNavrangeResourceDelivery& item) override {
            const auto r=owner.originalDispatchResource_4e4060(
                item.source,item.polygons,item.navigation);
            ++summary.resourcesVisited;
            summary.navrange.sourceGroups+=r.sourceGroups;
            summary.navrange.ownersCreated+=r.ownersCreated;
            summary.navrange.sourcePolygonsInserted+=r.sourcePolygonsInserted;
        }
        OriginalNavrangeRegisteredOwner& owner;
        OriginalNavrangeDispatchSummary summary{};
    } sink(owner);
    (void)originalResourceDispatch_27242c(buckets,sink);
    return sink.summary;
}
} // namespace ae3
