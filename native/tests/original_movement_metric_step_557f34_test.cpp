#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
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
std::unique_ptr<OriginalMovementChildD0_30c57c> child() {
    std::array<std::byte,0x110> data{};
    std::bitset<0x110> known{};
    for(std::size_t i=0xA4;i<0xAC;++i)known.set(i);
    return originalMovementChildCreate_30c57c(0x240000u,data,known,1,2);
}
struct OriginalOwner : OriginalMovementNormalCalls_557ed8,
                       OriginalMovementMetricOwner_557f34 {
    OriginalManagerResourceFloatFields_317b34 floatFields{
        0x700000u,1u,
        std::bit_cast<std::uint32_t>(3.0f),std::nullopt};
    std::uint32_t scaleRaw=0;
    int movementCallbacks=0,resourceReads=0,vf1Publications=0;
    std::array<std::uint32_t,4> lastVF1{};
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t va) override {
        assert(va==0x240000u);
        return {0x700000u,1u,std::nullopt};
    }
    std::uint32_t call_original_276b68(std::uint32_t parent,std::uint32_t resource) override {
        assert(parent==0x220000u && resource==0x7001C0u);
        return 0;
    }
    void call_original_2767f4(std::uint32_t) override {
        throw std::runtime_error("conditional source callback incorrectly invoked");
    }
    void call_original_2773d4(std::uint32_t parent) override {
        assert(parent==0x220000u);
    }
    OriginalManagerResourceFloatFields_317b34
    read_original_resource_float_fields(std::uint32_t va) override {
        assert(va==0x240000u);
        ++resourceReads;
        return floatFields;
    }
    OriginalResourceVU0Inputs_317b34
    read_original_resource_vu0_inputs(std::uint32_t va) override {
        assert(va==0x240000u);
        return {0x240000u,
                {std::bit_cast<std::uint32_t>(3.0f),
                 std::bit_cast<std::uint32_t>(4.0f),0,0},
                std::bit_cast<std::uint32_t>(2.0f),
                std::bit_cast<std::uint32_t>(10.0f)};
    }
    void publish_original_vf1_after_317b34(
        std::uint32_t parentVA,
        const std::array<std::uint32_t,4>& frame) override {
        assert(parentVA==0x240000u);
        ++vf1Publications;
        lastVF1=frame;
    }
    std::uint32_t read_original_parent_float70(std::uint32_t va) override {
        assert(va==0x240000u);
        return scaleRaw;
    }
    std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c& child,std::uint32_t f12) override {
        assert(child.originalByteAt(0xC0)==std::byte{1});
        assert(f12==std::bit_cast<std::uint32_t>(3.0f));
        ++movementCallbacks;
        return std::bit_cast<std::uint32_t>(1.25f);
    }
};
}
int main() {
    OriginalOwner owner{};
    auto source=child();
    bool refused=false;
    try {(void)originalMovementMetricStep_557f34(*source,owner);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && owner.resourceReads==0);
    originalMovementNormalBegin_557ed8(*source,0x220000u,owner);
    assert(source->originalByteAt(0xC0)==std::byte{1});
    assert(source->originalWordAt(0xB4)==0);
    // Original 557F50 scale=0 means source 36FB18 BC1FL
    // returns real positive-zero before physics callbacks.
    auto result=originalMovementMetricStep_557f34(*source,owner);
    assert(result.originalF20MetricRaw==
           std::bit_cast<std::uint32_t>(3.0f));
    assert(result.originalF12ScaledRaw==0);
    assert(result.originalMovementF0Raw==0);
    assert(owner.movementCallbacks==0 && owner.resourceReads==1);
    // Positive f12 below original status threshold requires the
    // actual source game callback; no invented collision result.
    owner.scaleRaw=std::bit_cast<std::uint32_t>(1.0f);
    result=originalMovementMetricStep_557f34(*source,owner);
    assert(result.originalF12ScaledRaw==
           std::bit_cast<std::uint32_t>(3.0f));
    assert(result.originalMovementF0Raw==
           std::bit_cast<std::uint32_t>(1.25f));
    assert(owner.movementCallbacks==1);
    // Switch to original VU0 norm source branch, not an invented
    // resource-table float: length=5, offset=2, cap=10 => metric7.
    owner.floatFields.originalParentWord40=0;
    owner.scaleRaw=0;
    result=originalMovementMetricStep_557f34(*source,owner);
    assert(result.originalF20MetricRaw==
           std::bit_cast<std::uint32_t>(7.0f));
    assert(result.originalMovementF0Raw==0);
    assert(owner.vf1Publications==1);
    assert(owner.lastVF1[0]==std::bit_cast<std::uint32_t>(25.0f));
    assert(owner.lastVF1[1]==std::bit_cast<std::uint32_t>(32.0f));
    // Missing true original +0x374 value cannot fall through.
    owner.floatFields.originalParentWord40=1;
    owner.floatFields.originalFloatAt374Bits.reset();
    refused=false;
    try {(void)originalMovementMetricStep_557f34(*source,owner);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    std::cout<<"ORIGINAL_MOVEMENT_557F34_METRIC_PREFIX_PASSED\n";
}
