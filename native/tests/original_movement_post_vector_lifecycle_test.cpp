#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_post_vector_557f54.hpp"
#include "ae3/original_movement_post_step_558030.hpp"
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
                   OriginalMovementPostStepContinuation_558030 {
    OriginalMovementChildD0_30c57c* child{};
    int setups=0,regenerations=0,moves=0,post=0;
    bool failPost=false,allowPostStep=false,failSource314c44=false;
    int postStepReads=0,source314c44Calls=0,sourceParentWord1cReads=0;
    std::uint32_t sourceParentFloat70Last=0u;
    std::uint32_t read_original_parent_word14(std::uint32_t va) override {
        assert(va==0x234000u);return 0x340000u;
    }
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t va) override {
        assert(va==0x234000u);++setups;
        return {0x700000u,1u,std::nullopt};
    }
    std::uint32_t call_original_276b68(
        std::uint32_t parent,std::uint32_t resource) override {
        assert(parent==0x340000u && resource==0x7001C0u);
        return 0;
    }
    void call_original_2767f4(std::uint32_t) override {
        throw std::logic_error("source conditional 2767F4 must not run");
    }
    void call_original_2773d4(std::uint32_t parent) override {
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
    std::optional<OriginalMovementPostStep_558030>
    read_original_post_step_after_557fc8(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==child && c.originalWordAt(0xB8)==2u);
        ++postStepReads;
        if(!allowPostStep)return std::nullopt;
        // Test-double values for an ALREADY COMPLETED source 557FC8..
        // 55802C continuation. No game engine or collision is invented.
        return OriginalMovementPostStep_558030{
            raw(0.0f),raw(3.0f),0x450000u};
    }
    std::uint32_t read_original_parent_word1c(
        std::uint32_t va) override {
        assert(va==0x234000u);
        ++sourceParentWord1cReads;
        return 0x880000u;
    }
    void call_original_314c44(
        std::uint32_t word1c,std::uint32_t s5,std::uint32_t mode) override {
        assert(word1c==0x880000u && s5==0x450000u && mode==2u);
        ++source314c44Calls;
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
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 controller{};
    OriginalMovementChildLifecycle_557e30 terminal(
        child(0.5f),first,&first,&first,&first);
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
    // The original C0 initialized-child branch must preserve the
    // state and re-enter post-step without repeating 557ED8 or 36F8C8.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,continuing)==0u);
    assert(continuationOwner.setups==1 &&
           continuationOwner.regenerations==1 &&
           continuationOwner.source314c44Calls==2 &&
           continuationOwner.postStepReads==2 && depth.depth==0u);
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

    std::cout<<"ORIGINAL_POST_VECTOR_LIFECYCLE_557F54_PASSED\n";
}
