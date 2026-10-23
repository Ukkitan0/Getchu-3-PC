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
                       OriginalMovementMetricOwner_557f34,
                       OriginalMovementRegenerationOwner_36f8c8 {
    OriginalManagerResourceFloatFields_317b34 floatFields{
        0x700000u,1u,
        std::bit_cast<std::uint32_t>(3.0f),std::nullopt};
    std::uint32_t scaleRaw=0;
    bool foreignOriginalVF1=false;
    bool iterativeFlagged=false,disagreeRegeneratedVF1=false;
    bool foreignLoopBinding=false,allowIterativeExternalFallback=false;
    int movementCallbacks=0,resourceReads=0,vf1Publications=0;
    int flaggedReads=0,flaggedPublications=0,regenPublications=0;
    OriginalMovementChildD0_30c57c* actualChild=nullptr;
    std::array<std::uint32_t,4> lastVF1{};
    std::array<std::uint32_t,4> liveVF1{};
    std::array<std::uint32_t,4> regeneratedVF1{};
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t va) override {
        assert(va==0x240000u);
        return {0x700000u,1u,std::nullopt};
    }
    std::uint64_t call_original_276b68(std::uint64_t parent,std::uint64_t resource) override {
        assert(parent==0x220000u && resource==0x7001C0u);
        return 0;
    }
    void call_original_2767f4(std::uint64_t) override {
        throw std::runtime_error("conditional source callback incorrectly invoked");
    }
    void call_original_2773d4(std::uint64_t parent) override {
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
        return {foreignOriginalVF1?0x250000u:0x240000u,
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
    std::optional<OriginalMovementFlaggedInputs_36fb5c>
    read_original_flagged_vf1_36fb5c(
        const OriginalMovementChildD0_30c57c& c,std::uint32_t f12) override {
        if(!iterativeFlagged)return std::nullopt;
        assert(&c==actualChild);
        ++flaggedReads;
        if(flaggedReads==1) {
            assert(f12==std::bit_cast<std::uint32_t>(0.5f));
            const auto one=std::bit_cast<std::uint32_t>(1.0f);
            // A legal negative source progress is delegated BEFORE
            // attempting the source VU0 SQRT: norm(1,1,0,0)
            // requires irrational sqrt(2), and cannot be refused
            // by the restricted native exact-only arithmetic path.
            const auto vf1=c.originalWordAt(0xB4)==
                             std::bit_cast<std::uint32_t>(-1.0f)
                ? std::array<std::uint32_t,4>{one,one,0u,0u}
                : std::array<std::uint32_t,4>{
                    std::bit_cast<std::uint32_t>(2.0f),0u,0u,0u};
            return OriginalMovementFlaggedInputs_36fb5c{
                0x01FFF020u,c.originalWordAt(0xB4),
                c.originalWordAt(0xB8),f12,vf1};
        }
        assert(flaggedReads==2 &&
               f12==std::bit_cast<std::uint32_t>(0.25f));
        const auto next=regeneratedVF1;
        auto altered=next;
        if(disagreeRegeneratedVF1)
            altered[3]^=0x100u;
        return OriginalMovementFlaggedInputs_36fb5c{
            0x01FFF020u,c.originalWordAt(0xB4),
            c.originalWordAt(0xB8),f12,altered};
    }
    void publish_original_vf1_after_36fb5c(
        const std::array<std::uint32_t,4>& vf1) override {
        ++flaggedPublications;
        liveVF1=vf1;
    }
    std::optional<OriginalMovementRegenerationBinding_36f8c8>
    read_original_flagged_loop_binding_36fc18(
        const OriginalMovementChildD0_30c57c& c) override {
        if(!iterativeFlagged)return std::nullopt;
        assert(&c==actualChild && c.originalWordAt(0xB8)==2u);
        return OriginalMovementRegenerationBinding_36f8c8{
            foreignLoopBinding?0x01FFF040u:0x01FFF020u,this};
    }
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t parentVA) override {
        assert(iterativeFlagged && parentVA==0x240000u &&
               actualChild->originalWordAt(0xB8)==0u);
        std::array<std::array<std::uint32_t,4>,8> real{};
        for(auto& reg:real)
            for(auto& lane:reg)
                lane=std::bit_cast<std::uint32_t>(1.0f);
        real[0]=liveVF1;
        return real;
    }
    void publish_original_36f8c8_vu0(
        const OriginalMovementRegenerateResult_36f8c8& result) override {
        assert(actualChild->originalWordAt(0xB8)==0u &&
               result.originalFlagsA8After==2u);
        ++regenPublications;
        regeneratedVF1=result.originalVF1After;
    }
    std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c& child,std::uint32_t f12) override {
        if(iterativeFlagged) {
            if(!allowIterativeExternalFallback)
                throw std::logic_error(
                    "native original 36FC18 loop must not use owner fallback");
            assert(f12==std::bit_cast<std::uint32_t>(0.5f));
            ++movementCallbacks;
            return std::bit_cast<std::uint32_t>(1.25f);
        }
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
    // A wrong retail parent must fail BEFORE publishing VF1
    // or accessing the next original parent scale.
    owner.foreignOriginalVF1=true;
    bool foreignRejected=false;
    try {(void)originalMovementMetricStep_557f34(*source,owner);}
    catch(const std::invalid_argument&){foreignRejected=true;}
    assert(foreignRejected && owner.vf1Publications==1);
    owner.foreignOriginalVF1=false;
    // Missing true original +0x374 value cannot fall through.
    owner.floatFields.originalParentWord40=1;
    owner.floatFields.originalFloatAt374Bits.reset();
    refused=false;
    try {(void)originalMovementMetricStep_557f34(*source,owner);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    // Full source 36FB70 nonterminal flagged iteration from actual
    // 557F34 metric: 36FC14 adds +1/8, 36FBE4 clears flags,
    // 36FC18 calls real native 36F8C8, 36FC20 branches back to
    // 36FB70 with regenerated VF1 and remaining original f20.
    // With status=.5 and VF1=(2,0,0,0), 0.5 movement
    // consumes original 0.25 chunk; then progress=.625,
    // remaining=.25 and flags regenerated to bit1. The
    // next sqrt for the polynomial vector is inexact, so the
    // exact-only native subset must refuse rather than
    // invent PS2 COP1 rounding or claim completed gameplay.
    OriginalOwner iterative{};
    iterative.iterativeFlagged=true;
    iterative.floatFields.originalFloatAt374Bits=
        std::bit_cast<std::uint32_t>(1.0f);
    iterative.scaleRaw=std::bit_cast<std::uint32_t>(0.5f);
    auto iterChild=child();
    iterChild->writeWord(0xB0,0x744dd0u);
    iterative.actualChild=iterChild.get();
    originalMovementNormalBegin_557ed8(*iterChild,0x220000u,iterative);
    iterChild->writeWord(0xB4,std::bit_cast<std::uint32_t>(0.5f));
    iterChild->writeWord(0xB8,2u);
    bool iterativeRefused=false;
    try{(void)originalMovementMetricStep_557f34(*iterChild,iterative);}
    catch(const std::domain_error&){iterativeRefused=true;}
    assert(iterativeRefused &&
           iterChild->originalWordAt(0xB4)==
               std::bit_cast<std::uint32_t>(0.625f) &&
           iterChild->originalWordAt(0xB8)==2u &&
           iterative.flaggedReads==2 &&
           iterative.flaggedPublications==1 &&
           iterative.regenPublications==1 &&
           iterative.movementCallbacks==0);
    assert(iterative.regeneratedVF1==
           (std::array<std::uint32_t,4>{
               std::bit_cast<std::uint32_t>(0.244140625f),
               std::bit_cast<std::uint32_t>(0.390625f),
               std::bit_cast<std::uint32_t>(0.625f),
               std::bit_cast<std::uint32_t>(1.0f)}));

    // The real 36FC24 reentry must use the exact live VF1
    // returned by the 36F8C8 regeneration; no claimed owner
    // can silently substitute a different destination lane.
    OriginalOwner wrongVF{};
    wrongVF.iterativeFlagged=true;
    wrongVF.disagreeRegeneratedVF1=true;
    wrongVF.floatFields.originalFloatAt374Bits=
        std::bit_cast<std::uint32_t>(1.0f);
    wrongVF.scaleRaw=std::bit_cast<std::uint32_t>(0.5f);
    auto wrongChild=child();
    wrongChild->writeWord(0xB0,0x744dd0u);
    wrongVF.actualChild=wrongChild.get();
    originalMovementNormalBegin_557ed8(*wrongChild,0x220000u,wrongVF);
    wrongChild->writeWord(0xB4,std::bit_cast<std::uint32_t>(0.5f));
    wrongChild->writeWord(0xB8,2u);
    iterativeRefused=false;
    try{(void)originalMovementMetricStep_557f34(*wrongChild,wrongVF);}
    catch(const std::invalid_argument&){iterativeRefused=true;}
    assert(iterativeRefused && wrongVF.regenPublications==1 &&
           wrongVF.flaggedReads==2 &&
           wrongChild->originalWordAt(0xB8)==2u &&
           wrongVF.movementCallbacks==0);

    // A flagged loop owner for an unrelated EE child scratch
    // address cannot cause ANY original 36FB70 VF1 state write.
    // Without the source-equal 36FC18 binding, fall back to
    // the real external callback only when NO opt-in owner
    // exists; a present-but-wrong owner is an error.
    OriginalOwner foreignLoop{};
    foreignLoop.iterativeFlagged=true;
    foreignLoop.foreignLoopBinding=true;
    foreignLoop.floatFields.originalFloatAt374Bits=
        std::bit_cast<std::uint32_t>(1.0f);
    foreignLoop.scaleRaw=std::bit_cast<std::uint32_t>(0.5f);
    auto foreignChild=child();
    foreignChild->writeWord(0xB0,0x744dd0u);
    foreignLoop.actualChild=foreignChild.get();
    originalMovementNormalBegin_557ed8(
        *foreignChild,0x220000u,foreignLoop);
    foreignChild->writeWord(0xB4,std::bit_cast<std::uint32_t>(0.5f));
    foreignChild->writeWord(0xB8,2u);
    bool foreignLoopRejected=false;
    try{(void)originalMovementMetricStep_557f34(
        *foreignChild,foreignLoop);}
    catch(const std::invalid_argument&){foreignLoopRejected=true;}
    assert(foreignLoopRejected && foreignLoop.flaggedReads==1 &&
           foreignLoop.flaggedPublications==0 &&
           foreignLoop.regenPublications==0 &&
           foreignLoop.movementCallbacks==0 &&
           foreignChild->originalWordAt(0xB4)==
               std::bit_cast<std::uint32_t>(0.5f) &&
           foreignChild->originalWordAt(0xB8)==2u);

    // The original 36FB70 accepts finite negative progress.
    // Native nine-pass cap is source-proven only for status>=0;
    // negative source states must delegate to the REAL owner's
    // complete 36FB18 method rather than silently stop early.
    OriginalOwner negativeOwner{};
    negativeOwner.iterativeFlagged=true;
    negativeOwner.allowIterativeExternalFallback=true;
    negativeOwner.floatFields.originalFloatAt374Bits=
        std::bit_cast<std::uint32_t>(1.0f);
    negativeOwner.scaleRaw=std::bit_cast<std::uint32_t>(0.5f);
    auto negativeChild=child();
    negativeChild->writeWord(0xB0,0x744dd0u);
    negativeOwner.actualChild=negativeChild.get();
    originalMovementNormalBegin_557ed8(
        *negativeChild,0x220000u,negativeOwner);
    negativeChild->writeWord(0xB4,std::bit_cast<std::uint32_t>(-1.0f));
    negativeChild->writeWord(0xB8,2u);
    const auto negativeResult=originalMovementMetricStep_557f34(
        *negativeChild,negativeOwner);
    assert(negativeResult.originalMovementF0Raw==
               std::bit_cast<std::uint32_t>(1.25f) &&
           negativeOwner.movementCallbacks==1 &&
           negativeOwner.flaggedReads==1 &&
           negativeOwner.flaggedPublications==0 &&
           negativeOwner.regenPublications==0 &&
           negativeChild->originalWordAt(0xB4)==
               std::bit_cast<std::uint32_t>(-1.0f) &&
           negativeChild->originalWordAt(0xB8)==2u);

    std::cout<<"ORIGINAL_MOVEMENT_557F34_METRIC_PREFIX_PASSED\n";
}
