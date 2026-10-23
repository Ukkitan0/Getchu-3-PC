#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_post_vector_557f54.hpp"
#include "ae3/original_movement_post_step_558030.hpp"
#include "ae3/original_movement_flag0_regenerate_36f83c.hpp"
#include "ae3/original_movement_post_tail_557fc8.hpp"
#include <string>
#include <vector>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
std::uint32_t raw(float v){return std::bit_cast<std::uint32_t>(v);}
struct Owner final:OriginalMovementParentRead_557e30,
                   OriginalMovementNormalCalls_557ed8,
                   OriginalMovementMetricOwner_557f34,
                   OriginalMovementRegenerationOwner_36f8c8,
                   OriginalMovementPostVectorOwner_557f54,
                   OriginalMovementPostStepContinuation_558030,
                   OriginalMovementPostTailOwner_557fc8 {
    OriginalMovementChildD0_30c57c* child{};
    int setups=0,regenerations=0,moves=0,post=0;
    bool failPost=false,allowPostStep=false,failSource314c44=false;
    bool allowFlag0=true,failFlag0=false;
    bool useWideS5=false;
    bool diagnosticFirst=false,diagnosticSecond=false,badScratchVA=false;
    int real30c0a0Calls=0,real276538Calls=0,real36fc28Calls=0;
    std::vector<std::string> actualTailCalls{};
    bool wrongMetricF20=false,wrongMovementF21=false;
    int postStepReads=0,source314c44Calls=0,sourceParentWord1cReads=0;
    int flag0Reads=0,flag0Publications=0,flag0Bindings=0;
    std::uint32_t sourceParentFloat70Last=0u;
    std::uint32_t read_original_parent_word14(std::uint32_t va) override {
        assert(va==0x234000u);return 0x340000u;
    }
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t va) override {
        assert(va==0x234000u);++setups;
        return {0x700000u,1u,std::nullopt};
    }
    std::uint64_t call_original_276b68(
        std::uint64_t parent,std::uint64_t resource) override {
        assert(parent==0x340000u && resource==0x7001C0u);
        return 0;
    }
    void call_original_2767f4(std::uint64_t) override {
        throw std::logic_error("source conditional 2767F4 must not run");
    }
    void call_original_2773d4(std::uint64_t parent) override {
        assert(parent==0x340000u);
    }
    OriginalManagerResourceFloatFields_317b34
    read_original_resource_float_fields(std::uint32_t va) override {
        assert(va==0x234000u);
        return {0x700000u,1u,raw(3.0f),std::nullopt};
    }
    OriginalResourceVU0Inputs_317b34
    read_original_resource_vu0_inputs(std::uint32_t) override {
        throw std::logic_error("original kind-one resource must not run VU0");
    }
    void publish_original_vf1_after_317b34(
        std::uint32_t,const std::array<std::uint32_t,4>&) override {
        throw std::logic_error("original 317B34 direct return does not publish VF1");
    }
    std::uint32_t read_original_parent_float70(std::uint32_t va) override {
        assert(va==0x234000u);return raw(1.0f);
    }
    std::optional<OriginalMovementRegenerationBinding_36f8c8>
    read_original_regeneration_binding_36f8c8(
        const OriginalMovementChildD0_30c57c&) override {
        return OriginalMovementRegenerationBinding_36f8c8{0x01FFF020u,this};
    }
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t va) override {
        assert(va==0x234000u);
        std::array<std::array<std::uint32_t,4>,8> regs{};
        for(auto& r:regs)for(auto& l:r)l=raw(1.0f);
        return regs;
    }
    void publish_original_36f8c8_vu0(
        const OriginalMovementRegenerateResult_36f8c8& result) override {
        assert(result.originalFlagsA8After==2u);
        ++regenerations;
    }
    std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c& c,std::uint32_t scaled) override {
        assert(&c==child && c.originalWordAt(0xB8)==2u &&
               scaled==raw(3.0f));
        ++moves;
        return 0u; // Test owner; not a real-game movement substitute
    }
    std::optional<OriginalMovementFlag0Binding_36f83c>
    read_original_flag0_binding_36f83c(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==child && c.originalWordAt(0xB8)==2u);
        ++flag0Bindings;
        if(!allowFlag0)return std::nullopt;
        return OriginalMovementFlag0Binding_36f83c{0x01FFF020u,this};
    }
    std::uint64_t read_original_child_scratch_guest_va_557fc8(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==child);
        actualTailCalls.emplace_back("scratch");
        return badScratchVA?0u:0x01FFF020u;
    }
    std::uint64_t call_original_30c0a0(
        std::uint32_t parent,const std::array<std::uint32_t,4>& a1) override {
        assert(parent==0x234000u);
        if(real30c0a0Calls==0) {
            // 558084 regenerates progress=0 -> [0,0,0,1].
            assert(a1[0]==0u && a1[3]==raw(1.0f));
        } else {
            // On bit0-set reentry 557FDC loads genuine child+90;
            // this test's source original lane0 is exactly 1.
            assert(a1[0]==raw(1.0f) && a1[3]==0u);
        }
        ++real30c0a0Calls;
        actualTailCalls.emplace_back("30C0A0");
        return useWideS5 ? 0x12345678450000ull : 0x450000ull;
    }
    void call_original_276538(
        std::uint64_t parentWord14,
        const std::array<std::uint32_t,4>& s3) override {
        assert(parentWord14==0x340000u && s3[0]==raw(1.0f));
        ++real276538Calls;
        actualTailCalls.emplace_back("276538");
    }
    std::uint8_t read_original_6354e7() override {
        actualTailCalls.emplace_back("6354e7");
        return diagnosticFirst?1u:0u;
    }
    std::uint8_t read_original_6354e0() override {
        actualTailCalls.emplace_back("6354e0");
        return diagnosticSecond?1u:0u;
    }
    void call_original_36fc28(std::uint64_t scratch) override {
        assert(scratch==0x01FFF020u);
        ++real36fc28Calls;
        actualTailCalls.emplace_back("36FC28");
    }
    void publish_original_36f83c_vu0(
        const OriginalMovementRegenerateResult_36f8c8& result) override {
        assert(result.originalFlagsA8After==3u);
        ++flag0Reads;
        actualTailCalls.emplace_back("publish36F83C");
        if(failFlag0)
            throw std::runtime_error("original 36F83C VF0 publication failed");
        ++flag0Publications;
    }
    std::optional<OriginalMovementPostStep_558030>
    read_original_post_step_after_557fc8(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==child && (c.originalWordAt(0xB8)&3u)==3u);
        ++postStepReads;
        if(!allowPostStep)return std::nullopt;
        // Test-double values for an ALREADY COMPLETED source 557FC8..
        // 55802C continuation. No game engine or collision is invented.
        return OriginalMovementPostStep_558030{
            wrongMovementF21 ? raw(1.0f):raw(0.0f),
            wrongMetricF20 ? raw(2.0f):raw(3.0f),
            0x450000u};
    }
    std::uint32_t read_original_parent_word1c(
        std::uint32_t va) override {
        assert(va==0x234000u);
        ++sourceParentWord1cReads;
        actualTailCalls.emplace_back("readParent1C");
        return 0x880000u;
    }
    void call_original_314c44(
        std::uint64_t word1c,std::uint64_t s5,std::uint32_t mode) override {
        assert(word1c==0x880000u &&
               s5==(useWideS5?0x12345678450000ull:0x450000ull) && mode==2u);
        ++source314c44Calls;
        actualTailCalls.emplace_back("314C44");
        if(failSource314c44)
            throw std::runtime_error("actual original 314C44 owner failed");
    }
    void store_original_parent_float70(
        std::uint32_t va,std::uint32_t rawValue) override {
        assert(va==0x234000u);
        sourceParentFloat70Last=rawValue;
    }
    void publish_original_557f54_vu0(
        const OriginalMovementPostVector_557f54& result) override {
        assert(result.originalVU0Q==raw(2.0f) ||
               result.originalVU0Q==raw(1.0f));
        ++post;
        if(failPost)
            throw std::runtime_error("source VF1/VF2 state publication failed");
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c> child(float x) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> known{};
    for(unsigned i=0x90;i<0xB0;++i)known.set(i);
    const auto bits=raw(x),pointer=0x744DD0u;
    for(unsigned i=0;i<4;++i) {
        scratch[0x90+i]=std::byte((bits>>(8*i))&255u);
        scratch[0xA0+i]=std::byte((pointer>>(8*i))&255u);
    }
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,known,1,2);
}
}
int main() {
    Owner first{};
    first.allowPostStep=true;
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 controller{};
    OriginalMovementChildLifecycle_557e30 terminal(
        child(0.5f),first,&first,&first,&first,&first);
    controller.child10=terminal.originalChildPointer();
    first.child=static_cast<OriginalMovementChildD0_30c57c*>(controller.child10);
    // 557ED8 -> 557F34 -> 36FC18 -> 36F8C8 ->
    // 557F54 length 0.5 < source 0.95 -> v0=1.
    // Original 3161B8 stores status 1 and deletes child.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,terminal)==1);
    assert(controller.word14==1 && controller.child10==nullptr &&
           !terminal.ownsOriginalChild() && depth.depth==0);
    assert(first.setups==1 && first.regenerations==1 &&
           first.moves==1 && first.post==1);
    // Genuine 557FBC distance early status1 must take precedence:
    // never call a post-step source owner after original return.
    assert(first.postStepReads==0 && first.source314c44Calls==0 &&
           first.flag0Bindings==0 && first.flag0Publications==0);

    // Length 1.0 must NOT claim a completed original frame.
    Owner second{};
    OriginalMovementChildLifecycle_557e30 unresolved(
        child(1.0f),second,&second,&second,&second);
    controller.child10=unresolved.originalChildPointer();
    controller.word14=99;
    second.child=static_cast<OriginalMovementChildD0_30c57c*>(controller.child10);
    bool refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,unresolved);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && unresolved.ownsOriginalChild() &&
           controller.word14==99 && controller.child10==second.child);
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,unresolved);}
    catch(const std::logic_error&){refused=true;}
    assert(second.setups==1 && second.regenerations==1 &&
           second.post==2 && depth.depth==0);
    second.failPost=true;
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,unresolved);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && unresolved.ownsOriginalChild() &&
           controller.word14==99 && depth.depth==0);
    // A verified upstream 557FC8..55802C owner may now hand off to
    // the genuine translated 558030 code. Status zero must keep the
    // source child alive through the retail 3161B8 controller.
    second.failPost=false;
    second.allowPostStep=true;
    // Rebind with the opt-in post-step continuation owner.
    Owner continuationOwner{};
    continuationOwner.allowPostStep=true;
    OriginalMovementChildLifecycle_557e30 continuing(
        child(1.0f),continuationOwner,&continuationOwner,
        &continuationOwner,&continuationOwner,&continuationOwner);
    controller.child10=continuing.originalChildPointer();
    controller.word14=99u;
    continuationOwner.child=static_cast<
        OriginalMovementChildD0_30c57c*>(controller.child10);
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,continuing)==0u);
    assert(controller.word14==0u &&
           controller.child10==continuationOwner.child &&
           continuing.ownsOriginalChild() && depth.depth==0u);
    assert(continuationOwner.setups==1 &&
           continuationOwner.regenerations==1 &&
           continuationOwner.moves==1 && continuationOwner.post==1);
    assert(continuationOwner.postStepReads==1 &&
           continuationOwner.sourceParentWord1cReads==1 &&
           continuationOwner.source314c44Calls==1);
    assert(continuationOwner.flag0Bindings==1 &&
           continuationOwner.flag0Publications==1 &&
           continuationOwner.child->originalWordAt(0xB8)==3u);
    // The original C0 initialized-child branch must preserve the
    // state and re-enter post-step without repeating 557ED8 or 36F8C8.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,continuing)==0u);
    assert(continuationOwner.setups==1 &&
           continuationOwner.regenerations==1 &&
           continuationOwner.source314c44Calls==2 &&
           continuationOwner.postStepReads==2 && depth.depth==0u);
    assert(continuationOwner.flag0Bindings==1 &&
           continuationOwner.flag0Publications==1);
    // A claimed post-step owner without the preceding original 557F54
    // vector publication MUST NOT skip that original source segment.
    Owner missingVector{};
    missingVector.allowPostStep=true;
    OriginalMovementChildLifecycle_557e30 noVector(
        child(1.0f),missingVector,&missingVector,
        &missingVector,nullptr,&missingVector);
    controller.child10=noVector.originalChildPointer();
    controller.word14=99u;
    missingVector.child=static_cast<
        OriginalMovementChildD0_30c57c*>(controller.child10);
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,noVector);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && controller.word14==99u &&
           controller.child10==missingVector.child &&
           noVector.ownsOriginalChild() && depth.depth==0u &&
           missingVector.postStepReads==0);
    // If the real 557FC8..55802C original implementation is absent,
    // do not infer the source state or report a successful actor frame.
    Owner unavailable{};
    OriginalMovementChildLifecycle_557e30 noSource(
        child(1.0f),unavailable,&unavailable,
        &unavailable,&unavailable,&unavailable);
    controller.child10=noSource.originalChildPointer();
    controller.word14=99u;
    unavailable.child=static_cast<
        OriginalMovementChildD0_30c57c*>(controller.child10);
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,noSource);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && unavailable.postStepReads==1 &&
           unavailable.source314c44Calls==0 &&
           controller.word14==99u &&
           controller.child10==unavailable.child &&
           noSource.ownsOriginalChild() && depth.depth==0u);

    // Failure of the genuine 314C44 owner must not be confused with
    // original status zero, which would write the manager controller.
    continuationOwner.failSource314c44=true;
    controller.child10=continuing.originalChildPointer();
    controller.word14=77u;
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,continuing);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && controller.word14==77u &&
           controller.child10==continuationOwner.child &&
           continuing.ownsOriginalChild() && depth.depth==0u);

    // Original 557FC8 bit0-clear cannot jump to source 558030
    // without original 558084 -> 36F83C regeneration.
    Owner noFlag0{};
    noFlag0.allowPostStep=true;
    noFlag0.allowFlag0=false;
    OriginalMovementChildLifecycle_557e30 needsFlag0(
        child(1.0f),noFlag0,&noFlag0,&noFlag0,&noFlag0,&noFlag0);
    controller.child10=needsFlag0.originalChildPointer();
    controller.word14=55u;
    noFlag0.child=static_cast<OriginalMovementChildD0_30c57c*>(controller.child10);
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,needsFlag0);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && noFlag0.flag0Bindings==1 &&
           noFlag0.flag0Publications==0 && noFlag0.postStepReads==0 &&
           noFlag0.child->originalWordAt(0xB8)==2u &&
           controller.word14==55u && depth.depth==0u);

    // A failed live VF publication must leave original bit0 clear;
    // status and child ownership cannot claim success.
    Owner failFlag0{};
    failFlag0.allowPostStep=true;
    failFlag0.failFlag0=true;
    OriginalMovementChildLifecycle_557e30 refusedRegeneration(
        child(1.0f),failFlag0,&failFlag0,&failFlag0,&failFlag0,&failFlag0);
    controller.child10=refusedRegeneration.originalChildPointer();
    controller.word14=66u;
    failFlag0.child=static_cast<OriginalMovementChildD0_30c57c*>(controller.child10);
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,refusedRegeneration);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && failFlag0.flag0Publications==0 &&
           failFlag0.postStepReads==0 &&
           failFlag0.child->originalWordAt(0xB8)==2u &&
           controller.word14==66u && depth.depth==0u);

    // Original COP1 f20 and f21 come from the same 557F34
    // resource/movement path; reject post-step source data that
    // conflicts with 557F48 and 557F60 without calling 314C44.
    continuationOwner.failSource314c44=false;
    for(int mismatch=0;mismatch<2;++mismatch) {
        continuationOwner.wrongMetricF20=(mismatch==0);
        continuationOwner.wrongMovementF21=(mismatch==1);
        controller.child10=continuing.originalChildPointer();
        controller.word14=71u;
        const int original314c44=continuationOwner.source314c44Calls;
        const int original1c=continuationOwner.sourceParentWord1cReads;
        refused=false;
        try{(void)originalManagerChildUpdate_3161b8(
            controller,0,0,0,depth,continuing);}
        catch(const std::invalid_argument&){refused=true;}
        assert(refused && controller.word14==71u &&
               controller.child10==continuationOwner.child &&
               continuing.ownsOriginalChild() && depth.depth==0u &&
               continuationOwner.source314c44Calls==original314c44 &&
               continuationOwner.sourceParentWord1cReads==original1c);
    }
    continuationOwner.wrongMetricF20=false;
    continuationOwner.wrongMovementF21=false;
    // Preferred path: source 557FC8 itself runs genuine 36F83C,
    // source 30C0A0 (sets s5), 276538, debug gate, and 558030.
    // The legacy 558030 caller must NOT also regenerate or execute.
    Owner nativeTail{};
    nativeTail.allowPostStep=false;
    nativeTail.useWideS5=true;
    OriginalMovementChildLifecycle_557e30 realTail(
        child(1.0f),nativeTail,&nativeTail,&nativeTail,
        &nativeTail,&nativeTail,&nativeTail);
    controller.child10=realTail.originalChildPointer();
    controller.word14=88u;
    nativeTail.child=static_cast<OriginalMovementChildD0_30c57c*>(
        controller.child10);
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,realTail)==0u);
    assert(controller.word14==0u && controller.child10==nativeTail.child &&
           realTail.ownsOriginalChild() && depth.depth==0u);
    assert(nativeTail.flag0Publications==1 && nativeTail.real30c0a0Calls==1 &&
           nativeTail.real276538Calls==1 && nativeTail.postStepReads==0 &&
           nativeTail.child->originalWordAt(0xB8)==3u);
    assert(nativeTail.actualTailCalls==std::vector<std::string>({
        "scratch","publish36F83C","30C0A0","276538","6354e7",
        "readParent1C","314C44"}));
    nativeTail.actualTailCalls.clear();
    // Reentry reads child+90 via original POR, not duplicate 36F83C.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,realTail)==0u);
    assert(nativeTail.setups==1 && nativeTail.regenerations==1 &&
           nativeTail.flag0Publications==1 && nativeTail.real30c0a0Calls==2 &&
           nativeTail.postStepReads==0);
    assert(nativeTail.actualTailCalls==std::vector<std::string>({
        "scratch","30C0A0","276538","6354e7",
        "readParent1C","314C44"}));
    nativeTail.actualTailCalls.clear();
    nativeTail.diagnosticFirst=true;
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,realTail)==0u);
    assert(nativeTail.real36fc28Calls==1);
    assert(nativeTail.actualTailCalls==std::vector<std::string>({
        "scratch","30C0A0","276538","6354e7","6354e0",
        "36FC28","readParent1C","314C44"}));

    // The guest scratch address for 36F83C is NOT the host pointer.
    Owner missingGuest{};
    missingGuest.badScratchVA=true;
    OriginalMovementChildLifecycle_557e30 refusesTail(
        child(1.0f),missingGuest,&missingGuest,&missingGuest,
        &missingGuest,&missingGuest,&missingGuest);
    controller.child10=refusesTail.originalChildPointer();
    controller.word14=59u;
    missingGuest.child=static_cast<OriginalMovementChildD0_30c57c*>(
        controller.child10);
    refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,refusesTail);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && controller.word14==59u &&
           controller.child10==missingGuest.child &&
           missingGuest.real30c0a0Calls==0 &&
           missingGuest.child->originalWordAt(0xB8)==2u &&
           depth.depth==0u);
    std::cout<<"ORIGINAL_POST_VECTOR_LIFECYCLE_557F54_PASSED\n";
}
