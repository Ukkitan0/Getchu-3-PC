#pragma once
#include "ae3/original_nav_route_dispatch.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace ae3 {
// Native-only weak state for actor route invalidation when the original
// 0x21F82C / 0x21F804 circular membership list changes or is destroyed.
// NOT an R5900/EE list field. A weak ref also rejects allocation ABA.
struct OriginalNavNativeListRevision {
    std::uint64_t generation=1;
};

// Original 0x4E3EB4 allocates a 12-byte sentinel, with next and prev
// pointing to itself. Actual source 32-bit pointer offsets are 0/4/8.
// This host-native class preserves circular list semantics, not guest ABI.
class OriginalNavCircularList {
public:
    struct Node {
        Node* next=nullptr;
        Node* prev=nullptr;
        const OriginalSpatialRecord70* polygon=nullptr;
    };
    OriginalNavCircularList();
    ~OriginalNavCircularList()=default;
    OriginalNavCircularList(const OriginalNavCircularList&)=delete;
    OriginalNavCircularList& operator=(const OriginalNavCircularList&)=delete;
    OriginalNavCircularList(OriginalNavCircularList&&)=delete;
    OriginalNavCircularList& operator=(OriginalNavCircularList&&)=delete;
    bool originalContains_4e3dac(const OriginalSpatialRecord70* polygon) const noexcept;
    bool originalInsertUnique_21f82c(const OriginalSpatialRecord70* polygon);
    std::size_t originalErase_21f804(const OriginalSpatialRecord70* polygon) noexcept;
    void originalClear_4e4154() noexcept;
    std::vector<const OriginalSpatialRecord70*> originalOrderedValues() const;
    std::size_t size() const noexcept {return nodes_.size();}
    bool debugOriginalSentinelValid() const noexcept;
    std::weak_ptr<const OriginalNavNativeListRevision>
        originalHostRevisionState() const noexcept { return revision_; }
    std::uint64_t originalHostGeneration() const noexcept {
        return revision_->generation;
    }
private:
    void noteNativeMutation() noexcept {
        if(++revision_->generation==0)++revision_->generation;
    }
    std::shared_ptr<OriginalNavNativeListRevision> revision_=
        std::make_shared<OriginalNavNativeListRevision>();
    std::unique_ptr<Node> sentinel_;
    std::vector<std::unique_ptr<Node>> nodes_;
};

// Owner+4 is a pointer to a 4-byte list proxy initialized at 0x21F604;
// that proxy owns the 12-byte sentinel and its intrusive nodes.
class OriginalNavListOwner {
public:
    void originalCreate_21f604();
    void originalDispose_21f64c() noexcept;
    void originalDispose_21f690() noexcept;
    OriginalNavCircularList* originalList() noexcept {return list_.get();}
    const OriginalNavCircularList* originalList() const noexcept {return list_.get();}
private:
    std::unique_ptr<OriginalNavCircularList> list_;
};

// 0x21EF3C is an enormous COP2/MMI original predicate (currently
// unresolved). Original 0x21F6D4 adds only polygons whose returned
// scalar float equals 0.0f, and skips duplicates.
class OriginalNavPopulationPredicate {
public:
    virtual ~OriginalNavPopulationPredicate()=default;
    virtual float call_original_21ef3c(
        const OriginalSpatialRecord70& polygon,
        const OriginalVU0Vec4& sourceCentroid)=0;
};
std::size_t originalNavPopulateList_21f6d4(
    OriginalNavListOwner& owner,
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const std::byte> originalNavigationFile,
    OriginalNavPopulationPredicate& unresolvedOriginalPredicate);

// True source-list owners feed the existing recovered route dispatcher.
// The original list insertion order is preserved when computing fallback
// distances (first strict-minimum polygon keeps the tie).
OriginalNavDispatchResult originalNavRouteDispatchOwned_2207d0(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const std::byte> originalNavigationFile,
    OriginalNavDispatchRequest request,
    const OriginalNavListOwner& originalMembershipOwner,
    OriginalNavRouteSplineCalls* unresolvedSpline=nullptr);
} // namespace ae3
