#include "ae3/original_vu0_geometry.hpp"
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
OriginalVU0Vec4 originalVU0Mean_21b468(
    std::span<const OriginalVU0Vec4> points) {
    if(points.empty())
        throw std::domain_error("Original 0x21B468 DIV.S count=0 PS2 FPU behavior unverified");
    if(points.size()>65535)
        throw std::out_of_range("Original 0x21B468 count is unsigned 16-bit");
    OriginalVU0Vec4 accum{};
    // Original PPACW zeroes GPR2, QMTC2 zeroes VF2, VADD.xyzw in loop.
    for(const auto& point:points) {
        accum.x+=point.x; accum.y+=point.y;
        accum.z+=point.z; accum.w+=point.w;
    }
    const float reciprocal=1.0f/static_cast<float>(points.size());
    accum.x*=reciprocal; accum.y*=reciprocal;
    accum.z*=reciprocal; accum.w*=reciprocal;
    return accum;
}
float originalVU0SquaredDistanceXYZ_21b210(
    const OriginalVU0Vec4& left,const OriginalVU0Vec4& right) noexcept {
    const float dx=left.x-right.x;
    const float dy=left.y-right.y;
    const float dz=left.z-right.z;
    // Original VADDy.xyw then VADDz.xyzw: X+Y+Z; W is not used.
    float sum=dx*dx;
    sum=sum+dy*dy;
    sum=sum+dz*dz;
    return sum;
}
float originalVU0DistanceXYZ_21b234(
    const OriginalVU0Vec4& left,const OriginalVU0Vec4& right) noexcept {
    return std::sqrt(originalVU0SquaredDistanceXYZ_21b210(left,right));
}
bool originalVU0PolygonEligible_21b788(
    std::span<const OriginalVU0Vec4> points,
    const OriginalVU0Vec4& query,float f12) noexcept {
    if(points.empty())return true;
    // 0x21B7AC VMOVE VF2,VF3, VF3 initialized to final polygon vertex.
    OriginalVU0Vec4 previous=points.back();
    for(const auto& current:points) {
        // 0x21B7C4 loads directed source edge current-previous.
        // 0x21B7C0 creates original world-up VF1=(0,1,0,0).
        // 0x21B7D0..0x21B7DC shuffle original EE halfwords
        // before 0x21B7E0 VOPMULA and 0x21B7E4 VOPMSUB.
        // The source horizontal plane normal is up CROSS edge,
        // (edge.z, 0, -edge.x), *not* (edge.y,0,-edge.z).
        // Verify against authentic woods_c navigation triangles:
        // all 59 centroids pass and all 177 directed-edge outside
        // probes fail; the previous implementation passed 0/59.
        const float edgeX=current.x-previous.x;
        const float edgeZ=current.z-previous.z;
        const float normalX=edgeZ;
        const float normalZ=-edgeX;
        // Original VMUL.xyzw plus VADDy/VADDz dot reduction.
        const float dotPrevious=normalX*previous.x+normalZ*previous.z;
        const float dotQuery=normalX*query.x+normalZ*query.z;
        // 0x21B80C SUB.S f0,f0,f12; 0x21B81C C.OLT.S.
        if(dotQuery<(dotPrevious-f12))return false;
        // 0x21B844 VMOVE VF2,VF3.
        previous=current;
    }
    return true;
}
float originalVU0PolylineLength_21b194(
    std::span<const OriginalVU0Vec4> points) noexcept {
    float total=0.0f;
    for(std::size_t i=0;i+1<points.size();++i)
        total=total+originalVU0DistanceXYZ_21b234(points[i],points[i+1]);
    return total;
}
namespace {
class RecoveredVU0Adapter final:public OriginalSpatialEngineCalls {
public:
    RecoveredVU0Adapter(std::span<const OriginalSpatialRecord70> source,
                       std::span<const OriginalSpatialSourcePolygon> polygons,
                       const OriginalVU0Vec4& originalQuery,
                       OriginalSpatialOriginalEligibility* originalPlane)
        : source_(source),polygons_(polygons),
          query_(originalQuery),eligibility_(originalPlane) {
        if(source.size()!=polygons.size())
            throw std::invalid_argument("Original registry and polygon metadata count mismatch");
    }
    bool call_original_21b788_eligibility(const OriginalSpatialRecord70& record) override {
        if(eligibility_)return eligibility_->call_original_21b788_planeEligibility(record);
        const auto i=static_cast<std::size_t>(&record-source_.data());
        if(i>=polygons_.size())throw std::out_of_range("Missing original polygon");
        // Real 0x21D904 initializes f12 to exactly zero.
        return originalVU0PolygonEligible_21b788(polygons_[i].sourcePoints,query_,0.0f);
    }
    float call_original_21b468_and_VU_distance(const OriginalSpatialRecord70& record) override {
        const auto i=static_cast<std::size_t>(&record-source_.data());
        if(i>=polygons_.size())throw std::out_of_range("Missing original polygon");
        return originalVU0DistanceXYZ_21b234(
            originalVU0Mean_21b468(polygons_[i].sourcePoints),query_);
    }
private:
    std::span<const OriginalSpatialRecord70> source_;
    std::span<const OriginalSpatialSourcePolygon> polygons_;
    const OriginalVU0Vec4& query_;
    OriginalSpatialOriginalEligibility* eligibility_;
};
}
const OriginalSpatialRecord70* originalSpatialNearestWithVU0_21d898(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourcePolygon> polygons,
    OriginalSpatialGridHits hits,
    const OriginalVU0Vec4& query,
    OriginalSpatialOriginalEligibility& eligibility) {
    RecoveredVU0Adapter adapter(records,polygons,query,&eligibility);
    return originalSpatialNearest_21d898(records,hits,adapter);
}
const OriginalSpatialRecord70* originalSpatialNearestGeometry_21d898(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourcePolygon> polygons,
    OriginalSpatialGridHits hits,
    const OriginalVU0Vec4& query) {
    RecoveredVU0Adapter adapter(records,polygons,query,nullptr);
    return originalSpatialNearest_21d898(records,hits,adapter);
}
const OriginalSpatialRecord70* originalSpatialNearestAllWithVU0_21da2c(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourcePolygon> polygons,
    const OriginalVU0Vec4& query) {
    RecoveredVU0Adapter adapter(records,polygons,query,nullptr);
    return originalSpatialNearestAll_21da2c(records,adapter);
}
} // namespace ae3
