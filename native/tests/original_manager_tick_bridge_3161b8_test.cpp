#include "ae3/original_manager_tick_bridge_3161b8.hpp"
#include "ae3/original_manager_type2_update_57c554.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ae3;
namespace {
void check(bool good,const char* msg) {
    if(!good) throw std::runtime_error(msg);
}
struct SourceOwner final:OriginalManagerUpdateOwner_316174 {
    OriginalManagerChild70* controller=nullptr;
    OriginalUpdateDepth_696c98* depth=nullptr;
    int child=42;
    std::uint32_t childResult=2,controllerResult=0xabcdu;
    bool nested=false,throwOnUpdate=false,throwOnDelete=false;
    int childUpdates=0,controllerUpdates=0,deletes=0;
    std::vector<std::uint32_t> nesting;
    std::uint32_t call_original_vtable5(
        void* object,std::uint32_t a1) override {
        check(depth!=nullptr && depth->depth!=0,
              "original 0x696C98 must be incremented during slot5");
        nesting.push_back(depth->depth);
        if(throwOnUpdate)
            throw std::runtime_error("real original source vtable5 unavailable");
        if(object==&child) {
            ++childUpdates;
            check(a1==0x11u || a1==0x22u,
                  "original 0x3161B8 forwarded a1 incorrectly");
            return childResult;
        }
        check(object==controller,"source continuation was not controller vtable5");
        ++controllerUpdates;
        check(a1==0x33u,"source controller continuation a1 changed");
        if(nested)
            check(originalManagerGuardedUpdate_316174(
                &child,0x22,*depth,*this)==childResult,
                "original shared depth was not nested");
        return controllerResult;
    }
    void call_original_child_vtable1_delete(void* object) override {
        check(object==&child,"source slot1 destructor got foreign child");
        ++deletes;
        if(throwOnDelete)
            throw std::runtime_error("genuine original child destructor unavailable");
    }
};
struct Type2 final:OriginalManagerType2Calls_57c554 {
    OriginalManagerTickBridge_3161b8& tick;
    SourceOwner& owner;
    explicit Type2(OriginalManagerTickBridge_3161b8& t,SourceOwner& o)
        :tick(t),owner(o) {}
    void* call_original_30c360(OriginalManagerChild70&) override {
        return &owner.child;
    }
    void call_original_child_vtable1_delete(void* ptr) override {
        tick.call_original_child_vtable1_delete(ptr);
    }
    std::uint32_t call_original_3161b8(
        void*& ptr,std::uint32_t a1) override {
        return tick.call_original_3161b8(ptr,a1);
    }
};
}
int main() {
    OriginalManagerChild70 c{},foreign{};
    OriginalUpdateDepth_696c98 depth{};
    SourceOwner source{};
    source.controller=&c;source.depth=&depth;
    OriginalManagerTickBridge_3161b8 tick(c,depth,source);
    Type2 controllerSource(tick,source);

    // Native 0x57C554 -> 0x3161B8 -> 0x316174 -> actual source
    // slot5, status+0x14, original slot1 delete and child+0x10 clear.
    c.alternative30=0;
    check(originalManagerType2Update_57c554(c,0x11,controllerSource)==2,
          "source type2 status2 return was not propagated");
    check(c.word14==2 && c.child10==nullptr && source.childUpdates==1 &&
          source.deletes==1 && depth.depth==0,
          "original child lifecycle did not complete");
    check(source.nesting==std::vector<std::uint32_t>{1},
          "source global nesting during child update changed");

    // Source status zero leaves controller child alive and returns
    // zero without consulting any variant-one vector.
    source.childResult=0;
    check(originalManagerType2Update_57c554(c,0x11,controllerSource)==0,
          "original active child should stop at 3161B8 zero gate");
    check(c.child10==&source.child && c.word14==0 &&
          source.deletes==1 && depth.depth==0,
          "active original child or status was lost");

    // Original 0x316174 called on CONTROLLER after an installed
    // method pair; true game owner supplies its own virtual slot5.
    source.nested=true;
    check(tick.call_original_316174(c,0x33)==source.controllerResult,
          "original controller continuation virtual result changed");
    check(source.nesting.size()==4 &&
          source.nesting[2]==1 && source.nesting[3]==2 &&
          depth.depth==0,
          "original shared update depth does not support nesting");
    source.nested=false;

    // Host must never route a different controller's +0x10 pointer
    // through this original game owner or publish foreign status.
    const auto before=c.word14;
    bool rejected=false;
    try {(void)tick.call_original_3161b8(foreign.child10,0x11);}
    catch(const std::invalid_argument&) {rejected=true;}
    check(rejected && c.word14==before && depth.depth==0 &&
          foreign.child10==nullptr,
          "source child-pointer alias invariant bypassed");
    rejected=false;
    try {(void)tick.call_original_316174(foreign,0x33);}
    catch(const std::invalid_argument&) {rejected=true;}
    check(rejected && depth.depth==0,"foreign controller continuation accepted");

    // Actual source errors must propagate, not fabricate completion.
    source.throwOnUpdate=true;rejected=false;
    try {(void)tick.call_original_3161b8(c.child10,0x11);}
    catch(const std::runtime_error&) {rejected=true;}
    check(rejected && depth.depth==0 && c.word14==before &&
          c.child10==&source.child,
          "original missing slot5 mutated controller status/lifetime");
    source.throwOnUpdate=false;
    source.childResult=2;source.throwOnDelete=true;rejected=false;
    try {(void)tick.call_original_3161b8(c.child10,0x11);}
    catch(const std::runtime_error&) {rejected=true;}
    check(rejected && depth.depth==0 && c.child10==&source.child &&
          c.word14==2,
          "failed genuine destructor silently erased original child");

    rejected=false;
    try {tick.call_original_child_vtable1_delete(nullptr);}
    catch(const std::invalid_argument&) {rejected=true;}
    check(rejected,"invalid original child destructor pointer accepted");
    std::cout<<"ORIGINAL_CONTROLLER_TICK_BRIDGE_3161B8_PASSED\n";
    return 0;
}
