#include "ae3/original_nav_linked_owner.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void w16(std::vector<std::byte>& b,std::size_t at,std::uint16_t n){b.at(at)=std::byte(n&255);b.at(at+1)=std::byte(n>>8);}
static void w32(std::vector<std::byte>& b,std::size_t at,std::uint32_t n){for(int i=0;i<4;i++)b.at(at+i)=std::byte((n>>(8*i))&255);}
struct Eligibility:OriginalNavPopulationPredicate{
    std::vector<float> positions;
    float call_original_21ef3c(const OriginalSpatialRecord70&,const OriginalVU0Vec4& center)override{
        positions.push_back(center.x);
        return center.x==1.0f?1.0f:0.0f;
    }
};
int main(){
    std::array<OriginalSpatialRecord70,4> records{};
    std::vector<std::byte> data(0x80 + 4*0x1e);
    w32(data,0x24,4);w32(data,0x3c,0x80);
    for(std::size_t i=0;i<4;i++){
        w16(data,0x80 + i*0x1e + 0x16,1);
        const auto bits=std::bit_cast<std::uint32_t>(static_cast<float>(i));
        for(unsigned j=0;j<4;j++) records[i].originalFields[0x40+j]=std::byte((bits>>(8*j))&255);
    }
    w16(data,0x80 + 2,1);w16(data,0x80 + 4,1);
    w16(data,0x80 + 0x1e + 2,1);w16(data,0x80 + 0x1e + 4,2);
    w16(data,0x80 + 2*0x1e + 2,1);w16(data,0x80 + 2*0x1e + 4,3);
    OriginalNavListOwner membership,fallback;
    membership.originalCreate_21f604();fallback.originalCreate_21f604();
    auto* list=membership.originalList();
    ck(list->debugOriginalSentinelValid(),"original circular sentinel invalid");
    ck(list->originalInsertUnique_21f82c(&records[0]),"new polygon insertion failed");
    ck(!list->originalInsertUnique_21f82c(&records[0]),"source dedup failed");
    ck(list->originalInsertUnique_21f82c(&records[2]),"second polygon insert");
    ck(list->originalInsertUnique_21f82c(&records[3]),"third polygon insert");
    ck(list->originalOrderedValues()==std::vector<const OriginalSpatialRecord70*>({&records[3],&records[2],&records[0]}),
       "original 0x21F82C inserts before first link, not after");
    ck(list->debugOriginalSentinelValid(),"original next/prev links invalid");
    ck(list->originalErase_21f804(&records[2])==1,"original erase failed");
    ck(list->originalErase_21f804(&records[2])==0,"duplicate erase changed topology");
    ck(list->originalOrderedValues()==std::vector<const OriginalSpatialRecord70*>({&records[3],&records[0]}),
       "original source list erasure order");
    list->originalClear_4e4154();
    ck(list->size()==0&&list->debugOriginalSentinelValid(),"original list clear invalid sentinel");
    Eligibility predicate;
    ck(originalNavPopulateList_21f6d4(membership,records,data,predicate)==3,"original float-zero admission");
    ck(predicate.positions==std::vector<float>({0,1,2,3}),"original 0x21F6D4 sequential scan order");
    ck(list->originalOrderedValues()==std::vector<const OriginalSpatialRecord70*>({&records[3],&records[2],&records[0]}),
       "original eligibility list reverse source scan");
    ck(originalNavPopulateList_21f6d4(membership,records,data,predicate)==0,"original repeated population must deduplicate");
    ck(fallback.originalList()->originalInsertUnique_21f82c(&records[3]),"fallback insertion");
    ck(fallback.originalList()->originalInsertUnique_21f82c(&records[2]),"fallback prepend");
    OriginalNavDispatchRequest query{};
    query.sourceSeedIndex=1;query.maximumDistance=10;query.routeMode=0;
    query.originalReference={1.5f,0,0,0};
    const auto result=originalNavRouteDispatchOwned_2207d0(records,data,query,membership);
    ck(result.fallbackUsed,"original owner list membership fallback not detected");
    ck(result.fallbackPolygon==&records[2],"original same-owner fallback nearest based on source order");
    ck(result.selectedEndpoint>=0,"original owner dispatcher produced no endpoint");
    membership.originalDispose_21f64c();
    ck(!membership.originalList(),"original 0x21F64C owner retained destroyed sentinel");
    fallback.originalDispose_21f690();
    ck(!fallback.originalList(),"original 0x21F690 owner retained destroyed sentinel");
    bool error=false;
    try{(void)originalNavPopulateList_21f6d4(membership,records,data,predicate);}
    catch(const std::logic_error&){error=true;}
    ck(error,"uninitialized original owner silently accepted");
}
