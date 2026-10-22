#include "ae3/original_resource_stage_sequence.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main() {
 try {
    const std::vector<VfiEntry> originals{
       {"debug/us/stage/woods_c/area.luc",100,20},
       {"debug/us/stage/woods_c/placeholder.plc",120,20},
       {"debug/us/stage/woods_c/navigation.pck.sz",140,20},
       {"debug/us/stage/woods_c/actionseq.asq",160,20},
       {"debug/us/stage/woods_c/monkeydb.xml",180,20},
       {"debug/us/stage/woods_c/other/navigATion.pck.sz",200,20},
       {"debug/us/stage/another_c/actionseq.asq",220,20}
    };
    OriginalStageResourceCatalog catalog(originals);
    const auto found=originalStageLookupCandidates_22d7b4(catalog,"woods_c");
    const std::array<const char*,4> names{"placeholder.plc","navigation.pck.sz",
                                          "actionseq.asq","monkeydb.xml"};
    const std::array<std::uint32_t,4> callsites{0x22d828,0x22d8c0,0x22d8e8,0x22d928};
    const std::array<std::uint32_t,4> consumers{0x272294,0x4f0a5c,0x30c798,0x25bfe0};
    for(std::size_t i=0;i<found.size();++i){
       ck(found[i].source.originalLookupCallVA==callsites[i],"Original stage order/call VA");
       ck(found[i].source.originalConsumerVA==consumers[i],"Original consumer VA");
       ck(found[i].actualVfiIndices.size()==1,"Only one actual source member per key");
       const auto& original=originalStageRequireUniqueCandidate_22d7b4(catalog,found[i]);
       ck(original.stageRelativePath==names[i],"Never fabricate resource extensions");
       ck(original.sourceEntry.offset==120+i*20,"Original VFI source offset changed");
    }
    const auto absent=originalStageLookupCandidates_22d7b4(catalog,"another_c");
    ck(absent[0].actualVfiIndices.empty() &&
       absent[1].actualVfiIndices.empty() &&
       absent[2].actualVfiIndices.size()==1 &&
       absent[3].actualVfiIndices.empty(),"No guessed missing stage entries");
    bool rejected=false;
    try{(void)originalStageRequireUniqueCandidate_22d7b4(catalog,absent[0]);}
    catch(const std::out_of_range&){rejected=true;}
    ck(rejected,"Missing original resource source incorrectly accepted");
    auto ambiguous=originals;
    ambiguous.push_back({"debug/us/stage/woods_c/navigation.pck",240,20});
    OriginalStageResourceCatalog duplicateCatalog(ambiguous);
    const auto candidates=originalStageLookupCandidates_22d7b4(duplicateCatalog,"woods_c");
    ck(candidates[1].actualVfiIndices.size()==2,"Actual ambiguous source resources not retained");
    rejected=false;
    try{(void)originalStageRequireUniqueCandidate_22d7b4(duplicateCatalog,candidates[1]);}
    catch(const std::out_of_range&){rejected=true;}
    ck(rejected,"Ambiguous source key must not be guessed");
    std::cout<<"ORIGINAL_STAGE_INIT_SEQUENCE_TEST_PASSED lookups=4 candidates=source-backed ambiguity=reject\n";
    return 0;
 } catch(const std::exception& e){
    std::cerr<<"ORIGINAL_STAGE_INIT_SEQUENCE_TEST_FAILED: "<<e.what()<<'\n';return 1;
 }
}
