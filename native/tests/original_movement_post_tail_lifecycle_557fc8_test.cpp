#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_post_vector_557f54.hpp"
#include "ae3/original_movement_post_tail_557fc8.hpp"
#include "ae3/original_manager_update_316174.hpp"
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
std::uint32_t raw(float f){return std::bit_cast<std::uint32_t>(f);}
struct Owner final : OriginalMovementParentRead_557e30,
                     OriginalMovementNormalCalls_557ed8,
                     OriginalMovementMetricOwner_557f34,
                     OriginalMovementRegenerationOwner_36f8c8,
                     OriginalMovementPostVectorOwner_557f54,
                     OriginalMovementPostTailOwner_557fc8 {
    OriginalMovementChildD0_30c57c* active{};
    int initialized=0,regenBit1=0,regenBit0=0,updates=0;
    int postVectors=0,manager30c0=0,manager2765=0;
    bool failTailVF=false,diagnostic=false;
    std::uint32_t read_original_parent_word14(std::uint32_t p) override {
        assert(p==0x234000u);return 0x340000u;
    }
    OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t p) override {
        assert(p==0x234000u);++initialized;
        return {0x700000u,1u,std::nullopt};
    }
    std::uint32_t call_original_276b68(
        std::uint32_t p,std::uint32_t resource) override {
        assert(p==0x340000u && resource==0x7001C0u);return 0;
    }
    void call_original_2767f4(std::uint32_t) override {
        throw std::runtime_error("unexpected original 2767F4");
    }
    void call_original_2773d4(std::uint32_t p) override {assert(p==0x340000u);}
    OriginalManagerResourceFloatFields_317b34
    read_original_resource_float_fields(std::uint32_t p) override {
        assert(p==0x234000u);
        return {0x700000u,1u,raw(3.0f),std::nullopt};
    }
    OriginalResourceVU0Inputs_317b34
    read_original_resource_vu0_inputs(std::uint32_t) override {
        throw std::runtime_error("original type-one resource must skip VU0");
    }
    void publish_original_vf1_after_317b34(
        std::uint32_t,const std::array<std::uint32_t,4>&) override {
        throw std::runtime_error("unexpected resource VF1 state");
    }
    std::uint32_t read_original_parent_float70(std::uint32_t p) override {
        assert(p==0x234000u);return raw(1.0f);
    }
    std::optional<OriginalMovementRegenerationBinding_36f8c8>
    read_original_regeneration_binding_36f8c8(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==active);
        return OriginalMovementRegenerationBinding_36f8c8{0x01FFF020u,this};
    }
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t p) override {
        assert(p==0x234000u);
        std::array<std::array<std::uint32_t,4>,8> regs{};
        for(auto& reg:regs)for(auto& lane:reg)lane=raw(1.0f);
        return regs;
    }
    void publish_original_36f8c8_vu0(
        const OriginalMovementRegenerateResult_36f8c8& r) override {
        assert(r.originalFlagsA8After==2u);++regenBit1;
    }
    void publish_original_36f83c_vu0(
        const OriginalMovementRegenerateResult_36f8c8& r) override {
        assert(r.originalFlagsA8After==3u);
        if(failTailVF)throw std::runtime_error("actual source VF1 publication failed");
        ++regenBit0;
    }
    std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c& c,std::uint32_t f12) override {
        assert(&c==active && c.originalWordAt(0xB8)&2u);
        assert(f12==raw(3.0f));
        return 0u; // test double for genuine original owner only
    }
    void publish_original_557f54_vu0(
        const OriginalMovementPostVector_557f54& r) override {
        assert(r.originalGprS3VF2==r.originalVF2After);
        assert(r.originalVF2After[0]==raw(1.0f));++postVectors;
    }
    std::uint64_t read_original_child_scratch_guest_va_557fc8(
        const OriginalMovementChildD0_30c57c& c) override {
        assert(&c==active);return 0x01FFF020u;
    }
    std::uint64_t call_original_30c0a0(
        std::uint32_t p,const std::array<std::uint32_t,4>& a1) override {
        assert(p==0x234000u);
        if(manager30c0==0) {
            // Source 558084 regeneration returns full 128-bit
            // VF1 polynomial: progress 0 -> [0,0,0,1].
            assert(a1[0]==0 && a1[3]==raw(1.0f));
        } else {
            // Source 557FDC LQ of original child+0x90, not a
            // fabricated vector reconstructed from s3.
            assert(a1[0]==raw(1.0f));
        }
        ++manager30c0;return 0x12345678450000ull;
    }
    void call_original_276538(
        std::uint64_t p,const std::array<std::uint32_t,4>& s3) override {
        assert(p==0x340000u && s3[0]==raw(1.0f));
        ++manager2765;
    }
    std::uint8_t read_original_6354e7() override {return diagnostic?1:0;}
    std::uint8_t read_original_6354e0() override {return 1;}
    void call_original_36fc28(std::uint64_t) override {
        throw std::runtime_error("diagnostic must not fire when E0!=0");
    }
    std::uint32_t read_original_parent_word1c(std::uint32_t p) override {
        assert(p==0x234000u);return 0x880000u;
    }
    void call_original_314c44(
        std::uint32_t a0,std::uint64_t a1,std::uint32_t a2) override {
        assert(a0==0x880000u && a1==0x12345678450000ull && a2==2u);
        ++updates;
    }
    void store_original_parent_float70(
        std::uint32_t,std::uint32_t) override {
        throw std::runtime_error("test child is ongoing; parent+70 write invalid");
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c> makeChild(){
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> known{};
    for(unsigned i=0x80;i<0xB0;++i)known.set(i);
    const auto one=raw(1.0f),pointer=0x744DD0u;
    for(unsigned i=0;i<4;++i) {
        scratch[0x90+i]=std::byte((one>>(8*i))&255u);
        scratch[0xA0+i]=std::byte((pointer>>(8*i))&255u);
    }
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,known,1,2);
}
}
int main(){
    Owner owner{};
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 controller{};
    OriginalMovementChildLifecycle_557e30 child(
        makeChild(),owner,&owner,&owner,&owner,nullptr,&owner);
    controller.child10=child.originalChildPointer();
    owner.active=static_cast<OriginalMovementChildD0_30c57c*>(controller.child10);
    controller.word14=77u;
    // First original update returns zero through 314C44:
    // child survives, original bit1 + bit0 regenerations each run.
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,child)==0u);
    assert(controller.word14==0u && controller.child10==owner.active);
    assert(depth.depth==0 && child.ownsOriginalChild());
    assert(owner.initialized==1 && owner.regenBit1==1 &&
           owner.regenBit0==1 && owner.postVectors==1 &&
           owner.manager30c0==1 && owner.manager2765==1 &&
           owner.updates==1 && owner.active->originalWordAt(0xB8)==3u);
    // Second source 557EB4 BNEZL enters 557F34 directly.
    // No duplicate initial route/bit regeneration. Original
    // 557FDC now reads the full original +0x90 vector.
    owner.diagnostic=true;
    assert(originalManagerChildUpdate_3161b8(
        controller,0,0,0,depth,child)==0u);
    assert(owner.initialized==1 && owner.regenBit1==1 &&
           owner.regenBit0==1 && owner.postVectors==2 &&
           owner.manager30c0==2 && owner.manager2765==2 &&
           owner.updates==2);
    // Final original status publication to +0x14 must still
    // be zero; no fake completion or child deletion.
    assert(controller.word14==0u && child.ownsOriginalChild() &&
           controller.child10==owner.active && depth.depth==0u);
    std::cout<<"ORIGINAL_POST_TAIL_LIFECYCLE_557FC8_PASSED\n";
}
