#include "ae3/original_movement_post_tail_557fc8.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
namespace {
std::uint32_t raw(float f){return std::bit_cast<std::uint32_t>(f);}
struct Owner:OriginalMovementPostTailOwner_557fc8 {
    std::vector<std::string> calls{};
    std::uint32_t publishedRatio{};
    bool diagFirst=false,diagSecond=false,failVF=false,failManager=false;
    bool expectSignExtendedParent=false;
    std::uint64_t read_original_child_scratch_guest_va_557fc8(
        const OriginalMovementChildD0_30c57c&) override {
        return 0x01FFF020u;
    }
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t p) override {
        assert(p==0x234000u);calls.emplace_back("readVF");
        std::array<std::array<std::uint32_t,4>,8> regs{};
        for(auto& r:regs)for(auto& x:r)x=raw(1.0f);
        return regs;
    }
    void publish_original_36f83c_vu0(
        const OriginalMovementRegenerateResult_36f8c8& out) override {
        assert(out.originalFlagsA8After==3u);
        calls.emplace_back("publishVF");
        if(failVF)throw std::runtime_error("real VF1/VF4/ACC owner failed");
    }
    std::uint64_t call_original_30c0a0(
        std::uint32_t p,const std::array<std::uint32_t,4>& originalA1) override {
        assert(p==0x234000u && originalA1[0]==raw(1.0f));
        calls.emplace_back("30C0A0");
        if(failManager)
            throw std::runtime_error("actual original 30C0A0 owner failed");
        // Source 557FEC DADDU preserves full EE v0 low 64 bits.
        return 0x12345678450000ull;
    }
    void call_original_276538(
        std::uint64_t p,const std::array<std::uint32_t,4>& originalS3) override {
        assert(p==(expectSignExtendedParent?0xffffffff81234000ull:0x340000ull) &&
               originalS3[0]==raw(1.0f));
        calls.emplace_back("276538");
    }
    std::uint8_t read_original_6354e7() override {
        calls.emplace_back("6354e7");return diagFirst?1:0;
    }
    std::uint8_t read_original_6354e0() override {
        calls.emplace_back("6354e0");return diagSecond?1:0;
    }
    void call_original_36fc28(std::uint64_t originalVA) override {
        assert(originalVA==0x01FFF020u);calls.emplace_back("36FC28");
    }
    std::uint32_t read_original_parent_word1c(std::uint32_t p) override {
        assert(p==0x234000u);calls.emplace_back("parent1c");
        return 0x880000u;
    }
    void call_original_314c44(
        std::uint32_t a0,std::uint64_t a1,std::uint32_t a2) override {
        assert(a0==0x880000u && a1==0x12345678450000ull && a2==2u);
        calls.emplace_back("314C44");
    }
    void store_original_parent_float70(
        std::uint32_t p,std::uint32_t bits) override {
        assert(p==0x234000u);
        publishedRatio=bits;
        calls.emplace_back("store70");
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c>
makeChild(float progress,unsigned flags,float vector=1.0f,
          bool knownA1=true,unsigned mode8=1) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> known{};
    for(unsigned i=0x80;i<0xB0;++i)known.set(i);
    // On bit0-set original 557FDC must LQ child+0x90,
    // copied from scratch+0x80 with genuine initialized bytes.
    if(!knownA1)
        for(unsigned i=0x80;i<0x90;++i)known.reset(i);
    const auto p=raw(progress),pointer=0x744DD0u;
    const auto one=raw(1.0f),vec=raw(vector);
    for(unsigned i=0;i<4;++i) {
        scratch[0x80+i]=std::byte((one>>(i*8))&255u);
        scratch[0x90+i]=std::byte((vec>>(i*8))&255u);
        scratch[0xA0+i]=std::byte((pointer>>(i*8))&255u);
        scratch[0xA4+i]=std::byte((p>>(i*8))&255u);
        scratch[0xA8+i]=std::byte((flags>>(i*8))&255u);
    }
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,known,mode8,2);
}
}
int main() {
    Owner o{};
    auto c=makeChild(1.0f,2u);
    auto source=originalMovementPostVector_557f54(*c,0x340000u);
    assert(!source.originalSlot5EarlyStatus);
    // Source bit0 clear: 36F83C sets bit0, full 128-bit v0
    // passes via POR to 30C0A0, VF2 passes via POR to 276538.
    assert(originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o)==2u);
    assert(c->originalWordAt(0xB8)==3u &&
           o.publishedRatio==raw(0.5f));
    assert(o.calls==std::vector<std::string>({
        "readVF","publishVF","30C0A0","276538","6354e7","store70"}));
    o.calls.clear();
    // Bit0 already set: source 557FDC LQ child+0x90,
    // POR full 128-bit source to 30C0A0; no regeneration.
    c=makeChild(0.5f,3u);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    assert(!source.originalSlot5EarlyStatus);
    assert(originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o)==0u);
    assert(o.calls==std::vector<std::string>({
        "30C0A0","276538","6354e7","parent1c","314C44"}));
    o.calls.clear();o.diagFirst=true;
    c=makeChild(1.0f,3u);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    assert(originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o)==2u);
    assert(o.calls==std::vector<std::string>({
        "30C0A0","276538","6354e7","6354e0","36FC28","store70"}));
    // The genuine 557FC0 early status1 must bypass this tail.
    o.calls.clear();
    c=makeChild(0.5f,3u,0.5f);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    assert(source.originalSlot5EarlyStatus==1u);
    bool refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && o.calls.empty());
    // Source bit0 unset, failed real VF publication must not
    // set child+B8 bit0 or report a completed update.
    c=makeChild(1.0f,2u);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    o.failVF=true;
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && c->originalWordAt(0xB8)==2u);
    // Original 557FDC consumes real 128-bit child+0x90 on
    // the already-flagged branch. Unknown copy provenance must
    // not turn into fabricated zero vector data for 30C0A0.
    o.failVF=false;o.calls.clear();
    c=makeChild(0.5f,3u,1.0f,false);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && o.calls.empty());

    // The source 557FE4 JAL 30C0A0 is before 276538 and
    // every diagnostic/558030 branch. Failure of the real
    // game owner must not be treated as successful frame return.
    c=makeChild(0.5f,3u);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    o.failManager=true;o.calls.clear();
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && o.calls==std::vector<std::string>({"30C0A0"}));
    // 557E68 LW s4,parent+14 sign-extends to full EE 64 bits.
    // Mode+8 zero skips 557F90 BBit032 but 557FF0 DADDU must
    // preserve all sign-extension bits into original 276538 a0.
    o.failManager=false;o.diagFirst=false;o.calls.clear();
    o.expectSignExtendedParent=true;
    c=makeChild(0.5f,3u,1.0f,true,0);
    source=originalMovementPostVector_557f54(*c,0xffffffff81234000ull);
    assert(!source.originalSlot5EarlyStatus);
    assert(originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0xffffffff81234000ull,source,
        {raw(2),raw(4),std::nullopt},o)==0u);
    assert(o.calls==std::vector<std::string>({
        "30C0A0","276538","6354e7","parent1c","314C44"}));
    // A raw zero-extended high-bit s4 is not produced by
    // 557E68 LW. Even if original mode+8 bypasses 557F90
    // tagged-pointer check, original 557FF0 must not receive it.
    // Refuse before calling 30C0A0 or touching any child flags.
    o.calls.clear();
    c=makeChild(0.5f,3u,1.0f,true,0);
    source=originalMovementPostVector_557f54(*c,0xffffffff81234000ull);
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x81234000ull,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && o.calls.empty() &&
           c->originalWordAt(0xB8)==3u);

    // The standalone source tail must not skip 557F54's bit1-clear
    // regeneration by accepting a detached post-vector object.
    // Original bit1-clear must branch to 558094 -> 36F8C8 first.
    o.calls.clear();
    c=makeChild(0.5f,0u);
    auto validVector=originalMovementPostVector_557f54(
        *makeChild(1.0f,3u),0x340000u);
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,validVector,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && o.calls.empty() &&
           c->originalWordAt(0xB8)==0u);

    // Original 557F8C DMFC2 s3,VF2 saves all four 32-bit
    // lanes, followed by 557FF8 POR a1,s3,zero. Reject a
    // detached owner state where the published EE s3 differs
    // from the actual earlier VU0 VF2, BEFORE any source call.
    c=makeChild(1.0f,3u);
    source=originalMovementPostVector_557f54(*c,0x340000u);
    const auto unchanged=source;
    source.originalGprS3VF2[3]^=1u;
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && o.calls.empty() &&
           c->originalWordAt(0xB8)==3u);
    source=unchanged;
    source.originalGprS3VF2X^=1u;
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0x340000u,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && o.calls.empty());

    // 557F90 BBit032 chooses a different tagged-parent branch
    // for nonzero mode+8; detached post-tail callers may not
    // bypass that earlier source decision with an ordinary VF2.
    source=unchanged;
    refused=false;
    try{(void)originalMovementPostTail_557fc8(
        *c,0x01FFF020u,0xffffffff81234000ull,source,
        {raw(2),raw(4),std::nullopt},o);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && o.calls.empty() &&
           c->originalWordAt(0xB8)==3u);

    std::cout<<"ORIGINAL_POST_TAIL_557FC8_TO_558030_PASSED\n";
}
