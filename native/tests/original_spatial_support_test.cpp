#include "ae3/original_spatial_support.hpp"
#include <cmath>
#include <cstdint>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
using namespace ae3;
static void check(bool ok,std::source_location source=std::source_location::current()) {
    if(!ok)throw std::runtime_error("Original spatial regression at "+std::to_string(source.line()));
}
struct Scoring final:OriginalSpatialEngineCalls {
    const OriginalSpatialRecord70* base=nullptr;
    std::vector<std::size_t> checked,scored;
    bool call_original_21b788_eligibility(const OriginalSpatialRecord70& r) override {
        const auto i=static_cast<std::size_t>(&r-base);
        checked.push_back(i);return i!=3;
    }
    float call_original_21b468_and_VU_distance(const OriginalSpatialRecord70& r) override {
        const auto i=static_cast<std::size_t>(&r-base);
        scored.push_back(i);return i==2?2.f:7.f;
    }
};
struct Polyline final:OriginalPolylineMetric {
    std::vector<std::pair<int,int>> steps;
    float call_original_VU0_consecutive_distance(
        const std::array<std::byte,16>& a,
        const std::array<std::byte,16>& b) override {
        int i=std::to_integer<int>(a[0]),j=std::to_integer<int>(b[0]);
        steps.emplace_back(i,j);
        return i==1?2.5f:1.25f;
    }
};
struct Release final:OriginalSpatialRecordReleaseCalls {
    void *a=nullptr,*b=nullptr,*c=nullptr;
    std::vector<std::string> calls;
    void call_original_41a3d8(void* ptr) override {
        check(ptr==reinterpret_cast<void*>(0xaa)||ptr==reinterpret_cast<void*>(0xcc));
        calls.push_back(ptr==reinterpret_cast<void*>(0xaa)?"data-A":"data-C");
    }
    void call_original_41a3b0(void* ptr) override {
        check(ptr==a||ptr==b||ptr==c);
        calls.push_back(ptr==a?"free-A":ptr==b?"free-B":"free-C");
    }
};
int main() {
    const std::array<std::uint16_t,6> offsets{0,4,8,12,16,20};
    const std::array<std::uint16_t,24> pool{1,0,0,0, 2,1,3,0, 2,1,2,0,
        0,0,0,0, 1,3,0,0, 1,0,0,0};
    OriginalSpatialGridFields grid{};
    grid.originX00=-10.f;grid.originY04=20.f;
    grid.stepX10=5.f;grid.stepY14=10.f;
    grid.dimensionX18=3;grid.dimensionY1c=2;
    grid.cellOffsets50=offsets;grid.hitBuffer08=pool;
    auto hits=originalSpatialGridLookup_4e3c14(grid,-3.f,21.f);
    check(hits.originalRecordIndices.size()==2);
    check(hits.originalRecordIndices[0]==1&&hits.originalRecordIndices[1]==3);
    hits=originalSpatialGridLookup_4e3c14(grid,-8.f,25.f);
    check(hits.originalRecordIndices.size()==1&&hits.originalRecordIndices[0]==0);
    hits=originalSpatialGridLookup_4e3c14(grid,1000.f,1000.f);
    check(hits.originalRecordIndices.size()==1&&hits.originalRecordIndices[0]==0);
    hits=originalSpatialGridLookup_4e3c14(grid,-10.f,30.f);
    check(hits.originalRecordIndices.empty());
    std::vector<OriginalSpatialRecord70> records(4);
    Scoring scoring{};scoring.base=records.data();
    check(originalSpatialNearestFromGrid_21d898(records,grid,1.f,20.f,scoring)==&records[2]);
    check(scoring.checked==std::vector<std::size_t>({1,2}));
    check(scoring.scored==std::vector<std::size_t>({1,2}));
    bool rejected=false;
    try{(void)originalSpatialGridLookup_4e3c14(grid,std::nanf(""),20.f);}
    catch(const std::domain_error&){rejected=true;}check(rejected);
    auto invalid=grid;invalid.cellOffsets50={};rejected=false;
    try{(void)originalSpatialGridLookup_4e3c14(invalid,0,0);}
    catch(const std::out_of_range&){rejected=true;}check(rejected);
    check(originalSpatialPlaneEligible_21b788({}));
    const std::array<OriginalSpatialPlaneComparison,3> inside{{{3,3},{5,4},{0,0}}};
    const std::array<OriginalSpatialPlaneComparison,3> outside{{{10,2},{1,2},{10,1}}};
    check(originalSpatialPlaneEligible_21b788(inside));
    check(!originalSpatialPlaneEligible_21b788(outside));
    check(!originalSpatialPlaneEligible_21b788(inside,-2.f));
    const std::array<OriginalSpatialPlaneComparison,1> nanCase{{{std::nanf(""),1}}};
    check(originalSpatialPlaneEligible_21b788(nanCase));
    check(originalSpatialReciprocalPlaneCount_21b468(1)==1.f);
    check(originalSpatialReciprocalPlaneCount_21b468(2)==0.5f);
    check(originalSpatialReciprocalPlaneCount_21b468(3)==1.f/3.f);
    rejected=false;
    try{(void)originalSpatialReciprocalPlaneCount_21b468(0);}
    catch(const std::domain_error&){rejected=true;}check(rejected);
    std::array<std::array<std::byte,16>,3> source{};
    for(std::size_t i=0;i<3;i++)source[i][0]=std::byte(i+1);
    OriginalSpatialPointArray points{source};
    check(originalSpatialPointCount_21b178(points)==3);
    check(originalSpatialPoint_21b180(points,2)[0]==std::byte{3});
    Polyline metric{};
    check(originalSpatialPolylineLength_21b194(points,metric)==3.75f);
    check(metric.steps==std::vector<std::pair<int,int>>({{1,2},{2,3}}));
    metric.steps.clear();
    check(originalSpatialPolylineLength_21b194(OriginalSpatialPointArray{},metric)==0.f);
    check(metric.steps.empty());
    OriginalSpatialChildOwnership a{reinterpret_cast<void*>(0xaa)},b{},
        c{reinterpret_cast<void*>(0xcc)};
    OriginalSpatialRecordOwnership record{};
    record.childPointers_04_08_0c={&a,&b,&c};record.subobjectAt20=&b;
    Release cleanup{};cleanup.a=&a;cleanup.b=&b;cleanup.c=&c;
    originalSpatialRecordDestroy_21b384(record,cleanup);
    check(cleanup.calls==std::vector<std::string>({
        "data-A","free-A","free-B","data-C","free-C"}));
    cleanup.calls.clear();
    OriginalSpatialRecordOwnership emptyRecord{};
    originalSpatialRecordDestroy_21b384(emptyRecord,cleanup);
    check(cleanup.calls.empty());
}
