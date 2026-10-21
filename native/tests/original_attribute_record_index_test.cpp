#include "ae3/original_attribute_record_index.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool v){if(!v)throw std::runtime_error("source record stride/index mismatch");}
int main(){
    const OriginalAttributeRecordIndex_2bb0f4 original{0x1000,4,0xB0};
    ck(originalAttributeRecordGuestAddress_2bb0f4(original,0)==0x1000u);
    ck(originalAttributeRecordGuestAddress_2bb0f4(original,1)==0x10b0u);
    ck(originalAttributeRecordGuestAddress_2bb0f4(original,3)==0x1210u);
    ck(originalAttributeRecordGuestAddress_2bb0f4({0xfffffffcu,1,16},1)==0xcu);
    ck(originalAttributeRecordGuestAddress_2bb0f4({100u,3,-4},2)==92u);
    std::vector<std::byte> source(4*0xb0);
    source[0x160]=std::byte{0x31};
    source[0x160+1]=std::byte{0x32};
    const auto rec=originalAttributeRecordBytesChecked_2bb0f4(source,0x1000u,original,2,2);
    ck(rec.size()==2&&rec[0]==std::byte{0x31}&&rec[1]==std::byte{0x32});
    bool threw=false;
    try{(void)originalAttributeRecordBytesChecked_2bb0f4(source,0x1000u,original,4,1);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    threw=false;
    try{(void)originalAttributeRecordBytesChecked_2bb0f4(source,0x1000u,original,3,0xb1);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    threw=false;
    try{(void)originalAttributeRecordBytesChecked_2bb0f4(source,0x1001u,original,0,1);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    // End-to-end: original raw input -> parser owner fields -> index.
    OriginalAttributeTreeOwner decoded;
    const auto parsed=originalAttributeBindLoadedText_2bb03c(
        decoded,"meta\nstn:A:1:2\nS:0:name\nstride:176\nbase:-8\n",0x1008u);
    const auto layout=originalAttributeRecordIndexFromParser_2bb0f4(parsed.parsed);
    ck(layout.sourceCountAt20==2&&layout.sourceStrideAt24==176);
    ck(layout.sourceRecordBaseAt04==0x1000u);
    ck(originalAttributeRecordGuestAddress_2bb0f4(layout,1)==0x10b0u);
    for(std::uint32_t i=0;i<2000;++i){
        const auto idx=OriginalAttributeRecordIndex_2bb0f4{0x2000u,1,7};
        ck(originalAttributeRecordGuestAddress_2bb0f4(idx,i)==0x2000u+i*7u);
    }
}
