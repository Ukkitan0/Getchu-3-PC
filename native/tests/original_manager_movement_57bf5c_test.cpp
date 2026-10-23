#include "ae3/original_manager_movement_57bf5c.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
struct Calls final:OriginalManagerMovementCalls_57bf5c {
    int old=12,created=34;
    float direction=0,metric=3;
    int globalE7=0,globalE0=0;
    bool publishOldDuringCreate=false,vectorEqual=true;
    bool failExactVu=false,failRealOwner=false;
    std::uint32_t update=1,done=0xABCDEF;
    std::vector<std::string> order;
    float call_original_vu0_direction_57bfac(
        OriginalManagerChild70&) override {
        order.push_back("VU-direction");
        if(failExactVu)throw std::runtime_error("original VU direction unresolved");
        return direction;
    }
    void call_original_21b854(OriginalManagerChild70&) override {
        order.push_back("21B854");
    }
    float call_original_vu0_metric_57c090(
        OriginalManagerChild70&) override {
        order.push_back("VU-metric");return metric;
    }
    void call_original_441f40(
        std::array<std::byte,0x40>& scratch) override {
        order.push_back("441F40");
        scratch[0]=std::byte{0x3C};
    }
    void call_original_36f5f4(
        OriginalMovementScratch_57bf5c& scratch) override {
        order.push_back("36F5F4");
        assert(scratch.sourceStack20[0]==std::byte{0x3C});
        assert(scratch.sourceStack60[0]==std::byte{0x3C});
    }
    void* call_original_30c57c(
        OriginalManagerChild70&,const OriginalMovementScratch_57bf5c& scratch) override {
        order.push_back("30C57C");
        assert(scratch.sourceStack20[0]==std::byte{0x3C});
        if(publishOldDuringCreate)active->child10=&old;
        return &created;
    }
    OriginalManagerChild70* active=nullptr;
    void call_original_child_vtable1_delete(void* child) override {
        assert(child==&old);order.push_back("child-delete");
    }
    std::uint32_t call_original_3161b8(
        void*& child,std::uint32_t) override {
        order.push_back("3161B8");assert(child==&created);return update;
    }
    std::uint32_t call_original_316174(
        OriginalManagerChild70& controller,std::uint32_t) override {
        order.push_back("316174");
        assert(controller.word0c==0);
        if(failRealOwner)throw std::runtime_error("316174 unresolved");
        return done;
    }
    bool call_original_vectors_equal_57c1e0(
        const OriginalManagerChild70&) override {
        order.push_back("PCEQW");return vectorEqual;
    }
    std::uint8_t read_original_6354e7() override {
        order.push_back("6354E7");return globalE7;
    }
    std::uint8_t read_original_6354e0() override {
        order.push_back("6354E0");return globalE0;
    }
    void call_original_21db64(OriginalManagerChild70&) override {
        order.push_back("21DB64");
    }
};
static void same(const Calls& calls,std::initializer_list<const char*> expected) {
    assert(calls.order==std::vector<std::string>(expected.begin(),expected.end()));
}
int main() {
    OriginalManagerChild70 controller{};
    Calls calls{};calls.active=&controller;
    const auto run=[&](std::uint32_t a1) {
        return originalManagerMovementUpdate_57bf5c(controller,a1,calls);
    };
    // Source 57C004 C.OLT.s against original -0.707: early pair
    // from 0x708A30; no 21B854/constructor/update called.
    calls.direction=-0.8f;
    assert(run(0x42)==calls.done);
    assert(controller.word08==0x57C27C && controller.word0c==0);
    same(calls,{"VU-direction","316174"});
    // Strict branch: equality to original constant is NOT early.
    calls.order.clear();calls.direction=-0.707f;calls.metric=1;
    controller.word08=0;controller.word0c=7;controller.flag1c=1;
    assert(run(0)==calls.done);
    same(calls,{"VU-direction","21B854","VU-metric","316174"});
    assert(controller.word08==0x57C27C);
    // Null child, >=2 source distance, two 441F40 initializers,
    // original 36F5F4, genuine 30C57C and 3161B8.
    calls.order.clear();calls.direction=0;calls.metric=2;
    controller.flag1c=0;controller.word14=7;
    assert(run(9)==calls.done);
    same(calls,{"VU-direction","VU-metric","441F40","441F40",
                "36F5F4","30C57C","3161B8","316174"});
    assert(controller.child10==&calls.created);
    // Already present source child must bypass ALL VU work.
    calls.order.clear();calls.failExactVu=true;controller.word14=2;
    controller.flag1c=0;controller.alternative30=0;
    assert(run(10)==2);
    same(calls,{"3161B8"});
    calls.order.clear();controller.alternative30=1;
    calls.vectorEqual=true;
    assert(run(10)==2);same(calls,{"3161B8","PCEQW"});
    calls.order.clear();calls.vectorEqual=false;
    assert(run(10)==1);same(calls,{"3161B8","PCEQW"});
    calls.order.clear();controller.flag1c=1;
    assert(run(11)==calls.done);same(calls,{"3161B8","316174"});
    assert(controller.word08==0x57C48C);
    // Failed 3161B8 skips method publication, preserves source
    // pair and reads diagnostic globals only along the retail gate.
    calls.order.clear();controller.word08=0x112233;
    calls.update=0;calls.globalE7=0;
    assert(run(12)==0);assert(controller.word08==0x112233);
    same(calls,{"3161B8","6354E7"});
    calls.order.clear();calls.globalE7=1;calls.globalE0=1;
    assert(run(12)==0);
    same(calls,{"3161B8","6354E7","6354E0"});
    calls.order.clear();calls.globalE0=0;
    assert(run(12)==0);
    same(calls,{"3161B8","6354E7","6354E0","21DB64"});
    // Game owner must supply real source VU and 316174. No
    // default synthetic vector or success fallback is permitted.
    calls.order.clear();calls.update=1;
    calls.failRealOwner=true;controller.word14=3;
    bool refused=false;try{(void)run(13);}catch(const std::runtime_error&){refused=true;}
    assert(refused && controller.word08==0x57C27C);
    calls.failRealOwner=false;
    calls.failExactVu=true;controller.child10=nullptr;calls.order.clear();
    refused=false;try{(void)run(14);}catch(const std::runtime_error&){refused=true;}
    assert(refused && controller.child10==nullptr);
    // Original 0x57C164 rereads child+0x10 after 30C57C and
    // destroys any synchronously installed prior original child.
    calls.failExactVu=false;calls.publishOldDuringCreate=true;
    calls.metric=3;calls.order.clear();controller.word14=0;
    assert(run(15)==calls.done);
    assert(controller.child10==&calls.created);
    same(calls,{"VU-direction","21B854","VU-metric","441F40",
                "441F40","36F5F4","30C57C","child-delete",
                "3161B8","316174"});
    // NaN source comparisons are false (C.OLT.S unordered),
    // so no invented early exit can be taken.
    controller.child10=nullptr;calls.publishOldDuringCreate=false;
    calls.direction=std::numeric_limits<float>::quiet_NaN();
    calls.metric=std::numeric_limits<float>::quiet_NaN();
    calls.order.clear();controller.flag1c=0;
    assert(run(16)==calls.done);
    assert(controller.child10==&calls.created);
    std::cout<<"ORIGINAL_57BF5C_CONTROL_FLOW_PASSED\n";
}
