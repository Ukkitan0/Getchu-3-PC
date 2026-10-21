#include "ae3/original_navrange_group_binding.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool v){if(!v)throw std::runtime_error("original navrange resource-binding regression");}
static void w32(std::span<std::byte> data,std::size_t p,std::uint32_t v){
    for(unsigned i=0;i<4;i++)data[p+i]=std::byte((v>>(i*8))&255u);
}
static void w16(std::span<std::byte> data,std::size_t p,std::uint16_t v){
    data[p]=std::byte(v&255u);data[p+1]=std::byte(v>>8);
}
struct GroundedCallback final : OriginalNavrangeSourceEligibility {
    std::size_t calls=0;
    float call_original_21ef3c(const OriginalNavGroupKey20& key,
        const OriginalSpatialRecord70&,const OriginalVU0Vec4& center,
        const OriginalNavrangeResourceFrame& resource) override {
        ++calls;
        ck(key.originalBytes[0]==std::byte{0x10}||key.originalBytes[0]==std::byte{0x20});
        ck(resource.argumentA0_20[0]==0x3f000000u);
        ck(resource.argumentA0_20[1]==0x7fc01234u);
        ck(resource.quaternionA2_30[0]==0u&&resource.quaternionA2_30[3]==0x3f800000u);
        ck(resource.argumentA1_40[0]==0x40000000u);
        ck(center.x==0.f||center.x==1.f);
        const auto inverse=originalNavrangeFrameInverse_21ef9c_21efdc(resource);
        ck(inverse[0].x==1.f&&inverse[1].y==1.f&&inverse[2].z==1.f);
        return 0.0f; // UNIT TEST ONLY; never used as production eligibility default
    }
};
int main(){
    std::array<std::byte,0x64> header{};
    const char* magic="navrange";
    for(unsigned i=0;i<8;i++)header[i]=std::byte(static_cast<unsigned char>(magic[i]));
    w32(header,0x20,0x3f000000u);
    w32(header,0x24,0x7fc01234u);
    w32(header,0x30,0u);w32(header,0x3c,0x3f800000u);
    w32(header,0x40,0x40000000u);
    w32(header,0x54,3);w32(header,0x5c,0x620000u);
    std::array<std::byte,0x60> keys{};
    keys[0]=std::byte{0x10};keys[0x20]=std::byte{0x20};keys[0x40]=std::byte{0x10};
    std::array<OriginalNavGroupAllocation,1> allocations{{{0x620000,keys}}};
    const OriginalNavGroupSource source{header,allocations};
    std::array<OriginalSpatialRecord70,2> records{};
    std::array<std::byte,0xc0> nav{};
    w32(nav,0x24,2);w32(nav,0x3c,0x80);
    for(unsigned i=0;i<2;i++){
        w16(nav,0x80+0x1e*i+0x16,1);
        w32(records[i].originalFields,0x40,
             std::bit_cast<std::uint32_t>(static_cast<float>(i)));
    }
    OriginalNavGroupRegistry registry;
    GroundedCallback predicate;
    const auto result=originalNavBuildGroupsSourceAware_4e4060(
        registry,source,records,nav,predicate);
    ck(result.sourceGroups==3 && result.ownersCreated==2);
    ck(result.sourcePolygonsInserted==4 && predicate.calls==6);
    ck(registry.size()==2);
    const auto again=originalNavBuildGroupsSourceAware_4e4060(
        registry,source,records,nav,predicate);
    ck(again.ownersCreated==0&&again.sourcePolygonsInserted==0&&predicate.calls==12);
    const auto before=predicate.calls;
    header[0]=std::byte{0};
    const auto skipped=originalNavBuildGroupsSourceAware_4e4060(
        registry,source,records,nav,predicate);
    ck(skipped.sourceGroups==0&&predicate.calls==before);
    header[0]=std::byte{'n'};
    w32(header,0x54,0);
    const auto zero=originalNavBuildGroupsSourceAware_4e4060(
        registry,source,records,nav,predicate);
    ck(zero.sourceGroups==0&&predicate.calls==before);
    registry.originalClear_4e3fec();
}
