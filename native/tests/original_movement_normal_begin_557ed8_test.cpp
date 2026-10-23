#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bitset>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;

namespace {
std::unique_ptr<OriginalMovementChildD0_30c57c> makeChild(
    std::uint32_t mode) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> known{};
    for(unsigned i=0xA4;i<0xAC;++i) known.set(i);
    // Source pre-update status 0.25; source setup must reset it.
    const auto bits=std::bit_cast<std::uint32_t>(0.25f);
    for(unsigned i=0;i<4;++i)
        scratch[0xA4+i]=std::byte((bits>>(8*i))&0xffu);
    for(unsigned i=0;i<4;++i)
        scratch[0xA8+i]=std::byte{0xF5};
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,known,1,mode);
}
struct Owner final:OriginalMovementNormalCalls_557ed8 {
    OriginalManagerResourceFields_317b9c fields{0x701000u,0u,0u};
    std::vector<std::string> calls{};
    OriginalMovementChildD0_30c57c* observedChild=nullptr;
    std::uint64_t collisionResult=0;
    std::uint64_t expectedParentWord14=0x345000u;
    bool failCollision=false, failRecalc=false, failReset=false;
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t parentVA) override {
        assert(parentVA==0x234000u);
        // 0x557EE8 JAL executes +0xB8-zero delay slot BEFORE
        // the original parent read inside 0x317B9C.
        if(observedChild)
            assert(observedChild->originalWordAt(0xB4)==0 &&
                   observedChild->originalWordAt(0xB8)==0);
        calls.emplace_back("read-parent");
        return fields;
    }
    std::uint64_t call_original_276b68(
        std::uint64_t parentWord14,std::uint32_t resourceVA) override {
        assert(parentWord14==expectedParentWord14);
        const auto expected=originalManagerSelectResource_317b9c(fields,2);
        assert(resourceVA==expected);
        calls.emplace_back("276B68");
        if(failCollision) throw std::runtime_error("missing source 276B68");
        return collisionResult;
    }
    void call_original_2767f4(std::uint64_t parentWord14) override {
        assert(parentWord14==expectedParentWord14);
        calls.emplace_back("2767F4");
        if(failRecalc) throw std::runtime_error("missing source 2767F4");
    }
    void call_original_2773d4(std::uint64_t parentWord14) override {
        assert(parentWord14==expectedParentWord14);
        calls.emplace_back("2773D4");
        if(failReset) throw std::runtime_error("missing source 2773D4");
    }
};
struct Parent final:OriginalMovementParentRead_557e30 {
    int calls=0;
    std::uint32_t sourceWord14=0x345000u;
    std::uint32_t read_original_parent_word14(std::uint32_t address) override {
        assert(address==0x234000u);
        ++calls;return sourceWord14;
    }
};
void cleared(const OriginalMovementChildD0_30c57c& child) {
    assert(child.originalWordAt(0xB4)==0 &&
           child.originalWordAt(0xB8)==0);
}
}
int main() {
    // Complete source 317B9C mode2 selection and original 557ED8
    // reset/collision call order, including conditional 2767F4.
    Owner source{};
    auto child=makeChild(2);
    assert(child->originalWordAt(0xB8)==0xF5F5F5F5u);
    source.observedChild=child.get();
    originalMovementNormalBegin_557ed8(
        *child,0x345000u,source);
    cleared(*child);
    assert(child->originalByteAt(0xC0)==std::byte{1});
    assert(source.calls==std::vector<std::string>({"read-parent","276B68","2773D4"}));

    source.collisionResult=1;source.calls.clear();
    child=makeChild(2);source.observedChild=child.get();
    originalMovementNormalBegin_557ed8(*child,0x345000u,source);
    assert(source.calls==std::vector<std::string>(
        {"read-parent","276B68","2767F4","2773D4"}));
    cleared(*child);assert(child->originalByteAt(0xC0)==std::byte{1});

    // Source callbacks must be genuine. A failed route setup cannot
    // publish child +0xC0=1 prematurely; earlier SWC1 reset survives.
    source.failCollision=true;source.calls.clear();child=makeChild(2);source.observedChild=child.get();
    bool rejected=false;
    try{originalMovementNormalBegin_557ed8(*child,0x345000u,source);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && source.calls==std::vector<std::string>({"read-parent","276B68"}));
    cleared(*child);assert(child->originalByteAt(0xC0)==std::byte{0});
    source.failCollision=false;
    source.failRecalc=true;source.calls.clear();child=makeChild(2);source.observedChild=child.get();
    rejected=false;
    try{originalMovementNormalBegin_557ed8(*child,0x345000u,source);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected&&source.calls==std::vector<std::string>({"read-parent","276B68","2767F4"}));
    assert(child->originalByteAt(0xC0)==std::byte{0});
    source.failRecalc=false;
    source.failReset=true;source.calls.clear();child=makeChild(2);source.observedChild=child.get();
    rejected=false;
    try{originalMovementNormalBegin_557ed8(*child,0x345000u,source);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected&&source.calls==std::vector<std::string>(
        {"read-parent","276B68","2767F4","2773D4"}));
    assert(child->originalByteAt(0xC0)==std::byte{0});
    source.failReset=false;

    // With actual normal owner, 557E30 progresses to 557F30
    // then explicitly refuses unresolved 317B34/36FB18 gameplay.
    source.collisionResult=1;source.calls.clear();
    Parent parent{};
    OriginalMovementChildLifecycle_557e30 lifecycle(
        makeChild(2),parent,&source);
    source.observedChild=static_cast<OriginalMovementChildD0_30c57c*>(
        lifecycle.originalChildPointer());
    OriginalManagerChild70 controller{};
    OriginalUpdateDepth_696c98 depth{};
    controller.child10=lifecycle.originalChildPointer();
    controller.word14=0x777u;
    rejected=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,lifecycle);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected&&parent.calls==1 && source.calls==
           std::vector<std::string>({"read-parent","276B68","2767F4","2773D4"}));
    assert(depth.depth==0 && controller.word14==0x777u &&
           controller.child10==lifecycle.originalChildPointer() &&
           lifecycle.ownsOriginalChild());
    // Complete retail setup state remains visible for later
    // 557F34 translation rather than being silently rolled back.
    assert(source.calls.size()==4);

    // Retail 557E68 LW s4,parent+14 sign extends to 64 bits.
    // 557EF8, 557F0C, 557F14, 557F18 DADDU copies the
    // full register to the 276B68/2767F4/2773D4 callbacks.
    source.expectedParentWord14=0xffffffff81234000ull;
    source.collisionResult=1u;
    source.calls.clear();
    child=makeChild(2);
    source.observedChild=child.get();
    originalMovementNormalBegin_557ed8(
        *child,source.expectedParentWord14,source);
    assert(source.calls==std::vector<std::string>(
        {"read-parent","276B68","2767F4","2773D4"}));
    assert(child->originalByteAt(0xC0)==std::byte{1});

    // Verify the real 557E30 child lifecycle SIGN-EXTENDS the
    // 32-bit callback result before entering normal setup.
    Parent signedParent{};
    signedParent.sourceWord14=0x81234000u;
    source.calls.clear();
    OriginalMovementChildLifecycle_557e30 signedLifecycle(
        makeChild(2),signedParent,&source);
    source.observedChild=static_cast<OriginalMovementChildD0_30c57c*>(
        signedLifecycle.originalChildPointer());
    controller.child10=signedLifecycle.originalChildPointer();
    controller.word14=0x707u;
    rejected=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,signedLifecycle);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected && signedParent.calls==1 && source.calls==
           std::vector<std::string>({"read-parent","276B68",
                                     "2767F4","2773D4"}));
    assert(controller.word14==0x707u &&
           controller.child10==signedLifecycle.originalChildPointer() &&
           signedLifecycle.ownsOriginalChild() && depth.depth==0u);
    // Source 557F08 BEQZ v0 inspects the full EE 64-bit
    // return, even when ONLY the high 32 bits are nonzero.
    // Truncation used to incorrectly SKIP original 2767F4.
    source.collisionResult=0x100000000ull;
    source.calls.clear();
    child=makeChild(2);
    source.observedChild=child.get();
    originalMovementNormalBegin_557ed8(
        *child,source.expectedParentWord14,source);
    assert(source.calls==std::vector<std::string>(
        {"read-parent","276B68","2767F4","2773D4"}));
    assert(child->originalByteAt(0xC0)==std::byte{1});

    std::cout<<"ORIGINAL_NORMAL_MOVEMENT_557ED8_SETUP_PASSED\n";
}
