#include "ae3/original_attribute_record_trailer.hpp"
#include "ae3/original_attribute_string.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("original attribute trailer test failed");}
static OriginalAttributeTokenizedLine line(const char* s){return originalAttributeSplitColon_2ba5f4(s);}
int main(){
    OriginalAttributeTokenization source;
    source.sourceLines={line("first"),line("2:A:B:1234"),line("S:64:name"),
        line("V:128:volume_scale"),line("len:35"),line("base:-40")};
    auto fields=originalAttributeStructuredFields_2ba584(source,0x00102000u);
    ck(fields.body.sourceHeader.sourceChildCountAt1c==2);
    ck(fields.body.sourceHeader.sourceFourthWordAt20==1234);
    ck(fields.body.sourceChildren.size()==2);
    ck(fields.body.sourceChildren[0].sourceKeyAt04=="name");
    ck(fields.body.sourceChildren[1].sourceValueOffsetAt08==128);
    ck(fields.trailer.sourcePenultimateWordAt24==35);
    ck(fields.trailer.sourceFinalRebaseDelta==-40);
    ck(fields.trailer.originalRebasedPointerAt04==0x00101fd8u);
    source.sourceLines.back()=line("base:16");
    ck(originalAttributeStructuredFields_2ba584(source,0xfffffffcu)
        .trailer.originalRebasedPointerAt04==12u);
    source.sourceLines.back()=line("base:-1");
    ck(originalAttributeStructuredFields_2ba584(source,0)
        .trailer.originalRebasedPointerAt04==0xffffffffu);
    source.sourceLines[source.sourceLines.size()-2]=line("short");
    bool threw=false;
    try{(void)originalAttributeStructuredFields_2ba584(source,0);}
    catch(const std::invalid_argument&){threw=true;}
    ck(threw);
    source.sourceLines[source.sourceLines.size()-2]=line("len:2");
    source.sourceLines.pop_back();
    threw=false;
    try{(void)originalAttributeStructuredFields_2ba584(source,0);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    source.sourceLines.push_back(line("base:0"));
    source.sourceLines[1]=line("0:A:B:0");
    ck(originalAttributeStructuredFields_2ba584(source,0)
        .body.sourceChildren.empty());
    for(int i=1;i<=2000;++i){
        source.sourceLines[source.sourceLines.size()-2]=line((std::string("len:")+std::to_string(i)).c_str());
        source.sourceLines.back()=line((std::string("base:")+std::to_string(-i)).c_str());
        const auto x=originalAttributeStructuredFields_2ba584(source,static_cast<std::uint32_t>(i));
        ck(x.trailer.sourcePenultimateWordAt24==i && x.trailer.originalRebasedPointerAt04==0u);
    }
}
