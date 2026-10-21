#include "ae3/original_nav_route_admission.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace ae3 {
namespace {
std::uint16_t u16(std::span<const std::byte> data,std::size_t at) {
    if(at>data.size()||data.size()-at<2)
        throw std::out_of_range("Original .nav descriptor exceeds source allocation");
    return static_cast<std::uint16_t>(std::to_integer<unsigned char>(data[at])|
        (unsigned(std::to_integer<unsigned char>(data[at+1]))<<8));
}
std::uint32_t u32(std::span<const std::byte> data,std::size_t at) {
    return std::uint32_t(u16(data,at))|(std::uint32_t(u16(data,at+2))<<16);
}
struct DescriptorView {
    std::span<const std::byte> data;
    std::size_t count=0,base=0;
    DescriptorView(std::span<const std::byte> source,std::size_t recordCount):data(source) {
        count=u32(source,0x24);
        base=u32(source,0x3c);
        if(count!=recordCount||base>source.size()||
           count>(source.size()-base)/0x1e)
            throw std::out_of_range("Original graph polygon/descriptor mismatch");
    }
    std::size_t descriptor(std::size_t idx) const {
        if(idx>=count)throw std::out_of_range("Original .nav polygon index invalid");
        return base+idx*0x1e;
    }
    std::uint16_t degree(std::size_t idx) const {
        const auto d=u16(data,descriptor(idx)+2);
        if(d>3)throw std::out_of_range("Original .nav link count exceeds descriptor capacity");
        return d;
    }
    std::uint16_t target(std::size_t idx,std::size_t slot) const {
        if(slot>=degree(idx))throw std::out_of_range("Original adjacency slot invalid");
        const auto value=u16(data,descriptor(idx)+4+2*slot);
        if(value>=count)throw std::out_of_range("Original .nav target out of range");
        return value;
    }
    std::uint16_t kind(std::size_t idx,std::size_t slot) const {
        if(slot>=degree(idx))throw std::out_of_range("Original adjacency edge type slot invalid");
        return u16(data,descriptor(idx)+0xa+2*slot);
    }
    std::uint16_t vertexCount(std::size_t idx) const {
        const auto countValue=u16(data,descriptor(idx)+0x16);
        if(countValue==0||countValue>3)
            throw std::out_of_range("Unrecovered original polygon vertex layout");
        return countValue;
    }
};
OriginalVU0Vec4 centroid(const OriginalSpatialRecord70& r,std::uint16_t count){
    const auto points=originalSpatialVertices_21b468(r);
    return originalVU0Mean_21b468(std::span(points).first(count));
}
}
void originalNavStartEpoch_21afb8(OriginalNavEpochState& state,std::size_t count){
    if(state.lastSeenEpoch.size()!=count||state.seen.size()!=count){
        state.epoch=0;
        state.lastSeenEpoch.assign(count,0);
        state.seen.assign(count,0);
    }
    // 0x21AFCC increments epoch; when it becomes zero, 0x21AFF8
    // clears original record +0x28 marks for the entire source graph.
    state.epoch++;
    if(state.epoch==0){
        std::fill(state.seen.begin(),state.seen.end(),0);
        std::fill(state.lastSeenEpoch.begin(),state.lastSeenEpoch.end(),0);
    }
}
bool originalNavAlreadySeen_21b014(OriginalNavEpochState& state,std::size_t i){
    if(i>=state.seen.size())throw std::out_of_range("Original visit mark index invalid");
    if(state.lastSeenEpoch[i]!=state.epoch){
        state.lastSeenEpoch[i]=state.epoch;
        state.seen[i]=0;
    }
    return state.seen[i]!=0;
}
void originalNavMarkSeen_21b038(OriginalNavEpochState& state,std::size_t i){
    if(i>=state.seen.size())throw std::out_of_range("Original visit mark index invalid");
    state.lastSeenEpoch[i]=state.epoch;
    state.seen[i]=1;
}
bool originalNavMembership_4e3dac(const OriginalSpatialRecord70* target,
    std::span<const OriginalSpatialRecord70* const> list) noexcept {
    for(const auto* original:list)if(original==target)return true;
    return false;
}
OriginalNavAdmissionResult originalNavAdmit_220aa4(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> data,
    const OriginalNavAdmissionRequest& request,OriginalNavEpochState& stamp){
    DescriptorView graph(data,records.size());
    if(request.startRecordIndex>=records.size())
        throw std::out_of_range("Original 0x220AA4 seed polygon out of range");
    originalNavStartEpoch_21afb8(stamp,records.size());
    OriginalNavAdmissionResult result;
    // Iterative DFS preserves the original 0x220AA4 recursive pre-order
    // and first-seen gating without risking a host call-stack overflow.
    struct Pending{std::uint32_t index;std::size_t nextLink;bool entered;};
    std::vector<Pending> stack{{request.startRecordIndex,0,false}};
    while(!stack.empty()){
        auto& item=stack.back();
        if(!item.entered){
            if(originalNavAlreadySeen_21b014(stamp,item.index)){
                stack.pop_back();continue;
            }
            ++result.candidateRecordsVisited;
            if(request.filterEnabled && request.originalFilterListExists &&
               !originalNavMembership_4e3dac(&records[item.index],request.originalFilterList)){
                stack.pop_back();continue;
            }
            const auto center=centroid(records[item.index],graph.vertexCount(item.index));
            const float distance=originalVU0DistanceXYZ_21b234(center,request.query);
            // Source 0x220B5C C.OLT.S: equality rejects candidate.
            if(!(distance<request.maximumDistance)){
                stack.pop_back();continue;
            }
            if(result.sourceRecordIndicesInDepthFirstOrder.size()>=kOriginalNavWorkspaceCapacity)
                throw std::out_of_range("Original fixed 256-node route allocation exceeded");
            originalNavMarkSeen_21b038(stamp,item.index);
            result.sourceRecordIndicesInDepthFirstOrder.push_back(item.index);
            item.entered=true;
        }
        const auto n=graph.degree(item.index);
        if(item.nextLink>=n){stack.pop_back();continue;}
        const auto child=graph.target(item.index,item.nextLink++);
        stack.push_back({child,0,false});
    }
    return result;
}
OriginalNavRouteWorkspace originalNavBuildAdmittedWorkspace_220c00(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav,
    const OriginalNavAdmissionResult& admitted){
    DescriptorView graph(nav,records.size());
    OriginalNavRouteWorkspace output;
    const auto& indices=admitted.sourceRecordIndicesInDepthFirstOrder;
    if(indices.size()>kOriginalNavWorkspaceCapacity)
        throw std::out_of_range("Original 0x2207D0 workspace capacity");
    std::vector<std::int32_t> sourceToNode(records.size(),-1);
    output.nodes.reserve(indices.size());
    for(std::size_t i=0;i<indices.size();++i){
        const auto source=indices[i];
        if(source>=records.size()||sourceToNode[source]>=0)
            throw std::invalid_argument("Invalid repeated original admitted polygon");
        sourceToNode[source]=static_cast<std::int32_t>(i);
        OriginalNavRouteNode38 node;
        node.polygon04=&records[source];
        node.centroid=centroid(records[source],graph.vertexCount(source));
        output.nodes.push_back(node);
    }
    for(std::size_t i=0;i<indices.size();i++){
        const auto source=indices[i];
        auto& node=output.nodes[i];
        for(std::size_t slot=0;slot<graph.degree(source);slot++){
            const auto neighbor=graph.target(source,slot);
            const auto target=sourceToNode[neighbor];
            // Original 0x220CBC skips neighbors which were not admitted.
            if(target==-1)continue;
            const auto destSlot=node.linkCount08++;
            node.destinationIndices0c[destSlot]=target;
            node.linkTypes18[destSlot]=graph.kind(source,slot);
            node.sourceLinkIndices24[destSlot]=static_cast<std::uint32_t>(slot);
        }
    }
    return output;
}
std::vector<const OriginalSpatialRecord70*> originalNavRouteFromSource_2207d0(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> data,
    const OriginalNavAdmissionRequest& request,
    OriginalNavRouteSplineCalls* spline){
    OriginalNavEpochState stamp;
    const auto admitted=originalNavAdmit_220aa4(records,data,request,stamp);
    auto workspace=originalNavBuildAdmittedWorkspace_220c00(records,data,admitted);
    if(workspace.nodes.empty())return {};
    originalNavRouteRelax_220d5c(workspace,spline);
    // Partial top-level host convenience: nearest-to-query end. The
    // original 0x2207D0 also has farthest/alternate modes and a
    // 0x21F8B8 fallback which are not yet translated here.
    const auto endpoint=originalNavRouteNearest_220f60(workspace,request.query);
    return originalNavRouteExtract_220994(workspace,endpoint);
}
} // namespace ae3
