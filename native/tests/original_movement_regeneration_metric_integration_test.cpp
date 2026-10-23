#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bitset>
#include <bit>
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
std::uint32_t raw(float v){return std::bit_cast<std::uint32_t>(v);}
std::unique_ptr<OriginalMovementChildD0_30c57c> createChild() {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> known;
    for(unsigned i=0xA0;i<0xB0;++i)known.set(i);
    const auto p=0x00744dd0u;
    for(unsigned i=0;i<4;++i)
        scratch[0xA0+i]=std::byte((p>>(i*8))&255u);
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,known,1,2);
}
struct GenuineOwner final:OriginalMovementMetricOwner_557f34,
                          OriginalMovementNormalCalls_557ed8,
                          OriginalMovementRegenerationOwner_36f8c8 {
    OriginalMovementChildD0_30c57c* actualChild=nullptr;
    bool enableRegen=true,failPublication=false;
    int reads=0,published=0,movement=0;
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t va) override {
        assert(va==0x234000u);
        return {0x700000u,1u,std::nullopt};
    }
    std::uint32_t call_original_276b68(
        std::uint64_t parent,std::uint32_t resource) override {
        assert(parent==0x123000u && resource==0x7001c0u);
        return 0;
    }
    void call_original_2767f4(std::uint64_t) override {
        throw std::logic_error("source 2767F4 should be skipped");
    }
    void call_original_2773d4(std::uint64_t parent) override {
        assert(parent==0x123000u);
    }
    OriginalManagerResourceFloatFields_317b34
    read_original_resource_float_fields(std::uint32_t va) override {
        assert(va==0x234000u);
        return {0x700000u,1u,raw(3.0f),std::nullopt};
    }
    OriginalResourceVU0Inputs_317b34
    read_original_resource_vu0_inputs(std::uint32_t) override {
        throw std::logic_error("source resource VU0 branch not selected");
    }
    void publish_original_vf1_after_317b34(
        std::uint32_t,const std::array<std::uint32_t,4>&) override {
        throw std::logic_error("source resource VF1 publish not selected");
    }
    std::uint32_t read_original_parent_float70(std::uint32_t va) override {
        assert(va==0x234000u);
        return raw(1.0f);
    }
    std::optional<OriginalMovementRegenerationBinding_36f8c8>
    read_original_regeneration_binding_36f8c8(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==actualChild);
        return enableRegen
            ? std::optional<OriginalMovementRegenerationBinding_36f8c8>(
                  {0x01FFF020u,this})
            : std::nullopt;
    }
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t va) override {
        assert(va==0x234000u);
        ++reads;
        std::array<std::array<std::uint32_t,4>,8> regs{};
        for(auto& reg:regs)for(auto& lane:reg)lane=raw(1.0f);
        return regs;
    }
    void publish_original_36f8c8_vu0(
        const OriginalMovementRegenerateResult_36f8c8& result) override {
        assert(actualChild->originalWordAt(0xB8)==0u);
        assert(result.originalVF1After[3]==raw(1.0f));
        if(failPublication)
            throw std::runtime_error("original VF1/VF4/ACC publication failed");
        ++published;
    }
    std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c& c,std::uint32_t f12) override {
        assert(&c==actualChild);
        assert(c.originalWordAt(0xB8)==2u);
        assert(f12==raw(3.0f));
        ++movement;
        return raw(1.25f); // TEST DOUBLE for genuine owner, not production gameplay
    }
};
}
int main() {
    GenuineOwner owner{};
    auto c=createChild();
    owner.actualChild=c.get();
    originalMovementNormalBegin_557ed8(*c,0x123000u,owner);
    assert(c->originalWordAt(0xB8)==0);
    assert(c->originalByteAt(0xC0)==std::byte{1});
    // Authentic source: 557F34 metric -> 36FB18,
    // bit1 CLEAR -> 36FC18 -> 36F8C8 -> bit1 SET
    // -> owner's complete original 36FB18 continuation.
    const auto r=originalMovementMetricStep_557f34(*c,owner);
    assert(r.originalF20MetricRaw==raw(3.0f));
    assert(r.originalF12ScaledRaw==raw(3.0f));
    assert(r.originalMovementF0Raw==raw(1.25f));
    assert(c->originalWordAt(0xB8)==2u);
    assert(owner.reads==1 && owner.published==1 && owner.movement==1);
    (void)originalMovementMetricStep_557f34(*c,owner);
    assert(owner.reads==1 && owner.published==1 && owner.movement==2);
    c=createChild();
    owner.actualChild=c.get();
    originalMovementNormalBegin_557ed8(*c,0x123000u,owner);
    owner.failPublication=true;
    bool refused=false;
    try{(void)originalMovementMetricStep_557f34(*c,owner);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && c->originalWordAt(0xB8)==0u && owner.movement==2);
    std::cout<<"ORIGINAL_557F34_TO_36F8C8_REGENERATION_PASSED\n";
}
