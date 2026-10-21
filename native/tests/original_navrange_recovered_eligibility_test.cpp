#include "ae3/original_navrange_recovered_eligibility.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool value){if(!value)throw std::runtime_error("Original navrange F20 source regression");}
static std::uint32_t bits(float x){return std::bit_cast<std::uint32_t>(x);}
static bool near(float a,float b,float d=0.003f){return std::fabs(a-b)<d;}
static void w32(std::span<std::byte> out,std::size_t at,std::uint32_t value){
    for(unsigned i=0;i<4;i++)out[at+i]=std::byte((value>>(i*8))&255u);
}
static void w16(std::span<std::byte> out,std::size_t at,std::uint16_t value){
    out[at]=std::byte(value&255u);out[at+1]=std::byte(value>>8);
}
int main(){
    OriginalNavrangeResourceFrame frame{};
    frame.argumentA0_20={bits(10.f),bits(20.f),bits(30.f),bits(0.f)};
    frame.quaternionA2_30={bits(0.f),bits(0.f),bits(0.f),bits(1.f)};
    frame.argumentA1_40={bits(2.f),bits(3.f),bits(4.f),bits(1.f)};
    const auto basis=originalNavrangeExtentBasis_21effc_21f178(frame.argumentA1_40);
    ck(basis[0].x==2.f&&basis[0].y==0.f&&basis[0].z==0.f);
    ck(basis[1].x==0.f&&basis[1].y==3.f&&basis[1].z==0.f);
    ck(basis[2].x==0.f&&basis[2].y==0.f&&basis[2].z==4.f);
    auto inside=originalNavrangeScore_21ef3c(frame,{11.f,22.f,33.f,1.f});
    ck(near(inside.projectedCoordinates[0],1.f));
    ck(near(inside.projectedCoordinates[1],2.f));
    ck(near(inside.projectedCoordinates[2],3.f));
    ck(inside.f20==0.f);
    auto outside=originalNavrangeScore_21ef3c(frame,{13.f,15.f,37.f,1.f});
    ck(near(outside.f20,14.f));
    auto edge=originalNavrangeScore_21ef3c(frame,{12.f,17.f,34.f,1.f});
    ck(edge.f20==0.f);
    // 0x21EF5C and 0x21EF9C are full-width POR copies, not
    // upper-half PCPYUD shuffles: quaternion X/Y must NOT be lost.
    auto tilted=frame;tilted.quaternionA2_30[0]=bits(0.4f);
    tilted.quaternionA2_30[1]=bits(-0.6f);
    const auto other=originalNavrangeScore_21ef3c(tilted,{13.f,15.f,37.f,1.f});
    ck(!near(outside.f20,other.f20));
    auto rotated=frame;
    rotated.quaternionA2_30[2]=bits(0.70710678118f);
    rotated.quaternionA2_30[3]=bits(0.70710678118f);
    const auto yaw=originalNavrangeScore_21ef3c(rotated,{10.f,23.f,30.f,1.f});
    ck(near(yaw.localCentroid.x,3.f)&&near(yaw.localCentroid.y,0.f));
    ck(near(yaw.f20,1.f));
    auto zero=frame;zero.argumentA1_40[0]=bits(0.f);
    ck(std::isnan(originalNavrangeScore_21ef3c(
        zero,{11.f,20.f,30.f,1.f}).projectedCoordinates[0]));
    for(int i=1;i<=1000;i++){
        const float delta=static_cast<float>(i)*0.125f;
        const float beyond=delta-2.f;
        const auto result=originalNavrangeScore_21ef3c(
            frame,{10.f+delta,20.f,30.f,1.f});
        ck(near(result.f20,delta<=2.f?0.f:beyond*beyond,0.06f));
    }
    // Real keyed owner population can now use an explicit native
    // source-derived callback instead of a fixture-only predicate.
    std::array<std::byte,0x64> header{};
    constexpr char magic[]="navrange";
    for(int i=0;i<8;i++)header[i]=std::byte(static_cast<unsigned char>(magic[i]));
    w32(header,0x30+3*4,bits(1.f)); // quaternion identity
    w32(header,0x40,bits(2.f));w32(header,0x44,bits(3.f));
    w32(header,0x48,bits(4.f));
    w32(header,0x54,1);w32(header,0x5c,0x650000u);
    std::array<std::byte,0x20> key{};key[0]=std::byte{0x23};
    const std::array<OriginalNavGroupAllocation,1> alloc{{{0x650000u,key}}};
    OriginalNavGroupSource source{header,alloc};
    std::array<OriginalSpatialRecord70,3> records{};
    std::array<std::byte,0x110> nav{};
    w32(nav,0x24,3);w32(nav,0x3c,0x80);
    for(int i=0;i<3;i++){
        w16(nav,0x80+0x1e*i+0x16,1);
        w32(records[i].originalFields,0x40,bits(float(i==2?5:i)));
    }
    OriginalNavGroupRegistry registry;
    const auto result=originalNavBuildGroupsRecovered_4e4060(
        registry,source,records,nav);
    ck(result.sourceGroups==1&&result.ownersCreated==1);
    ck(result.sourcePolygonsInserted==2);
    ck(registry.size()==1);
    const auto keys=originalNavGroupKeys_4e4060(source);
    auto* owner=registry.originalFind_36d6f4(keys[0]);
    ck(owner&&owner->originalList()->originalOrderedValues()==
        std::vector<const OriginalSpatialRecord70*>({&records[1],&records[0]}));
    const auto second=originalNavBuildGroupsRecovered_4e4060(
        registry,source,records,nav);
    ck(second.ownersCreated==0&&second.sourcePolygonsInserted==0);
    registry.originalClear_4e3fec();
}
