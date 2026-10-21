#include "ae3/original_attribute_string.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool ok){if(!ok) throw std::runtime_error("original retail source string test mismatch");}
static void word(std::span<std::byte> out,std::size_t p,std::uint32_t x){
    for(unsigned i=0;i<4;i++)out[p+i]=std::byte((x>>(8*i))&255u);
}
static void str(std::span<std::byte> out,std::size_t p,std::string_view value){
    ck(p+value.size()<out.size());
    for(std::size_t i=0;i<value.size();i++)out[p+i]=std::byte(static_cast<unsigned char>(value[i]));
    out[p+value.size()]=std::byte{0};
}
int main(){
    static_assert(OriginalEmptyStrings32_2bb1ac=="EmptyStrings32");
    // Explicitly decoded original-style index. Tree population from the
    // retail serial format is a separate unresolved game function.
    const OriginalAttributeKeyNode midi{"midi",32};
    const OriginalAttributeKeyNode name{"name",80,&midi};
    const OriginalAttributeKeyNode vb{"vb",224};
    const OriginalAttributeKeyNode volume{"volume_scale",384};
    const OriginalAttributeKeyNode vh{"vh",320,&vb,&volume};
    const OriginalAttributeKeyNode reverb{"reverb",144,&name,&vh};
    std::vector<std::byte> memory(512);
    str(memory,32,"user_override.mid");
    str(memory,80,"track01");
    str(memory,144,"EmptyStrings32");
    str(memory,224,"");
    str(memory,320,"mybank.hd");
    word(memory,384,std::bit_cast<std::uint32_t>(0.75f));
    ck(originalAttributeString_2bb3bc(memory,&reverb,"midi","M01.mid")=="user_override.mid");
    ck(originalAttributeString_2bb3bc(memory,&reverb,"name","noname")=="track01");
    ck(originalAttributeString_2bb3bc(memory,&reverb,"reverb","system")=="system");
    ck(originalAttributeString_2bb3bc(memory,&reverb,"vb","M01.bd")=="");
    ck(originalAttributeString_2bb3bc(memory,&reverb,"vh","M01.hd")=="mybank.hd");
    ck(originalAttributeString_2bb3bc(memory,&reverb,"absent","fallback")=="fallback");
    ck(originalAttributeString_2bb3bc(memory,nullptr,"name","noname")=="noname");
    auto descriptor=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,&reverb);
    ck(descriptor.sourceStrings[0]=="track01");
    ck(descriptor.sourceStrings[1]=="user_override.mid");
    ck(descriptor.sourceStrings[2]=="system");
    ck(descriptor.sourceStrings[3].empty());
    ck(descriptor.sourceStrings[4]=="mybank.hd");
    ck(descriptor.volumeScale==0.75f);
    // A byte-exact null-terminated source string may be longer than
    // the original 15-character short-string representation.
    const std::string longString(98,'R');
    str(memory,32,longString);
    ck(originalAttributeString_2bb3bc(memory,&reverb,"midi","M01.mid")==longString);
    // Existing buffer spans may include zero bytes inside a field, in
    // which case the original C-string construction stops at first NUL.
    memory[34]=std::byte{0};
    ck(originalAttributeString_2bb3bc(memory,&reverb,"midi","M01.mid")=="RR");
    const OriginalAttributeKeyNode pastEnd{"invalid",uint32_t(memory.size())};
    bool threw=false;
    try {(void)originalAttributeString_2bb3bc(memory,&pastEnd,"invalid","x");}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    // An absent terminator is rejected instead of reading outside the
    // owner's game-data allocation, unlike unsandboxed original CSTR code.
    const OriginalAttributeKeyNode noTerm{"unterminated",496};
    for(std::size_t i=496;i<memory.size();i++)memory[i]=std::byte{'X'};
    threw=false;
    try {(void)originalAttributeString_2bb3bc(memory,&noTerm,"unterminated","x");}
    catch(const std::out_of_range&){threw=true;}
    ck(threw);
    word(memory,384,0x7fffffffu);
    descriptor=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,&reverb);
    ck(descriptor.volumeScale==1.f);
    word(memory,384,0x80000000u);
    descriptor=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,&reverb);
    ck(std::bit_cast<std::uint32_t>(descriptor.volumeScale)==0x80000000u);
    for(unsigned n=0;n<1500;n++){
        const auto val=std::string("s")+std::to_string(n);
        str(memory,80,val);
        ck(originalAttributeString_2bb3bc(memory,&reverb,"name","default")==val);
    }
    // Fully absent indexed fields fall back to the original literal
    // set, including float 1.0f, with no alternate engine behavior.
    auto empty=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,nullptr);
    ck(empty.sourceStrings[0]=="noname"&&empty.sourceStrings[1]=="M01.mid");
    ck(empty.sourceStrings[2]=="system"&&empty.sourceStrings[3]=="M01.bd");
    ck(empty.sourceStrings[4]=="M01.hd"&&empty.volumeScale==1.f);
}
