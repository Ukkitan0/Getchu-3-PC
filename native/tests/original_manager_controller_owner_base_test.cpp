#include "ae3/original_manager_controller_owner_base.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ae3;
namespace {
void check(bool ok,const char* reason) {
    if(!ok) throw std::runtime_error(reason);
}

struct SourceDispatch final:OriginalManagerDispatchCalls_57c620 {
    std::uint32_t call_original_direct_57c7ac(
        std::uint32_t,std::int32_t) override {
        throw std::runtime_error("unrecovered source direct method refused");
    }
    std::uint32_t call_original_virtual_57c79c(
        std::uint32_t,std::int32_t) override {
        throw std::runtime_error("unrecovered tagged source method refused");
    }
    void call_original_30ffd0(void*,void*) override {
        throw std::runtime_error("unavailable original polygon adopter");
    }
    std::uint8_t read_original_6354e7() override {return 0;}
    std::uint8_t read_original_6354e0() override {return 0;}
    void call_original_2478ec_247d84_warning() override {
        throw std::runtime_error("unavailable original diagnostics");
    }
};
struct ActualVtable final:OriginalManagerUpdateOwner_316174 {
    OriginalManagerChild70* controller=nullptr;
    OriginalUpdateDepth_696c98* depth=nullptr;
    OriginalManagerNativeMethodOwner* methods=nullptr;
    int actualSourceChild=15;
    int sourceChildTicks=0,controllerTicks=0,deletions=0;
    bool unavailable=false;
    std::vector<std::uint32_t> childNesting,controllerNesting;

    std::uint32_t call_original_vtable5(void* object,std::uint32_t a1) override {
        check(depth!=nullptr && depth->depth!=0,
              "original 0x696C98 not shared across controller owner methods");
        if(object==&actualSourceChild) {
            ++sourceChildTicks;childNesting.push_back(depth->depth);
            if(unavailable)throw std::runtime_error("source child virtual slot 5 missing");
            return 2; // Source fixture status2, NOT native implementation.
        }
        check(object==controller && methods!=nullptr,
              "source continuation vtable[5] must address actual controller");
        ++controllerTicks;controllerNesting.push_back(depth->depth);
        return originalManagerDispatchNativeMethods_57c620(*controller,a1,*methods);
    }
    void call_original_child_vtable1_delete(void* obj) override {
        check(obj==&actualSourceChild,"wrong original child destructor");
        ++deletions;
    }
};
struct GameSpecificOwner final:OriginalManagerControllerOwnerBase {
    ActualVtable& vtable;
    explicit GameSpecificOwner(
        OriginalManagerChild70& controller,
        OriginalUpdateDepth_696c98& depth,ActualVtable& vt)
        :OriginalManagerControllerOwnerBase(controller,depth,vt),vtable(vt){}

    // These are deliberately fixture implementations; the native
    // production owner still needs the ACTUAL game data/functions.
    float call_original_vu0_direction_57bfac(OriginalManagerChild70&) override {
        throw std::runtime_error("retail original 57BFAC VU0 not provided");
    }
    void call_original_21b854(OriginalManagerChild70&) override {
        throw std::runtime_error("retail projection missing");
    }
    float call_original_vu0_metric_57c090(OriginalManagerChild70&) override {
        throw std::runtime_error("retail original 57C090 VU0 not provided");
    }
    void call_original_36f5f4(OriginalMovementScratch_57bf5c&) override {
        throw std::runtime_error("retail original 36F5F4 missing");
    }
    void* call_original_30c57c(
        OriginalManagerChild70&,
        const OriginalMovementScratch_57bf5c&) override {
        throw std::runtime_error("retail source 30C57C child creation missing");
    }
    std::uint8_t read_original_6354e7() override {return 0;}
    std::uint8_t read_original_6354e0() override {return 0;}
    void call_original_21db64(OriginalManagerChild70&) override {
        throw std::runtime_error("source diagnostic owner unavailable");
    }
    std::uint32_t call_original_3176ac(void*) override {return 0;}
    void* call_original_30c1d4(void*,std::uint32_t) override {
        throw std::runtime_error("unexpected 30C1D4 constructor");
    }
    std::array<std::byte,16> read_original_parent14_vector40(
        const OriginalManagerChild70&) override {
        return {}; // fixture input, not a replacement game vector
    }
    void call_original_57c7cc(OriginalManagerChild70&,std::uint32_t) override {
        throw std::runtime_error("retail actor route owner unavailable");
    }
    void* call_original_30c21c(
        OriginalManagerChild70&,
        const std::array<std::byte,16>&) override {
        return &vtable.actualSourceChild;
    }
    void* call_original_30c520(OriginalManagerChild70&) override {
        return &vtable.actualSourceChild;
    }
    void* call_original_30c360(OriginalManagerChild70&) override {
        return &vtable.actualSourceChild;
    }
};
}
int main() {
    OriginalManagerChild70 controller{},foreign{};
    OriginalUpdateDepth_696c98 originalGlobalDepth{};
    ActualVtable genuineChildAndControllerVtables{};
    SourceDispatch sourceDispatcher{};
    genuineChildAndControllerVtables.controller=&controller;
    genuineChildAndControllerVtables.depth=&originalGlobalDepth;
    GameSpecificOwner originalOwner(
        controller,originalGlobalDepth,genuineChildAndControllerVtables);
    auto methods=originalOwner.nativeMethodOwner(sourceDispatcher);
    genuineChildAndControllerVtables.methods=&methods;
    controller.word08=0x57C27C;
    controller.word0c=0;
    controller.alternative30=0;
    controller.flag1c=0;

    // Source 57C27C original readiness fails and transitions to
    // 57C388 through 316174. That route action creates a real
    // source fixture child, which runs 3161B8 -> 316174, returns
    // original status2, deletes the child and transitions to
    // 57C48C through the SAME shared nesting global.
    const auto result=originalManagerDispatchNativeMethods_57c620(
        controller,0,methods);
    check(result==2 && controller.word08==0x57C48C &&
          controller.word0c==0 && controller.word14==2 &&
          controller.child10==nullptr,
          "source 57C27C->57C388->57C48C nested transitions failed");
    check(genuineChildAndControllerVtables.sourceChildTicks==2 &&
          genuineChildAndControllerVtables.controllerTicks==2 &&
          genuineChildAndControllerVtables.deletions==2 &&
          originalGlobalDepth.depth==0,
          "actual 3161B8 common lifecycle not shared across methods");
    check(genuineChildAndControllerVtables.controllerNesting==
              std::vector<std::uint32_t>({1,2}) &&
          genuineChildAndControllerVtables.childNesting==
              std::vector<std::uint32_t>({2,3}),
          "original 0x696C98 nesting was not preserved at each virtual call");

    // Separate native type-two method MUST use the same 3161B8
    // implementation without another independently written callback.
    controller.word08=0x57C554;
    check(originalManagerDispatchNativeMethods_57c620(
              controller,0,methods)==2 &&
          controller.word14==2 && controller.child10==nullptr &&
          genuineChildAndControllerVtables.sourceChildTicks==3 &&
          genuineChildAndControllerVtables.deletions==3 &&
          originalGlobalDepth.depth==0,
          "type-two owner diverged from common original tick behavior");

    // Identical signatures in all five source callback base classes
    // must resolve to the SINGLE original owned-controller bridge.
    bool refused=false;
    try {(void)originalOwner.call_original_3161b8(foreign.child10,0);}
    catch(const std::invalid_argument&) {refused=true;}
    check(refused && foreign.child10==nullptr &&
          originalGlobalDepth.depth==0,
          "foreign original controller child pointer accepted");
    refused=false;
    try {(void)originalOwner.call_original_316174(foreign,0);}
    catch(const std::invalid_argument&) {refused=true;}
    check(refused && originalGlobalDepth.depth==0,
          "foreign original continuation accepted");

    // Missing real child method remains a hard failure, preserving
    // source +0x14 status, child ownership and update nesting.
    genuineChildAndControllerVtables.unavailable=true;
    controller.word08=0x57C554;
    const auto beforeDeletes=genuineChildAndControllerVtables.deletions;
    refused=false;
    try {(void)originalManagerDispatchNativeMethods_57c620(
             controller,0,methods);}
    catch(const std::runtime_error&) {refused=true;}
    check(refused && controller.child10==
                        &genuineChildAndControllerVtables.actualSourceChild &&
          controller.word14==2 &&
          genuineChildAndControllerVtables.deletions==beforeDeletes &&
          originalGlobalDepth.depth==0,
          "missing real child update was silently treated as success");
    std::cout<<"ORIGINAL_SHARED_CONTROLLER_OWNER_3161B8_PASSED\n";
}
