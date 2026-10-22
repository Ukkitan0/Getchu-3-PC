#include "ae3/original_nav_linked_owner.hpp"
#include <algorithm>
#include <stdexcept>

namespace ae3 {
OriginalNavCircularList::OriginalNavCircularList()
    : sentinel_(std::make_unique<Node>()) {
    sentinel_->next=sentinel_.get();
    sentinel_->prev=sentinel_.get();
}
bool OriginalNavCircularList::originalContains_4e3dac(
    const OriginalSpatialRecord70* polygon) const noexcept {
    for(auto* node=sentinel_->next;node!=sentinel_.get();node=node->next)
        if(node->polygon==polygon)return true;
    return false;
}
bool OriginalNavCircularList::originalInsertUnique_21f82c(
    const OriginalSpatialRecord70* polygon){
    if(originalContains_4e3dac(polygon))return false;
    auto node=std::make_unique<Node>();
    node->polygon=polygon;
    // 0x21F85C..0x21F884 inserts BEFORE the old first node,
    // not at the tail. Reverse-of-insertion ordering is essential.
    nodes_.reserve(nodes_.size()+1); // allocate before changing live links
    node->next=sentinel_->next;
    node->prev=sentinel_->next->prev;
    node->next->prev=node.get();
    node->prev->next=node.get();
    nodes_.push_back(std::move(node));
    noteNativeMutation();
    return true;
}
std::size_t OriginalNavCircularList::originalErase_21f804(
    const OriginalSpatialRecord70* polygon) noexcept {
    std::size_t removed=0;
    for(auto i=nodes_.begin();i!=nodes_.end();){
        Node* node=i->get();
        if(node->polygon!=polygon){++i;continue;}
        node->next->prev=node->prev;
        node->prev->next=node->next;
        i=nodes_.erase(i);
        ++removed;
    }
    if(removed)noteNativeMutation();
    return removed;
}
void OriginalNavCircularList::originalClear_4e4154() noexcept {
    if(!nodes_.empty())noteNativeMutation();
    nodes_.clear();
    sentinel_->next=sentinel_.get();
    sentinel_->prev=sentinel_.get();
}
std::vector<const OriginalSpatialRecord70*> OriginalNavCircularList::originalOrderedValues() const {
    std::vector<const OriginalSpatialRecord70*> values;
    values.reserve(nodes_.size());
    for(auto* node=sentinel_->next;node!=sentinel_.get();node=node->next)
        values.push_back(node->polygon);
    return values;
}
bool OriginalNavCircularList::debugOriginalSentinelValid() const noexcept {
    const auto* sentinel=sentinel_.get();
    if(!sentinel || !sentinel->next || !sentinel->prev ||
       sentinel->next->prev!=sentinel||sentinel->prev->next!=sentinel)return false;
    std::size_t count=0;
    for(auto* n=sentinel->next;n!=sentinel;n=n->next){
        if(!n || !n->next || !n->prev || n->next->prev!=n || n->prev->next!=n)return false;
        if(++count>nodes_.size())return false;
    }
    return count==nodes_.size();
}
void OriginalNavListOwner::originalCreate_21f604(){
    if(list_)throw std::logic_error("Original NavGraph list owner already initialized");
    list_=std::make_unique<OriginalNavCircularList>();
}
void OriginalNavListOwner::originalDispose_21f64c() noexcept{list_.reset();}
void OriginalNavListOwner::originalDispose_21f690() noexcept{list_.reset();}
namespace {
std::uint16_t read16(std::span<const std::byte> data,std::size_t index){
    if(index>data.size()||data.size()-index<2)
        throw std::out_of_range("Original NavGraph descriptor header truncated");
    return static_cast<std::uint16_t>(
        std::to_integer<unsigned char>(data[index])|
        (unsigned(std::to_integer<unsigned char>(data[index+1]))<<8));
}
std::uint32_t read32(std::span<const std::byte> data,std::size_t index){
    return std::uint32_t(read16(data,index))|(std::uint32_t(read16(data,index+2))<<16);
}
}
std::size_t originalNavPopulateList_21f6d4(
    OriginalNavListOwner& owner,std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav,OriginalNavPopulationPredicate& predicate){
    auto* list=owner.originalList();
    if(!list)throw std::logic_error("Original 0x21F604 list owner not constructed");
    const std::size_t count=read32(nav,0x24),table=read32(nav,0x3c);
    if(count!=records.size()||table>nav.size()||count>(nav.size()-table)/0x1e)
        throw std::out_of_range("Original navigation polygon descriptor table missing");
    std::size_t added=0;
    for(std::size_t i=0;i<count;i++){
        const auto pointCount=read16(nav,table+0x1e*i+0x16);
        if(pointCount==0||pointCount>3)
            throw std::out_of_range("Unrecovered original polygon vertex layout");
        const auto points=originalSpatialVertices_21b468(records[i]);
        const auto midpoint=originalVU0Mean_21b468(std::span(points).first(pointCount));
        // 0x21F758..0x21F764 tests F0==0.0f, not nonzero / non-NaN.
        if(predicate.call_original_21ef3c(records[i],midpoint)!=0.0f)continue;
        // 0x21F768..0x21F77C: original exact pointer dedup.
        if(list->originalInsertUnique_21f82c(&records[i]))++added;
    }
    return added;
}
OriginalNavDispatchResult originalNavRouteDispatchOwned_2207d0(
    std::span<const OriginalSpatialRecord70> records,std::span<const std::byte> nav,
    OriginalNavDispatchRequest request,const OriginalNavListOwner& member,
    OriginalNavRouteSplineCalls* spline){
    if(!member.originalList())
        throw std::logic_error("Original NavGraph ownership not initialized");
    const auto orderedMembers=member.originalList()->originalOrderedValues();
    request.sourceMembershipListExists=true;
    request.orderedOriginalMembershipList=orderedMembers;
    return originalNavRouteDispatch_2207d0(records,nav,request,spline);
}
} // namespace ae3
