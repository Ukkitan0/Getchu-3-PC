#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;

struct GenuineParent final:OriginalMovementParentRead_557e30 {
    int originalReads=0;
    bool refuse=false;
    std::uint32_t read_original_parent_word14(std::uint32_t va) override {
        ++originalReads;
        assert(va==0x00246800u);
        if(refuse)throw std::runtime_error("original parent+0x14 not loaded");
        return 0x1234;
    }
};
static std::unique_ptr<OriginalMovementChildD0_30c57c> makeOriginalChild() {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> realKnown;
    // Construction uses the exact original header, source data
    // bytes that are actually initialized, and retail 0xC0 reset.
    for(std::size_t offset=0xA0;offset<0xB0;++offset)
        realKnown.set(offset);
    return originalMovementChildCreate_30c57c(
        0x00246800u,scratch,realKnown,1,0x21);
}
int main() {
    GenuineParent parent{};
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 controller{};
    OriginalMovementChildLifecycle_557e30 source(makeOriginalChild(),parent);
    controller.child10=source.originalChildPointer();
    const auto address=controller.child10;

    // Original 0x3161B8 a2 lowbyte promotes bit0 of source a1.
    // Actual retail child slot5 0x557E30 returns source 1,
    // then actual original child slot1 0x556A0C releases actor.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,1,0,depth,source)==1);
    assert(controller.word14==1 && controller.child10==nullptr);
    assert(parent.originalReads==1 && !source.ownsOriginalChild());
    assert(source.originalChildPointer()==nullptr && depth.depth==0);
    bool rejected=false;
    try{(void)source.call_original_vtable5(address,1);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected);

    // Original a1 bit1 also returns 1. The source *must*
    // read genuine parent+0x14 even when this branch is taken.
    OriginalMovementChildLifecycle_557e30 second(makeOriginalChild(),parent);
    controller.child10=second.originalChildPointer();
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,1,depth,second)==1);
    assert(controller.word14==1 && controller.child10==nullptr);
    assert(parent.originalReads==2 && !second.ownsOriginalChild());

    // Normal per-frame gameplay follows unrecovered child slot5
    // branches at 0x557EB4+. Must refuse rather than return fake 0.
    OriginalMovementChildLifecycle_557e30 normal(makeOriginalChild(),parent);
    controller.child10=normal.originalChildPointer();
    controller.word14=0xABCDEFu;
    rejected=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,normal);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected && controller.word14==0xABCDEFu &&
           controller.child10==normal.originalChildPointer() &&
           normal.ownsOriginalChild() && depth.depth==0);
    assert(parent.originalReads==3);

    // Delay-slot parent read errors must propagate, with original
    // child still live and no synthetic status or deletion.
    parent.refuse=true;
    rejected=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,1,0,0,depth,normal);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && controller.word14==0xABCDEFu &&
           controller.child10==normal.originalChildPointer() &&
           normal.ownsOriginalChild() && depth.depth==0);
    assert(parent.originalReads==4);
    parent.refuse=false;

    // Mis-owned child pointers cannot be used for slot5 or delete.
    int foreign=42;
    rejected=false;
    try{(void)normal.call_original_vtable5(&foreign,1);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && normal.ownsOriginalChild());
    rejected=false;
    try{normal.call_original_child_vtable1_delete(&foreign);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && normal.ownsOriginalChild());
    // Completing source bit0 after previous error now performs real
    // original deletion at shared 0x3161B8 update depth.
    assert(originalManagerChildUpdate_3161b8(
        controller,1,0,0,depth,normal)==1);
    assert(controller.word14==1 && controller.child10==nullptr &&
           !normal.ownsOriginalChild() && depth.depth==0);
    std::cout<<"ORIGINAL_MOVEMENT_CHILD_LIFECYCLE_557E30_PASSED\n";
}
