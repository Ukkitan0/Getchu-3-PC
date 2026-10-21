#include "ae3/original_nav_route_search.hpp"
#include <algorithm>
#include <cfloat>
#include <stdexcept>

namespace ae3 {
namespace {
std::uint16_t read16(std::span<const std::byte> b,std::size_t p) {
    if(p>b.size()||b.size()-p<2)
        throw std::out_of_range("Original navigation descriptor truncated");
    return static_cast<std::uint16_t>(
        std::to_integer<unsigned char>(b[p])|
        (unsigned(std::to_integer<unsigned char>(b[p+1]))<<8));
}
std::uint32_t read32(std::span<const std::byte> b,std::size_t p) {
    return std::uint32_t(read16(b,p))|
        (std::uint32_t(read16(b,p+2))<<16);
}
}
OriginalNavRouteWorkspace originalNavRouteBuildSourceWorkspace_220c00(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav) {
    const auto count=static_cast<std::size_t>(read32(nav,0x24));
    const auto offset=static_cast<std::size_t>(read32(nav,0x3c));
    if(records.size()!=count||offset>nav.size()||
       count>(nav.size()-offset)/0x1e)
        throw std::out_of_range("Original navigation records do not match source");
    OriginalNavRouteWorkspace result;
    result.nodes.reserve(count);
    for(std::size_t i=0;i<count;i++){
        const auto at=offset+i*0x1e;
        OriginalNavRouteNode38 node;
        node.polygon04=&records[i];
        const auto pointCount=read16(nav,at+0x16);
        if(pointCount==0||pointCount>3)
            throw std::out_of_range("Unsupported original polygon vertex count");
        const auto raw=originalSpatialVertices_21b468(records[i]);
        node.centroid=originalVU0Mean_21b468(
            std::span<const OriginalVU0Vec4>(raw.data(),pointCount));
        const auto links=read16(nav,at+2);
        if(links>3)throw std::out_of_range("Original navigation record has >3 links");
        node.linkCount08=links;
        for(std::size_t k=0;k<links;k++){
            const auto dest=read16(nav,at+4+2*k);
            if(dest>=count)
                throw std::out_of_range("Original adjacency target outside graph");
            node.destinationIndices0c[k]=dest;
            node.linkTypes18[k]=read16(nav,at+0xa+2*k);
            node.sourceLinkIndices24[k]=static_cast<std::uint32_t>(k);
        }
        result.nodes.push_back(node);
    }
    return result;
}
std::int32_t originalNavRouteFindRecord_220d14(
    const OriginalNavRouteWorkspace& work,
    const OriginalSpatialRecord70* polygon) noexcept {
    // Original 0x220D14 iterates +0x38 records and compares field +0x4.
    for(std::size_t i=0;i<work.nodes.size();i++)
        if(work.nodes[i].polygon04==polygon)
            return static_cast<std::int32_t>(i);
    return -1;
}
void originalNavRouteRelax_220d5c(
    OriginalNavRouteWorkspace& work,OriginalNavRouteSplineCalls* spline) {
    if(work.nodes.empty())return;
    // Original 0x220D88/0x220D90 sets first predecessor=-1 and cost=0.
    auto& first=work.nodes.front();
    first.predecessor30=-1;
    first.cost34=0.0f;
    for(;;){
        float lowest=FLT_MAX; // genuine float global at 0x6E47E8
        std::int32_t candidate=-1;
        for(std::size_t i=0;i<work.nodes.size();i++){
            const auto& node=work.nodes[i];
            if(!node.visited00&&node.cost34<lowest){
                lowest=node.cost34;
                candidate=static_cast<std::int32_t>(i);
            }
        }
        if(candidate<0||lowest==FLT_MAX)break;
        auto& current=work.nodes[static_cast<std::size_t>(candidate)];
        current.visited00=true; // retail 0x220E30
        for(std::size_t k=0;k<current.linkCount08;k++){
            const auto target=current.destinationIndices0c[k];
            if(target<0||static_cast<std::size_t>(target)>=work.nodes.size())
                throw std::out_of_range("Original route target is not in current workspace");
            auto& neighbor=work.nodes[static_cast<std::size_t>(target)];
            if(neighbor.visited00)continue;
            float edgeCost;
            switch(current.linkTypes18[k]){
            case 0:
            case 2:
                // Both original branches reach 0x220EC8, 0x21B468
                // source centroid and original VU0 XYZ distance.
                edgeCost=originalVU0DistanceXYZ_21b234(
                    current.centroid,neighbor.centroid);
                break;
            case 1:
                // Original 0x220F18 loads the original path object
                // with 0x21B4E0 and sums it via 0x21B194.
                if(!spline)
                    throw std::runtime_error(
                        "Original type-1 spline dependency not reconstructed");
                edgeCost=spline->call_original_21b4e0_21b194(
                    *current.polygon04,current.sourceLinkIndices24[k]);
                break;
            default:
                // These are absent in the entire surveyed USA .nav set.
                throw std::runtime_error("Unknown original navigation edge type");
            }
            // Original ADD.S and C.OLT.S: strict less-than, keep old
            // predecessor on ties or NaNs.
            const float proposed=current.cost34+edgeCost;
            if(proposed<neighbor.cost34){
                neighbor.cost34=proposed;
                neighbor.predecessor30=candidate;
            }
        }
    }
}
std::int32_t originalNavRouteNearest_220f60(
    const OriginalNavRouteWorkspace& work,
    const OriginalVU0Vec4& query) noexcept {
    float best=FLT_MAX;
    std::int32_t index=-1;
    for(std::size_t i=0;i<work.nodes.size();i++){
        const float dist=originalVU0DistanceXYZ_21b234(
            work.nodes[i].centroid,query);
        if(dist<best){best=dist;index=static_cast<std::int32_t>(i);}
    }
    return index;
}
std::int32_t originalNavRouteFarthest_221030(
    const OriginalNavRouteWorkspace& work,
    const OriginalVU0Vec4& query) noexcept {
    float best=-FLT_MAX;
    std::int32_t index=-1;
    for(std::size_t i=0;i<work.nodes.size();i++){
        const float dist=originalVU0DistanceXYZ_21b234(
            work.nodes[i].centroid,query);
        if(best<dist){best=dist;index=static_cast<std::int32_t>(i);}
    }
    return index;
}
std::vector<const OriginalSpatialRecord70*> originalNavRouteExtract_220994(
    const OriginalNavRouteWorkspace& work,
    std::int32_t destination) {
    if(destination==-1)return {};
    std::vector<const OriginalSpatialRecord70*> reversed;
    while(destination!=-1){
        if(destination<0||
           static_cast<std::size_t>(destination)>=work.nodes.size()||
           reversed.size()>=work.nodes.size())
            throw std::runtime_error("Invalid or cyclical original predecessor");
        const auto& node=work.nodes[static_cast<std::size_t>(destination)];
        reversed.push_back(node.polygon04);
        if(destination==0)break;
        destination=node.predecessor30;
    }
    if(reversed.empty()||reversed.back()!=work.nodes.front().polygon04)
        throw std::runtime_error("Original route does not reach record zero");
    std::reverse(reversed.begin(),reversed.end());
    return reversed;
}
} // namespace ae3
