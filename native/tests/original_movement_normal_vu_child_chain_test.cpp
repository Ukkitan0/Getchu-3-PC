#include "ae3/original_movement_vu0_scalar_36f5f4.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
struct Parent final:OriginalMovementParentRead_557e30 {
    int reads=0;
    std::uint32_t read_original_parent_word14(std::uint32_t va) override {
        assert(va==0x123400);++reads;return 7;
    }
};
void lane(std::array<std::byte,16>& a,int i,float value) {
    const auto raw=std::bit_cast<std::uint32_t>(value);
    for(int n=0;n<4;++n)a[4*i+n]=std::byte((raw>>(8*n))&255u);
}
}
int main(){
    Parent parent{};
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 controller{};
    OriginalMovementVU0ScalarSource_36f5f4 originalVectors{};
    for(int i=0;i<4;++i) {
        lane(originalVectors.originalA1,i,float(i+1));
        lane(originalVectors.originalA2,i,float(i+1));
        lane(originalVectors.originalA3,i,float(i+1));
    }
    lane(originalVectors.originalA2,0,3.0f);
    lane(originalVectors.originalA3,0,1.5f);
    std::array<std::byte,0x110> scratch{};
    scratch.fill(std::byte{0xA5});
    std::bitset<0x110> originalKnown{};
    // Exact source: EE vectors -> original VU0 -> 36F6A8 scratch
    // -> 30C57C child -> 36F954 original status one ->
    // 557E30 status two -> 3161B8 original child destructor.
    const auto result=originalMovementVU0FirstReturn_36f5f4(
        scratch,originalKnown,0x1FFF020u,originalVectors);
    assert(result.scalar.originalSecondLengthSquaredBits==
           std::bit_cast<std::uint32_t>(0.25f));
    OriginalMovementChildLifecycle_557e30 owner(
        originalMovementChildCreate_30c57c(
            0x123400,scratch,originalKnown,1,2),parent);
    controller.child10=owner.originalChildPointer();
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,owner)==1);
    assert(controller.word14==2 && controller.child10==nullptr &&
           !owner.ownsOriginalChild() && depth.depth==0 &&
           parent.reads==1);

    // Source squared distance >=1 produces original scratch
    // +0xA4=0; real 36F954 proceeds into unrecovered gameplay.
    lane(originalVectors.originalA3,0,3.0f);
    originalKnown.reset();scratch.fill(std::byte{0xA5});
    const auto far=originalMovementVU0FirstReturn_36f5f4(
        scratch,originalKnown,0x1FFF020u,originalVectors);
    assert(far.scalar.originalSecondLengthSquaredBits==
           std::bit_cast<std::uint32_t>(4.0f));
    OriginalMovementChildLifecycle_557e30 unresolved(
        originalMovementChildCreate_30c57c(
            0x123400,scratch,originalKnown,1,2),parent);
    controller.child10=unresolved.originalChildPointer();
    controller.word14=111;
    bool refused=false;
    try{(void)originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,unresolved);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && controller.child10==unresolved.originalChildPointer() &&
           controller.word14==111 && unresolved.ownsOriginalChild() &&
           depth.depth==0 && parent.reads==2);
    std::cout<<"ORIGINAL_NORMAL_MOVEMENT_VU_TO_CHILD_COMPLETION_PASSED\n";
}
