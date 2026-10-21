#include "ae3/original_nav_group_registry.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok){if(!ok)throw std::runtime_error("original NavGraph group source test failed");}
static void w32(std::span<std::byte> bytes,std::size_t o,std::uint32_t x){
    for(int i=0;i<4;i++)bytes[o+i]=std::byte((x>>(8*i))&255u);
}
static void w16(std::span<std::byte> bytes,std::size_t o,std::uint16_t x){
    bytes[o]=std::byte(x&255u);bytes[o+1]=std::byte(x>>8);
}
struct GroupPredicate final:OriginalNavGroupEligibility {
    int calls=0;
    float call_original_21ef3c(const OriginalNavGroupKey20& key,
        const OriginalSpatialRecord70&,const OriginalVU0Vec4& center) override {
        calls++;
        // Unit-test fixture only: the actual 0x21EF3C remains unported.
        if(key.originalBytes[0]==std::byte{0x10})return center.x==1.f?1.f:0.f;
        return center.x==0.f?1.f:0.f;
    }
};
int main(){
    std::array<std::byte,0x64> source{};
    std::array<std::byte,0x60> keys{};
    constexpr char magic[]="navrange";
    for(std::size_t i=0;i<8;i++)
        source[i]=std::byte(static_cast<unsigned char>(magic[i]));
    source[8]=std::byte{0xfe}; // original mask ignores trailing bytes
    w32(source,0x54,3);w32(source,0x5c,0x610020);
    keys[0x00]=std::byte{0x10};keys[0x20]=std::byte{0x20};
    keys[0x40]=std::byte{0x10};
    const std::array<OriginalNavGroupAllocation,1> allocations{{{0x610020,keys}}};
    const OriginalNavGroupSource descriptor{source,allocations};
    ck(originalNavGroupHeaderMatches_4e4080(source));
    const auto parsed=originalNavGroupKeys_4e4060(descriptor);
    ck(parsed.size()==3&&parsed[0].originalBytes[0]==std::byte{0x10});
    ck(parsed[0]==parsed[2]&&!(parsed[0]==parsed[1]));
    std::array<OriginalSpatialRecord70,3> records{};
    std::array<std::byte,0x100> nav{};
    w32(nav,0x24,3);w32(nav,0x3c,0x80);
    for(std::size_t i=0;i<3;i++){
        w16(nav,0x80+0x1e*i+0x16,1);
        w32(records[i].originalFields,0x40,
            std::bit_cast<std::uint32_t>(static_cast<float>(i)));
    }
    OriginalNavGroupRegistry registry;
    GroupPredicate predicate;
    auto result=originalNavBuildGroups_4e4060(registry,descriptor,records,nav,predicate);
    ck(result.sourceGroups==3&&result.ownersCreated==2&&result.sourcePolygonsInserted==4);
    ck(registry.size()==2&&predicate.calls==9);
    auto* groupA=registry.originalFind_36d6f4(parsed[0]);
    auto* groupB=registry.originalFind_36d6f4(parsed[1]);
    ck(groupA&&groupB&&groupA!=groupB);
    ck(groupA->originalList()->originalOrderedValues()==
       std::vector<const OriginalSpatialRecord70*>({&records[2],&records[0]}));
    ck(groupB->originalList()->originalOrderedValues()==
       std::vector<const OriginalSpatialRecord70*>({&records[2],&records[1]}));
    ck(groupA->originalList()->debugOriginalSentinelValid()&&
       groupB->originalList()->debugOriginalSentinelValid());
    // Repeated source lookup reuses existing owners and list nodes.
    auto second=originalNavBuildGroups_4e4060(registry,descriptor,records,nav,predicate);
    ck(second.ownersCreated==0&&second.sourcePolygonsInserted==0&&registry.size()==2);
    ck(registry.originalFind_36d6f4(parsed[0])==groupA);
    registry.originalClear_4e3fec();ck(registry.size()==0);
    source[0]=std::byte{0};
    ck(!originalNavGroupHeaderMatches_4e4080(source));
    const auto gated=originalNavBuildGroups_4e4060(
        registry,{source,allocations},records,nav,predicate);
    ck(gated.sourceGroups==0&&registry.size()==0);
    source[0]=std::byte{static_cast<unsigned char>(magic[0])};
    bool caught=false;
    try{(void)originalNavGroupKeys_4e4060({source,{}});}
    catch(const std::out_of_range&){caught=true;}ck(caught);
    const std::array<OriginalNavGroupAllocation,2> ambiguous{{allocations[0],allocations[0]}};
    caught=false;
    try{(void)originalNavGroupKeys_4e4060({source,ambiguous});}
    catch(const std::invalid_argument&){caught=true;}ck(caught);
    w32(source,0x54,0xffff'ffffu);
    ck(originalNavGroupKeys_4e4060({source,{}}).empty());
    w32(source,0x54,0);
    ck(originalNavGroupKeys_4e4060({source,{}}).empty());
    caught=false;
    w32(source,0x54,999999);
    try{(void)originalNavGroupKeys_4e4060({source,allocations});}
    catch(const std::out_of_range&){caught=true;}ck(caught);
}
