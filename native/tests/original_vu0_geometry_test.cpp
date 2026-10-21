#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
struct Plane final:OriginalSpatialOriginalEligibility {
    const OriginalSpatialRecord70* records=nullptr;
    std::vector<std::size_t> calls;
    bool call_original_21b788_planeEligibility(const OriginalSpatialRecord70& r) override {
        const auto i=static_cast<std::size_t>(&r-records);
        calls.push_back(i);return i!=1;
    }
};
int main(){
    const std::array<OriginalVU0Vec4,3> points{{{0,0,0,2},{3,4,0,6},{3,4,12,10}}};
    const auto avg=originalVU0Mean_21b468(points);
    check(avg.x==2.f&&avg.y==8.f/3.f&&avg.z==4.f&&avg.w==6.f,
          "Original VADD/DIV/VMULx centroid wrong");
    check(originalVU0SquaredDistanceXYZ_21b210(points[0],points[1])==25.f,
          "Original VU squared XYZ distance wrong");
    check(originalVU0DistanceXYZ_21b234(points[0],points[1])==5.f,
          "Original VU SQRT.S distance wrong");
    check(originalVU0DistanceXYZ_21b234(points[1],points[2])==12.f,
          "Original second VU distance wrong");
    check(originalVU0PolylineLength_21b194(points)==17.f,
          "Original consecutive-point path length wrong");
    check(originalVU0PolylineLength_21b194({})==0.f,
          "Empty original path length must be zero");
    check(originalVU0PolylineLength_21b194(
          std::span<const OriginalVU0Vec4>(points).first(1))==0.f,
          "One-point original path length must be zero");
    const std::array<OriginalVU0Vec4,2> ring{{{0,0,0,0},{1,1,0,0}}};
    check(originalVU0PolygonEligible_21b788({},points[0]),
          "Zero-plane original polygon must pass");
    check(originalVU0PolygonEligible_21b788(ring,{0.5f,0,0,0}),
          "Original packed MMI/VU0 plane rejected inside");
    check(originalVU0PolygonEligible_21b788(ring,{0,0,0,0}),
          "Original plane boundary changed");
    check(!originalVU0PolygonEligible_21b788(ring,{-1,0,0,0}),
          "Original plane failed negative side rejection");
    check(!originalVU0PolygonEligible_21b788(ring,{2,0,0,0}),
          "Original plane failed positive side rejection");
    check(originalVU0PolygonEligible_21b788(ring,{-0.1f,0,0,0},0.2f),
          "Original f12 tolerance ignored");
    check(originalVU0DistanceXYZ_21b234({5,-3,2,1e8f},{5,-3,2,-1e8f})==0.f,
          "Original distance incorrectly includes W");
    const auto nan=std::numeric_limits<float>::quiet_NaN();
    check(std::isnan(originalVU0DistanceXYZ_21b234({nan,0,0,0},{0,0,0,0})),
          "Original NaN propagation changed");
    bool failed=false;
    try{(void)originalVU0Mean_21b468({});}catch(const std::domain_error&){failed=true;}
    check(failed,"Original zero-count FPU behavior must not be guessed");
    std::array<OriginalSpatialRecord70,4> records{};
    const std::array<OriginalVU0Vec4,1> p0{{{0,0,0,1}}},p1{{{10,0,0,1}}},p3{{{9,0,0,1}}};
    const std::array<OriginalVU0Vec4,2> p2{{{3,4,0,0},{3,4,0,4}}};
    const std::array<OriginalSpatialSourcePolygon,4> polygons{{{p0},{p1},{p2},{p3}}};
    const std::array<std::uint16_t,4> order{{1,2,0,3}};
    Plane plane{};plane.records=records.data();
    const OriginalVU0Vec4 query{2,3,0,888};
    check(originalSpatialNearestWithVU0_21d898(records,polygons,
          OriginalSpatialGridHits{order},query,plane)==&records[2],
          "Original nearest grid math failed");
    check(plane.calls==std::vector<std::size_t>({1,2,0,3}),
          "Original plane-candidate order changed");
    check(originalSpatialNearestAllWithVU0_21da2c(records,polygons,query)==&records[2],
          "Original direct registry score failed");
    const std::array<OriginalVU0Vec4,2> q0{{{0,0,0,0},{2,2,0,0}}};
    const std::array<OriginalVU0Vec4,2> q1{{{9,0,0,0},{11,2,0,0}}};
    const std::array<OriginalVU0Vec4,2> q2{{{3,0,0,0},{4,1,0,0}}};
    const std::array<OriginalVU0Vec4,2> q3{{{1,0,0,0},{3,2,0,0}}};
    const std::array<OriginalSpatialSourcePolygon,4> sourcePolygons{{{q0},{q1},{q2},{q3}}};
    check(originalSpatialNearestGeometry_21d898(records,sourcePolygons,
          OriginalSpatialGridHits{order},{3.4f,0,0,0})==&records[2],
          "Fully original plane/mean/XYZ distance selector failed");
    check(originalSpatialNearestGeometry_21d898(records,sourcePolygons,
          OriginalSpatialGridHits{std::span<const std::uint16_t>{}},query)==nullptr,
          "Original empty hit-list must return null");
    failed=false;
    try{(void)originalSpatialNearestAllWithVU0_21da2c(records,
        std::span<const OriginalSpatialSourcePolygon>(polygons).first(2),query);}
    catch(const std::invalid_argument&){failed=true;}
    check(failed,"Mismatched source geometry must not be accepted");
}
