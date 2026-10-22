#include "ae3/original_resource_record_registration.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool test,const char* description){
    if(!test)throw std::runtime_error(description);
}
int main(){
    try {
        // Synthetic metadata and payload allocation: NO game bytes.
        const std::array<std::byte,8> sample{{
            std::byte{11},std::byte{22},std::byte{33},std::byte{44},
            std::byte{55},std::byte{66},std::byte{77},std::byte{88}
        }};
        const std::array<OriginalStageSourceRecordInput_25182c,7> originals{{
            {"navigation.pck",std::span(sample).subspan(0,2),0},
            {"placeholder.plc",std::span(sample).subspan(2,1),1},
            {"actionseq.asq",std::span(sample).subspan(3,1),2},
            {"monkeydb.xml",std::span(sample).subspan(4,1),3},
            {"navigation.pck",std::span(sample).subspan(5,1),4},
            {"background.pck.sz",std::span(sample).subspan(6,1),5},
            {"no_extension",std::span(sample).subspan(7,1),6}
        }};
        const auto names=originalStageSplitFilename_24d858("background.pck.sz");
        ck(names.originalNameAt00=="background" &&
           names.originalTypeAt04=="pck.sz","First-dot split (not last-dot)");
        ck(originalStageSplitFilename_24d858("stage.nav").originalNameAt00=="stage",
           "Original split head");
        ck(originalStageSplitFilename_24d858("stage.nav").originalTypeAt04=="nav",
           "Original split tail");
        ck(originalStageSplitFilename_24d858("no_extension").originalNameAt00.empty() &&
           originalStageSplitFilename_24d858("no_extension").originalTypeAt04.empty(),
           "Original no-dot empty-string case");
        OriginalStageResourceRegistration_25182c all(originals,"*");
        ck(all.records().size()==7,"Original wildcard retains all source entries");
        ck(all.records()[0].originalNameAt00=="navigation" &&
           all.records()[0].originalTypeAt04=="pck" &&
           all.records()[0].originalSizeAt0c==2,
           "Source 0x24D858 four-field resource record");
        ck(all.records()[5].originalTypeAt04=="pck.sz",
           "Source suffix must not be guessed away before first-dot split");
        ck(all.originalFind_24d4f0("navigation")==&all.namedEntries()[0],
           "Source 0x24D4F0 resolves FIRST name-match in original order");
        ck(all.originalFind_24d4f0("navigation")->borrowedOriginalPayload[0]==std::byte{11},
           "Original payload source pointer identity");
        ck(all.originalFind_24d4f0("Navigation")==nullptr &&
           all.originalFind_24d4f0("nav")==nullptr,
           "Source lookup name is exact, case sensitive");
        OriginalStageResourceRegistration_25182c pck(originals,"pck");
        ck(pck.records().size()==2 && pck.records()[0].originalSourceIndex==0 &&
           pck.records()[1].originalSourceIndex==4,
           "0x2519FC type filter is on extension, not source NAME");
        ck(pck.originalFind_24d4f0("monkeydb")==nullptr,
           "Original type filter removes XML entry");
        OriginalStageResourceRegistration_25182c compressed(originals,"pck.sz");
        ck(compressed.records().size()==1 &&
           compressed.records()[0].originalNameAt00=="background",
           "First-dot extension source match");
        OriginalStageResourceRegistration_25182c none(originals,"pck.other");
        ck(none.records().empty()&&none.originalFind_24d4f0("navigation")==nullptr,
           "Unknown source selector must not invent a file");
        OriginalStageResourceRegistration_25182c unconditional(originals,nullptr);
        ck(unconditional.records().size()==all.records().size(),
           "Original null selector bypass");
        OriginalStageResourceRegistration_25182c wildprefix(originals,"*anything");
        ck(wildprefix.records().size()==all.records().size(),
           "Source selector checks only first ASCII asterisk");
        ck(all.originalFind_24d4f0("placeholder")->borrowedOriginalPayload[0] ==
           std::byte{33},"Borrowed original source lifetime");
        std::cout<<"ORIGINAL_STAGE_RECORD_REGISTRATION_TEST_PASSED "
                 <<"source=0x24D858,0x25182C,0x24D4F0 size=16 filter=type\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"ORIGINAL_STAGE_RECORD_REGISTRATION_TEST_FAILED "<<error.what()<<'\n';
        return 1;
    }
}
