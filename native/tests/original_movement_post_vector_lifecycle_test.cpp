#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_post_vector_557f54.hpp"
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
                   OriginalMovementPostVectorOwner_557f54 {
    OriginalMovementChildD0_30c57c* child{};
    int setups=0,regenerations=0,moves=0,post=0;
    bool failPost=false;
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
    std::cout<<"ORIGINAL_POST_VECTOR_LIFECYCLE_557F54_PASSED\n";
}
