#include "ae3/original_polygon_traversal.hpp"
#include <cstring>
#include <stdexcept>

namespace ae3 {
namespace {
OriginalVU0Vec4 readOriginalVertex(const OriginalSpatialRecord70& record,std::size_t slot) {
    if(slot>=3)throw std::out_of_range("Original polygon vector slot exceeds 2");
    OriginalVU0Vec4 value{};
    std::memcpy(&value,record.originalFields.data()+0x40+slot*0x10,0x10);
    return value;
}
struct Traversal {
    std::span<const OriginalSpatialRecord70> polygons;
    std::span<const OriginalPolygonLinks_21bd3c> links;
    OriginalPolygonEdgeIntersection_21ee00& intersection;
    OriginalPolygonTraverseResult_21bd3c result{};
    std::size_t maxCalls;
    std::optional<OriginalVU0Vec4> recurse(
        std::uint32_t index,const OriginalVU0Vec4& origin,
        const OriginalVU0Vec4& direction,float remaining) {
        if(result.sourcePolygonsTested>=maxCalls)
            throw std::runtime_error("Original source polygon recursion requires owner-provided larger safety budget");
        if(index>=polygons.size())
            throw std::out_of_range("Original source polygon recursion index outside stage");
        ++result.sourcePolygonsTested;
        const auto& polygon=polygons[index];
        const auto a=readOriginalVertex(polygon,0);
        const auto b=readOriginalVertex(polygon,1);
        const auto c=readOriginalVertex(polygon,2);
        OriginalPolygonRayDistanceBounds_21bd3c bounds{0.0f,remaining};
        // Original 0x21BD8C, 0x21BDA0, 0x21BDB4: three calls
        // strictly short-circuit on false; packet carries updates.
        ++result.sourceEdgeTests;
        if(!intersection.call_original_21ee00(a,b,origin,direction,bounds))return {};
        ++result.sourceEdgeTests;
        if(!intersection.call_original_21ee00(b,c,origin,direction,bounds))return {};
        ++result.sourceEdgeTests;
        if(!intersection.call_original_21ee00(c,a,origin,direction,bounds))return {};

        const float left=remaining-bounds.originalUpperAt24;
        // 0x21BDD0..0x21BDE8: source VMULx + VADD all 4 lanes.
        const OriginalVU0Vec4 advanced{
            origin.x+direction.x*bounds.originalUpperAt24,
            origin.y+direction.y*bounds.originalUpperAt24,
            origin.z+direction.z*bounds.originalUpperAt24,
            origin.w+direction.w*bounds.originalUpperAt24};
        // 0x21BDF4 writes original polygon identity before recursion.
        result.originalLastAcceptedPolygon=&polygon;
        if(!(left>0.0f))return advanced;

        const auto& neighborLinks=links[index];
        if(neighborLinks.edgeCount>3)
            throw std::out_of_range("Original 0x21B3F8 source degree exceeds 3");
        for(unsigned slot=0;slot<neighborLinks.edgeCount;++slot){
            const auto edge=neighborLinks.edges[slot];
            // 0x21BE48 -> 0x21B444; nonzero action kind skipped.
            if(edge.originalActionKind!=0)continue;
            ++result.sourceZeroKindRecursions;
            const auto found=recurse(edge.neighborSourceIndex,advanced,direction,left);
            if(found)return found;
        }
        return {};
    }
};
}
OriginalPolygonTraverseResult_21bd3c originalPolygonTraverse_21bd3c(
    std::span<const OriginalSpatialRecord70> polygons,
    std::span<const OriginalPolygonLinks_21bd3c> links,
    std::uint32_t sourceIndex,
    const OriginalVU0Vec4& origin,
    const OriginalVU0Vec4& direction,
    float remaining,
    OriginalPolygonEdgeIntersection_21ee00& callback,
    std::size_t maxCalls) {
    if(polygons.size()!=links.size())
        throw std::invalid_argument("Original 0x21BD3C source record/header counts must match");
    if(sourceIndex>=polygons.size())
        throw std::out_of_range("Original polygon source index invalid");
    if(maxCalls==0)throw std::invalid_argument("Native recursion guard must be nonzero");
    Traversal walker{polygons,links,callback,{},maxCalls};
    walker.result.sourceHitPoint=walker.recurse(sourceIndex,origin,direction,remaining);
    return walker.result;
}
} // namespace ae3
