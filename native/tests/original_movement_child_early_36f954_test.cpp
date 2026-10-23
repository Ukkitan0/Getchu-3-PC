#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_child_early_36f954.hpp"
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
struct Parent final:OriginalMovementParentRead_557e30 {
    int reads=0;
    std::uint32_t read_original_parent_word14(std::uint32_t va) override {
        assert(va==0x246800);++reads;return 5;
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c> child(float status,bool known=true) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> sourceKnown;
    if(known)for(unsigned n=0xA4;n<0xA8;++n)sourceKnown.set(n);
    const auto bits=std::bit_cast<std::uint32_t>(status);
    for(unsigned i=0;i<4;++i)scratch[0xA4+i]=std::byte((bits>>(8*i))&255u);
    return originalMovementChildCreate_30c57c(
        0x246800,scratch,sourceKnown,1,2);
}
}
int main() {
    Parent parent{};
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 controller{};
    auto terminal=child(1.0f);
    assert(originalMovementChildTerminal36F954(*terminal));
    OriginalMovementChildLifecycle_557e30 owner(std::move(terminal),parent);
    controller.child10=owner.originalChildPointer();
    // Zero flags: original 0x557EC8 -> 0x36F954 returns 1 ->
    // retail 0x557ED0 branch with delay v0=2 -> 0x3161B8
    // publishes source status2, deletes original child.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,owner)==1);
    assert(controller.word14==2 && controller.child10==nullptr &&
           !owner.ownsOriginalChild() && depth.depth==0 && parent.reads==1);
    auto above=child(2.0f);
    assert(originalMovementChildTerminal36F954(*above));
    auto below=child(0.0f);
    assert(!originalMovementChildTerminal36F954(*below));
    OriginalMovementChildLifecycle_557e30 unresolved(std::move(below),parent);
    controller.child10=unresolved.originalChildPointer();
    controller.word14=77;
    bool refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,unresolved);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && controller.child10==unresolved.originalChildPointer() &&
           controller.word14==77 && unresolved.ownsOriginalChild() &&
           depth.depth==0 && parent.reads==2);
    auto unknown=child(1.0f,false);
    refused=false;
    try{(void)originalMovementChildTerminal36F954(*unknown);}
    catch(const std::logic_error&){refused=true;}
    assert(refused);
    auto nanStatus=child(std::bit_cast<float>(0x7FC00001u));
    refused=false;
    try{(void)originalMovementChildTerminal36F954(*nanStatus);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    std::cout<<"ORIGINAL_36F954_NORMAL_EARLY_COMPLETION_PASSED\n";
}
