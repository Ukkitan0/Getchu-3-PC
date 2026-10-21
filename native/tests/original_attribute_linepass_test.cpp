#include "ae3/original_attribute_loaded_text.hpp"
#include "ae3/original_attribute_string.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("original line preprocess mismatch");}
static void w32(std::span<std::byte> b,std::size_t o,std::uint32_t x){
    for(int i=0;i<4;++i)b[o+i]=std::byte((x>>(i*8))&255u);
}
static void str(std::span<std::byte> b,std::size_t o,std::string_view v){
    ck(o+v.size()<b.size());
    for(std::size_t i=0;i<v.size();++i)b[o+i]=std::byte(static_cast<unsigned char>(v[i]));
    b[o+v.size()]=std::byte{0};
}
int main(){
    const auto scan=[](std::string_view s){return originalAttributeScanLines_2ba3ac(s);};
    ck(scan("").committedLines.empty());
    ck(scan("no newline").committedLines.empty());
    ck(scan("A\nB").committedLines==std::vector<std::string>({"A"}));
    ck(scan("\n").committedLines==std::vector<std::string>({""}));
    ck(scan("A\n\nB\n").committedLines==std::vector<std::string>({"A","","B"}));
    ck(scan("sizest\nb:123\nignored\n").committedLines==
        std::vector<std::string>({"sizest","b:123"}));
    const auto early=scan("abc sizest def\nbefore\nfoo b:bar\nafter\n");
    ck(early.sourceSawSizeMarker&&early.terminatedAtBColon);
    ck(early.committedLines==std::vector<std::string>({"abc sizest def","before","foo b:bar"}));
    ck(early.consumedSourceBytes==std::string("abc sizest def\nbefore\nfoo b:bar\n").size());
    const auto same=scan("sizestb:\nunrelated\nb:hit\n");
    ck(same.committedLines==std::vector<std::string>({"sizestb:","unrelated","b:hit"}));
    ck(same.terminatedAtBColon);
    const auto absent=scan("b:before\nsizest\nnope\n");
    ck(!absent.terminatedAtBColon&&absent.sourceSawSizeMarker);
    ck(absent.committedLines.size()==3);
    const auto unterminated=scan("sizest\nb:without-newline");
    ck(!unterminated.terminatedAtBColon&&unterminated.committedLines.size()==1);
    ck(scan("x\r\ny\n").committedLines==std::vector<std::string>({"x\r","y"}));
    const std::string binary("x\0y\n",4);
    ck(scan(binary).committedLines==std::vector<std::string>({std::string("x\0y",3)}));
    // Verify native-composed authentic call chain from source text to
    // original attribute keyed lookup; this is NOT a retail file fixture.
    OriginalAttributeTreeOwner owner;
    auto parsed=originalAttributeBindLoadedText_2bb03c(owner,
        "meta\n3:a:b:512\nS:64:name\nS:96:midi\nF:128:volume_scale\nbytes:100\nbase:-8\n",0x1000u);
    ck(parsed.committedPhysicalLines==7);
    ck(parsed.parsed.body.sourceChildren.size()==3);
    ck(parsed.parsed.trailer.sourcePenultimateWordAt24==100);
    ck(parsed.parsed.trailer.originalRebasedPointerAt04==0xff8u);
    ck(owner.size()==3&&owner.originalInvariantsHold());
    std::array<std::byte,256> source{};
    str(source,64,"John");str(source,96,"M33.mid");
    w32(source,128,std::bit_cast<std::uint32_t>(0.8f));
    const auto desc=originalAudioDescriptorFromDecodedAttributes_1bb9a8(source,owner.decodedRoot());
    ck(desc.sourceStrings[0]=="John"&&desc.sourceStrings[1]=="M33.mid");
    ck(desc.sourceStrings[2]=="system"&&desc.volumeScale==0.8f);
    bool failed=false;
    try {(void)originalAttributeBindLoadedText_2bb03c(owner,"bad\n",0);}
    catch(const std::invalid_argument&){failed=true;}
    ck(failed&&owner.size()==3); // parse failure leaves native owner intact
    for(std::size_t n=0;n<10000;++n){
        const auto sourceText=std::string(n%4,'x')+"\n";
        const auto outcome=scan(sourceText);
        ck(outcome.committedLines.size()==1);
        ck(outcome.committedLines[0]==std::string(n%4,'x'));
    }
}
