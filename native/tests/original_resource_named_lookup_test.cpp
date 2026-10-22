#include "ae3/original_resource_named_lookup.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>
using namespace ae3;
static void ck(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main() {
 try {
    const std::array<std::byte,4> originalPayload{{std::byte{11},std::byte{22},std::byte{33},std::byte{44}}};
    const std::array<OriginalStageNamedEntry16_24d4f0,6> rows{{
       {"placeholder",std::span(originalPayload).first(1),0},
       {"navigation", std::span(originalPayload).subspan(1,1),1},
       {"actionseq",  std::span(originalPayload).subspan(2,1),2},
       {"monkeydb",   std::span(originalPayload).subspan(3,1),3},
       {"navigation", std::span(originalPayload).subspan(3,1),4},
       {"other",      {},5}
    }};
    ck(originalStageNamedLookup_24d4f0(rows,"placeholder")==&rows[0],"Original first name match");
    ck(originalStageNamedLookup_24d4f0(rows,"navigation")==&rows[1],
       "Original duplicate-name search must return first source record");
    ck(originalStageNamedLookup_24d4f0(rows,"actionseq")==&rows[2],"Original order actionseq");
    ck(originalStageNamedLookup_24d4f0(rows,"monkeydb")==&rows[3],"Original order monkeydb");
    ck(originalStageNamedLookup_24d4f0(rows,"navigation")->borrowedOriginalPayload[0]==std::byte{22},
       "Original payload pointer at source entry+8");
    ck(originalStageNamedLookup_24d4f0(rows,"Navigation")==nullptr,"Original case-sensitive equality");
    ck(originalStageNamedLookup_24d4f0(rows,"nav")==nullptr,"Original full length not prefix");
    ck(originalStageNamedLookup_24d4f0(rows,"not-existing")==nullptr,"Original exhausted-vector null result");
    ck(originalStageNamedLookup_24d4f0(std::span<const OriginalStageNamedEntry16_24d4f0>{},
                                       "placeholder")==nullptr,"Original empty resource table");
    std::cout<<"ORIGINAL_STAGE_NAME_LOOKUP_TEST_PASSED original_va=0x24D4F0 first=1 exact=1 borrowed=1\n";
    return 0;
 }catch(const std::exception& e) {
    std::cerr<<"ORIGINAL_STAGE_NAME_LOOKUP_TEST_FAILED: "<<e.what()<<'\n';return 1;
 }
}
