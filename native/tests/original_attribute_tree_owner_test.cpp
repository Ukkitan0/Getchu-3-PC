#include "ae3/original_attribute_tree_owner.hpp"
#include "ae3/original_attribute_string.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool b,const char* why="tree map regression") {
    if(!b)throw std::runtime_error(why);
}
static void put32(std::span<std::byte> data,std::size_t p,std::uint32_t w){
    for(unsigned i=0;i<4;++i)data[p+i]=std::byte((w>>(i*8))&255u);
}
static void putString(std::span<std::byte> data,std::size_t p,std::string_view text) {
    ck(p+text.size()<data.size());
    for(std::size_t i=0;i<text.size();++i)data[p+i]=std::byte(static_cast<unsigned char>(text[i]));
    data[p+text.size()]=std::byte{0};
}
int main() {
    OriginalAttributeTreeOwner owner;
    ck(owner.size()==0 && owner.decodedRoot()==nullptr);
    ck(owner.originalInvariantsHold());
    auto* z=owner.originalInsertOrFind_541dac("z",10);
    ck(owner.decodedRoot()==z && owner.originalInvariantsHold());
    auto* m=owner.originalInsertOrFind_541dac("m",20);
    ck(owner.originalInvariantsHold());
    auto* a=owner.originalInsertOrFind_541dac("a",30);
    // Descending inserts invoke the original 0x49C3B0 right rotation.
    ck(owner.originalInvariantsHold()&&owner.decodedRoot()==m);
    ck(owner.decodedLeftmost()==a&&owner.decodedRightmost()==z);
    ck(owner.originalFind_5419ec("a")==a);
    ck(owner.originalFind_5419ec("m")==m);
    ck(owner.originalFind_5419ec("z")==z);
    ck(owner.originalFind_5419ec("b")==nullptr);
    ck(owner.originalInsertOrFind_541dac("m",1234)==m);
    ck(m->sourceValueOffset==20 && owner.size()==3);
    ck(owner.originalBindParsed_2ba9c0("m",88)==m);
    ck(m->sourceValueOffset==88 && owner.size()==3);
    ck(owner.originalBindParsed_2ba9c0("brand_new",12345)->sourceValueOffset==12345);
    ck(owner.originalFind_5419ec("brand_new")->sourceValueOffset==12345);
    ck(owner.originalInvariantsHold() && owner.size()==4);

    owner.clear();
    ck(owner.originalInvariantsHold());
    for(const auto* key : {"a", "z", "m", "y", "b", "c", "x", "j", "l", "k"}){
        (void)owner.originalInsertOrFind_541dac(key,10);
        ck(owner.originalInvariantsHold());
    }
    ck(owner.size()==10);
    owner.clear();
    // Source index is a unique-key balanced binary tree rather than a
    // std::map shadow: verify all rotations against a separate oracle.
    std::mt19937 rng(0x8a9ec3u);
    std::map<std::string,std::uint32_t> oracle;
    for(unsigned i=0;i<18000;++i) {
        const unsigned keyValue=rng()%13000;
        const auto key="attr_"+std::to_string(keyValue);
        const std::uint32_t offset=(keyValue*4u)%2048u;
        auto* inserted=owner.originalInsertOrFind_541dac(key,offset);
        auto [iter,newKey]=oracle.emplace(key,offset);
        ck(inserted->originalKey==iter->first);
        ck(inserted->sourceValueOffset==iter->second);
        if(i%131==0){
            ck(owner.originalInvariantsHold(),"RB structure invalid");
            ck(owner.size()==oracle.size());
        }
    }
    ck(owner.originalInvariantsHold()&&owner.size()==oracle.size());
    for(const auto& [key,offset] : oracle){
        auto* found=owner.originalFind_5419ec(key);
        ck(found&&found->sourceValueOffset==offset);
    }
    ck(owner.originalFind_5419ec("attr_13001")==nullptr);
    // Cross source-level path: recovered tree creation -> original
    // 0x5419EC lookup -> 0x2BB3BC string -> 0x2BB1D0 float ->
    // original 0x1BB9A8 audio descriptor field initialization.
    owner.clear();
    std::array<std::byte,1024> memory{};
    auto create=[&](std::string_view key,std::uint32_t offset){
        owner.originalInsertOrFind_541dac(key,offset);
    };
    owner.originalBindParsed_2ba9c0("volume_scale",64);
    create("midi",128);
    create("name",256);
    create("vh",360);
    create("reverb",448);
    create("vb",576);
    put32(memory,64,std::bit_cast<std::uint32_t>(0.75f));
    putString(memory,128,"M88.mid");
    putString(memory,256,"level_audio");
    putString(memory,360,"M88.hd");
    putString(memory,448,"EmptyStrings32");
    putString(memory,576,"M88.bd");
    owner.originalBindParsed_2ba9c0("volume_scale",64); // duplicate original key
    auto desc=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,owner.decodedRoot());
    ck(owner.originalInvariantsHold());
    ck(desc.sourceStrings[0]=="level_audio");
    ck(desc.sourceStrings[1]=="M88.mid");
    ck(desc.sourceStrings[2]=="system");
    ck(desc.sourceStrings[3]=="M88.bd");
    ck(desc.sourceStrings[4]=="M88.hd");
    ck(desc.volumeScale==0.75f);
    put32(memory,68,std::bit_cast<std::uint32_t>(2.25f));
    owner.originalBindParsed_2ba9c0("volume_scale",68); // source overwrites node +0x14
    desc=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,owner.decodedRoot());
    ck(desc.volumeScale==2.25f);

    owner.clear();
    desc=originalAudioDescriptorFromDecodedAttributes_1bb9a8(memory,owner.decodedRoot());
    ck(desc.sourceStrings[0]=="noname" && desc.volumeScale==1.f);
}
