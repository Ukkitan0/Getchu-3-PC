#include "ae3/original_attribute_record_parser.hpp"
#include "ae3/original_attribute_string.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("original 0x2BA584 attribute record regression");}
static OriginalAttributeTokenizedLine line(const char* text) {
    return originalAttributeSplitColon_2ba5f4(text);
}
static void storeWord(std::span<std::byte> b,std::size_t off,std::uint32_t n){
    for(unsigned i=0;i<4;i++)b[off+i]=std::byte((n>>(i*8))&255u);
}
static void storeString(std::span<std::byte> b,std::size_t off,std::string_view s){
    ck(off+s.size()<b.size());
    for(std::size_t i=0;i<s.size();++i)b[off+i]=std::byte(static_cast<unsigned char>(s[i]));
    b[off+s.size()]=std::byte{0};
}
int main(){
    const auto h=originalAttributeHeaderFields_2ba6a4_2ba6f0(line("3:A:B:512"));
    ck(h.sourceChildCountAt1c==3 && h.sourceFourthWordAt20==512);
    ck(originalAttributeHeaderFields_2ba6a4_2ba6f0(line("0:one:two:-17")).sourceFourthWordAt20==-17);
    bool threw=false;
    try{(void)originalAttributeHeaderFields_2ba6a4_2ba6f0(line("1:2:3"));}
    catch(const std::invalid_argument&){threw=true;}
    ck(threw);
    const auto child=originalAttributeChildFields_2ba7d8_2ba8b0(line("T:100:volume_scale"),2);
    ck(child.sourceFirstCharacterAt00==uint8_t('T'));
    ck(child.sourceKeyAt04=="volume_scale" && child.sourceValueOffsetAt08==100 && child.sourceLineIndex==2);
    ck(originalAttributeChildFields_2ba7d8_2ba8b0(line("XY:-42:foo")).sourceFirstCharacterAt00==uint8_t('X'));
    threw=false;
    try{(void)originalAttributeChildFields_2ba7d8_2ba8b0(line(":8:name"));}
    catch(const std::invalid_argument&){threw=true;}
    ck(threw);
    OriginalAttributeTokenization tokens;
    tokens.sourceLines={line("preface"),line("3:unused:unused:4096"),
        line("C:100:midi"),line("C:200:name"),line("N:300:volume_scale"),line("trailer:1")};
    const auto body=originalAttributeRecordsFromTokens_2ba584(tokens);
    ck(body.sourceHeader.sourceFourthWordAt20==4096&&body.sourceChildren.size()==3);
    ck(body.sourceChildren[0].sourceLineIndex==2);
    ck(body.sourceChildren[0].sourceKeyAt04=="midi");
    ck(body.sourceChildren[1].sourceKeyAt04=="name");
    ck(body.sourceChildren[2].sourceValueOffsetAt08==300);
    OriginalAttributeTreeOwner owner;
    originalAttributeBindDecodedChildren_2ba9c0(owner,body.sourceChildren);
    ck(owner.originalInvariantsHold()&&owner.size()==3);
    std::array<std::byte,512> source{};
    storeString(source,100,"M99.mid");
    storeString(source,200,"level89");
    storeWord(source,300,std::bit_cast<std::uint32_t>(0.25f));
    const auto audio=originalAudioDescriptorFromDecodedAttributes_1bb9a8(source,owner.decodedRoot());
    ck(audio.sourceStrings[0]=="level89" && audio.sourceStrings[1]=="M99.mid");
    ck(audio.sourceStrings[2]=="system" && audio.sourceStrings[3]=="M01.bd");
    ck(audio.sourceStrings[4]=="M01.hd" && audio.volumeScale==0.25f);
    const std::array<OriginalAttributeParsedChild34_2ba714,1> duplicate{{{'A',"midi",400,9}}};
    storeString(source,400,"SECOND.mid");
    originalAttributeBindDecodedChildren_2ba9c0(owner,duplicate);
    ck(owner.size()==3&&owner.originalInvariantsHold());
    ck(originalAudioDescriptorFromDecodedAttributes_1bb9a8(source,owner.decodedRoot()).sourceStrings[1]=="SECOND.mid");
    tokens.sourceLines[1]=line("0:unused:unused:0");
    ck(originalAttributeRecordsFromTokens_2ba584(tokens).sourceChildren.empty());
    tokens.sourceLines[1]=line("-1:unused:unused:0");
    threw=false;
    try{(void)originalAttributeRecordsFromTokens_2ba584(tokens);}
    catch(const std::invalid_argument&){threw=true;}
    ck(threw);
    tokens.sourceLines[1]=line("9:unused:unused:5");
    threw=false;
    try{(void)originalAttributeRecordsFromTokens_2ba584(tokens);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    tokens.sourceLines[1]=line("1:unused:unused:3");
    tokens.sourceLines[2]=line("R:0:n");
    for(int i=0;i<2000;++i){
        tokens.sourceLines[2]=line((std::string("R:")+std::to_string(i)+":k").c_str());
        auto found=originalAttributeRecordsFromTokens_2ba584(tokens);
        ck(found.sourceChildren.size()==1 && found.sourceChildren.front().sourceValueOffsetAt08==i);
    }
}
