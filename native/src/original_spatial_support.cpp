#include "ae3/original_spatial_support.hpp"
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
namespace {
// On R5900 original CVT.W.S means truncate toward zero, followed by
// signed clamps in source 0x4E3CC4..0x4E3CEC.
std::int32_t originalTruncAndClamp(float value,std::int32_t count) {
    if(count<=0 || std::isnan(value))
        throw std::domain_error("Unresolved invalid original grid coordinates");
    if(value<=0.0f)return 0;
    if(value>=static_cast<float>(count-1))return count-1;
    return static_cast<std::int32_t>(std::trunc(value));
}
}
OriginalSpatialGridHits originalSpatialGridLookup_4e3c14(
    const OriginalSpatialGridFields& g,float x,float y) {
    if(g.dimensionX18<=0||g.dimensionY1c<=0||
       g.stepX10==0.0f||g.stepY14==0.0f)
        throw std::domain_error("Invalid source-grid dimensions or spacing");
    // Original lwc1 grid+0,+4,+0x10,+0x14; div.s both axes.
    const float ny=(y-g.originY04)/g.stepY14;
    const float nx=(x-g.originX00)/g.stepX10;
    const auto iy=originalTruncAndClamp(ny,g.dimensionY1c);
    const auto ix=originalTruncAndClamp(nx,g.dimensionX18);
    const auto index=static_cast<std::size_t>(iy)*static_cast<std::size_t>(g.dimensionX18)+
                     static_cast<std::size_t>(ix);
    if(index>=g.cellOffsets50.size())
        throw std::out_of_range("Missing original grid cell map");
    // Original +0x50 LHU cell offset; shift left one, add pool at +8.
    const auto offset=static_cast<std::size_t>(g.cellOffsets50[index]);
    if(offset>=g.hitBuffer08.size())
        throw std::out_of_range("Original grid hit-header offset invalid");
    const auto count=static_cast<std::size_t>(g.hitBuffer08[offset]);
    if(count>g.hitBuffer08.size()-offset-1)
        throw std::out_of_range("Original grid hit list is truncated");
    return {g.hitBuffer08.subspan(offset+1,count)};
}
const OriginalSpatialRecord70* originalSpatialNearestFromGrid_21d898(
    std::span<const OriginalSpatialRecord70> records,
    const OriginalSpatialGridFields& grid,float x,float y,
    OriginalSpatialEngineCalls& original) {
    return originalSpatialNearest_21d898(
        records,originalSpatialGridLookup_4e3c14(grid,x,y),original);
}
bool originalSpatialPlaneEligible_21b788(
    std::span<const OriginalSpatialPlaneComparison> values,float originalF12) noexcept {
    // 0 planes -> 1. R5900 0x21B80C sub.s f0,f12, then
    // 0x21B81C c.olt.s f1,f0: if true return 0 immediately.
    for(const auto& v:values) {
        if(v.originalVU_f1<(v.originalVU_f0-originalF12))return false;
    }
    return true;
}
float originalSpatialReciprocalPlaneCount_21b468(std::uint16_t count) {
    // 0x21B4BC loads original 1.0f at 0x6E3E2C; 0x21B4C8 div.s.
    // 0's PS2-specific exceptional FPU semantics not yet verified.
    if(!count)throw std::domain_error("Unverified R5900 1/0 result");
    return 1.0f/static_cast<float>(count);
}
std::uint32_t originalSpatialPointCount_21b178(const OriginalSpatialPointArray& v) noexcept {
    return static_cast<std::uint32_t>(v.sourceVectors.size());
}
std::array<std::byte,16> originalSpatialPoint_21b180(
    const OriginalSpatialPointArray& v,std::size_t i) {
    if(i>=v.sourceVectors.size())throw std::out_of_range("Original point-array index");
    return v.sourceVectors[i];
}
float originalSpatialPolylineLength_21b194(
    const OriginalSpatialPointArray& points,OriginalPolylineMetric& original) {
    // 0x21B1B8 reads original count, loop while i < count-1.
    // Real 0x21B180 loads each adjacent LQ, real VU distance callback,
    // then 0x21B23C adds single-precision result into f20.
    float total=0.0f;
    for(std::size_t i=0;i+1<points.sourceVectors.size();i++) {
        total=total+original.call_original_VU0_consecutive_distance(
            originalSpatialPoint_21b180(points,i),
            originalSpatialPoint_21b180(points,i+1));
    }
    return total;
}
void originalSpatialChildDispose_21b14c(
    OriginalSpatialChildOwnership& child,OriginalSpatialRecordReleaseCalls& b) {
    if(child.originalAllocatedField04)
        b.call_original_41a3d8(child.originalAllocatedField04);
}
void originalSpatialSubobjectNoop_21afb0(void*) noexcept {}
void originalSpatialRecordDestroy_21b384(
    OriginalSpatialRecordOwnership& record,OriginalSpatialRecordReleaseCalls& b) {
    // 0x21B384 iterates exactly fields +4,+8,+12; child 0x21B14C
    // before original free 0x41A3B0. 0x21AFB0 is a real no-op.
    for(auto* child:record.childPointers_04_08_0c) {
        if(!child)continue;
        originalSpatialChildDispose_21b14c(*child,b);
        b.call_original_41a3b0(child);
    }
    originalSpatialSubobjectNoop_21afb0(record.subobjectAt20);
}
} // namespace ae3
