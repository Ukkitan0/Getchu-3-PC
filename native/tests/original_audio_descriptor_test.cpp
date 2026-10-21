#include "ae3/original_audio_descriptor.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool v){if(!v)throw std::runtime_error("original 0x1BB9A8 audio descriptor mismatch");}
struct TestSource final:OriginalAudioDescriptorSource {
    std::vector<std::string> calls;
    bool overrideFields=false;
    float customVolume=0.5f;
    std::string originalStringField_2bb3bc(
            std::string_view key,std::string_view fallback) override {
        calls.push_back(std::string(key));
        return overrideFields?std::string(key)+"_retail_property":std::string(fallback);
    }
    float originalFloatField_2bb1d0(std::string_view key,float fallback) override {
        ck(fallback==1.0f);
        calls.push_back(std::string(key));
        return overrideFields?customVolume:fallback;
    }
};
int main(){
    static_assert(OriginalAudioDescriptorStride_1bb458==176);
    static_assert(OriginalAudioVolumeFieldOffset_1bbafc==160);
    constexpr std::array<std::string_view,5> keys={"name","midi","reverb","vb","vh"};
    constexpr std::array<std::string_view,5> defaults={"noname","M01.mid","system","M01.bd","M01.hd"};
    for(std::size_t i=0;i<5;++i){
        ck(OriginalAudioDescriptorFields_1bb9a8[i].sourceName==keys[i]);
        ck(OriginalAudioDescriptorFields_1bb9a8[i].sourceDefault==defaults[i]);
        ck(OriginalAudioDescriptorFields_1bb9a8[i].originalSlotOffset==0x20*i);
    }
    TestSource empty;
    const auto a=originalAudioDescriptorFields_1bb9a8(empty);
    ck(a.volumeScale==1.0f);
    for(std::size_t i=0;i<5;++i)ck(a.value(i)==defaults[i]);
    ck(empty.calls==std::vector<std::string>({"name","midi","reverb","vb","vh","volume_scale"}));
    TestSource withValues;
    withValues.overrideFields=true;
    auto b=originalAudioDescriptorFields_1bb9a8(withValues);
    for(std::size_t i=0;i<5;++i)ck(b.value(i)==std::string(keys[i])+"_retail_property");
    ck(b.volumeScale==0.5f);
    // Source uses MOV.S F0,F20. Preserve negative zero and NaN data
    // rather than normalizing float values in the native model.
    withValues.customVolume=std::bit_cast<float>(0x80000000u);
    b=originalAudioDescriptorFields_1bb9a8(withValues);
    ck(std::bit_cast<std::uint32_t>(b.volumeScale)==0x80000000u);
    withValues.customVolume=std::bit_cast<float>(0x7fc01234u);
    b=originalAudioDescriptorFields_1bb9a8(withValues);
    ck(std::bit_cast<std::uint32_t>(b.volumeScale)==0x7fc01234u);
    for(unsigned i=0;i<1000;++i){
        withValues.customVolume=float(i)*0.125f;
        const auto v=originalAudioDescriptorFields_1bb9a8(withValues);
        ck(v.volumeScale==withValues.customVolume);
    }
}
