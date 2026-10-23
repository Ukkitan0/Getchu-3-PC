#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_metric_step_557f34.hpp"
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
struct Owner final:OriginalMovementParentRead_557e30,
                   OriginalMovementNormalCalls_557ed8,
                   OriginalMovementMetricOwner_557f34 {
    std::vector<std::string> calls{};
    std::uint32_t scale=0;
    std::uint32_t read_original_parent_word14(std::uint32_t va) override {
        assert(va==0x120000u);calls.emplace_back("parent14");
        return 0x340000u;
    }
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t va) override {
        assert(va==0x120000u);
        calls.emplace_back("317B9C-fields");
        return {0x700000u,1u,std::nullopt};
    }
    std::uint32_t call_original_276b68(std::uint64_t parent,std::uint32_t resource) override {
        assert(parent==0x340000u && resource==0x7001C0u);
        calls.emplace_back("276B68");return 0;
    }
    void call_original_2767f4(std::uint64_t) override {
        throw std::runtime_error("unexpected source callback 2767F4");
    }
    void call_original_2773d4(std::uint64_t parent) override {
        assert(parent==0x340000u);calls.emplace_back("2773D4");
    }
    OriginalManagerResourceFloatFields_317b34
    read_original_resource_float_fields(std::uint32_t va) override {
        assert(va==0x120000u);calls.emplace_back("317B34-fields");
        return {0x700000u,1u,std::bit_cast<std::uint32_t>(3.0f),std::nullopt};
    }
    OriginalResourceVU0Inputs_317b34
    read_original_resource_vu0_inputs(std::uint32_t) override {
        throw std::runtime_error("unexpected original 317B34 VU0 branch");
    }
    void publish_original_vf1_after_317b34(
        std::uint32_t,
        const std::array<std::uint32_t,4>&) override {
        throw std::runtime_error("unexpected source VF1 mutation on type1 fast path");
    }
    std::uint32_t read_original_parent_float70(std::uint32_t va) override {
        assert(va==0x120000u);calls.emplace_back("parent70");
        return scale;
    }
    std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c&,std::uint32_t scaled) override {
        assert(scaled==std::bit_cast<std::uint32_t>(3.0f));
        calls.emplace_back("36FB18-owner");
        throw std::runtime_error("unrecovered genuine movement step");
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c> makeChild() {
    std::array<std::byte,0x110> source{};
    std::bitset<0x110> known;
    for(std::size_t i=0xA0;i<0xB0;++i)known.set(i);
    return originalMovementChildCreate_30c57c(0x120000u,source,known,1,2);
}
}
int main() {
    Owner owner{};
    OriginalUpdateDepth_696c98 globalDepth{};
    OriginalManagerChild70 controller{};
    OriginalMovementChildLifecycle_557e30 child(makeChild(),owner,&owner,&owner);
    controller.child10=child.originalChildPointer();
    controller.word14=0xBEEFu;
    bool blocked=false;
    try {(void)originalManagerChildUpdate_3161b8(
             controller,0,0,0,globalDepth,child);}
    catch(const std::logic_error&){blocked=true;}
    assert(blocked && globalDepth.depth==0 &&
           controller.word14==0xBEEFu &&
           controller.child10==child.originalChildPointer() &&
           child.ownsOriginalChild());
    assert(owner.calls==std::vector<std::string>(
        {"parent14","317B9C-fields","276B68","2773D4",
         "317B34-fields","parent70"}));
    // The real source prefix ran and prepared child flags, but the
    // original 557F54+ actor collision/animation is NOT completed.
    owner.calls.clear();
    owner.scale=std::bit_cast<std::uint32_t>(1.0f);
    blocked=false;
    try {(void)originalManagerChildUpdate_3161b8(
             controller,0,0,0,globalDepth,child);}
    catch(const std::runtime_error&){blocked=true;}
    assert(blocked && globalDepth.depth==0 &&
           controller.word14==0xBEEFu &&
           controller.child10==child.originalChildPointer() &&
           owner.calls==std::vector<std::string>({
               "parent14","317B34-fields","parent70","36FB18-owner"}));
    // Source 0x557EB4 takes its BNEZL when child+C0==1,
    // skipping initial 317B9C and 276B68/2773D4 setup.
    // The second update must NOT clear the previous real
    // child state or create another fake native actor.
    std::cout<<"ORIGINAL_MOVEMENT_CHILD_METRIC_LIFECYCLE_PASSED\n";
}
