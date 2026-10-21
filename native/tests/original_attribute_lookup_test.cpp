#include "ae3/original_attribute_lookup.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("original attribute source lookup regression");}
static void write32(std::span<std::byte> b,std::size_t offset,std::uint32_t v){
    for(unsigned i=0;i<4;++i)b[offset+i]=std::byte((v>>(i*8))&0xff);
}
struct StringProvider final:OriginalAudioDescriptorSource {
    std::vector<std::string> observed;
    std::string originalStringField_2bb3bc(
        std::string_view k,std::string_view fallback) override {
        observed.emplace_back(k);
        return std::string(fallback);
    }
    float originalFloatField_2bb1d0(std::string_view,float fallback) override {
        return fallback;
    }
};
int main(){
    static_assert(OriginalAttributeMissingWord_2bb194==0x7fffffffu);
    // Explicitly decoded tree for the fixture; retail tree deserialization
    // is not claimed to be complete. Original search ordering is preserved.
    const OriginalAttributeKeyNode a{"audio",0,nullptr,nullptr};
    const OriginalAttributeKeyNode k{"keys",4,nullptr,nullptr};
    const OriginalAttributeKeyNode f{"float",8,&a,&k};
    const OriginalAttributeKeyNode v{"volume_scale",12,nullptr,nullptr};
    const OriginalAttributeKeyNode q{"reverb",16,nullptr,&v};
    const OriginalAttributeKeyNode root{"midi",20,&f,&q};
    ck(originalAttributeFind_5419ec(&root,"midi")==&root);
    ck(originalAttributeFind_5419ec(&root,"audio")==&a);
    ck(originalAttributeFind_5419ec(&root,"float")==&f);
    ck(originalAttributeFind_5419ec(&root,"keys")==&k);
    ck(originalAttributeFind_5419ec(&root,"reverb")==&q);
    ck(originalAttributeFind_5419ec(&root,"volume_scale")==&v);
    for(auto key:{"", "ame", "audiz", "floas", "floatz", "keyx", "missing", "volume_scalc", "zz"})
        ck(originalAttributeFind_5419ec(&root,key)==nullptr);
    ck(originalAttributeFind_5419ec(nullptr,"audio")==nullptr);

    std::array<std::byte,24> buffer{};
    write32(buffer,0,0x7fffffffu); // mapped, but original "absent" word
    write32(buffer,4,0x80000000u); // negative zero
    write32(buffer,8,0x7fc01234u); // NaN payload, not missing marker
    write32(buffer,12,std::bit_cast<std::uint32_t>(0.75f));
    write32(buffer,16,0xffaabbccu);
    write32(buffer,20,0x12345678u);
    ck(originalAttributeIntBits_2bb2b0(buffer,&root,"audio",77)==77);
    ck(originalAttributeIntBits_2bb2b0(buffer,&root,"missing",77)==77);
    ck(originalAttributeIntBits_2bb2b0(buffer,&root,"midi",77)==0x12345678u);
    ck(originalAttributeIntBits_2bb2b0(buffer,&root,"reverb",77)==0xffaabbccu);
    ck(std::bit_cast<std::uint32_t>(originalAttributeFloat_2bb1d0(buffer,&root,"keys",1.f))==0x80000000u);
    ck(std::bit_cast<std::uint32_t>(originalAttributeFloat_2bb1d0(buffer,&root,"float",1.f))==0x7fc01234u);
    ck(originalAttributeFloat_2bb1d0(buffer,&root,"volume_scale",1.f)==0.75f);
    ck(originalAttributeFloat_2bb1d0(buffer,&root,"audio",1.f)==1.f);
    ck(originalAttributeFloat_2bb1d0(buffer,&root,"absent",1.f)==1.f);
    const OriginalAttributeKeyNode invalid{"invalid",23,nullptr,nullptr};
    bool caught=false;
    try{(void)originalAttributeFloat_2bb1d0(buffer,&invalid,"invalid",1.f);}
    catch(const std::out_of_range&){caught=true;}
    ck(caught);

    StringProvider strings;
    OriginalAudioNumericAttributeBridge source(strings,buffer,&root);
    const auto desc=originalAudioDescriptorFields_1bb9a8(source);
    ck(strings.observed==std::vector<std::string>({"name","midi","reverb","vb","vh"}));
    ck(desc.volumeScale==0.75f);
    ck(desc.sourceStrings[1]=="M01.mid");
    for(int i=0;i<1000;i++){
        const auto num=float(i)*0.125f-40.f;
        write32(buffer,12,std::bit_cast<std::uint32_t>(num));
        ck(originalAttributeFloat_2bb1d0(buffer,&root,"volume_scale",1.f)==num);
    }
}
