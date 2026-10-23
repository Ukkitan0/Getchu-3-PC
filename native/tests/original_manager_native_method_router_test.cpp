#include "ae3/original_manager_native_method_router.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
struct O:OriginalManagerDispatchCalls_57c620 {
    int adopted=0,unknown=0,virtuals=0;
    int polygon=1;void* parent=nullptr;
    std::uint32_t call_original_direct_57c7ac(
        std::uint32_t,std::int32_t) override {
        ++unknown;throw std::runtime_error("unrecovered original method");
    }
    std::uint32_t call_original_virtual_57c79c(
        std::uint32_t,std::int32_t) override {
        ++virtuals;throw std::runtime_error("unrecovered tagged method");
    }
    void call_original_30ffd0(void* p,void* target) override {
        assert(p==parent && target==&polygon);++adopted;
    }
    std::uint8_t read_original_6354e7() override{return 0;}
    std::uint8_t read_original_6354e0() override{return 0;}
    void call_original_2478ec_247d84_warning() override {
        throw std::runtime_error("original diagnostics required");
    }
};
struct R:OriginalManagerRouteChildCalls_57c27c {
    int child=10;int ready=0,updates=0;void* parent=nullptr;
    std::uint32_t call_original_3176ac(void* p) override {
        assert(p==parent);return ready;
    }
    void* call_original_30c1d4(void*,std::uint32_t) override {return &child;}
    void call_original_child_vtable1_delete(void*) override {assert(false);}
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        ++updates;return 1;
    }
    std::uint32_t call_original_316174(
        OriginalManagerChild70&,std::uint32_t) override{return 1;}
};
struct G:OriginalManagerRoutedCalls_57c388 {
    int child=11,route=0;std::array<std::byte,16> vector{};
    std::array<std::byte,16> read_original_parent14_vector40(
        const OriginalManagerChild70&) override{return vector;}
    void call_original_57c7cc(
        OriginalManagerChild70&,std::uint32_t) override{++route;}
    void* call_original_30c21c(
        OriginalManagerChild70&,const std::array<std::byte,16>&) override {
        return &child;
    }
    void call_original_child_vtable1_delete(void*) override {assert(false);}
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        return 1;
    }
    std::uint32_t call_original_316174(
        OriginalManagerChild70&,std::uint32_t) override{return 1;}
};
struct F:OriginalManagerFollowupCalls_57c48c {
    int child=12;
    void* call_original_30c520(OriginalManagerChild70&) override {
        return &child;
    }
    void call_original_child_vtable1_delete(void*) override {assert(false);}
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        return 1;
    }
    bool call_original_variant_vector_equal_57c508(
        const OriginalManagerChild70&) override{return true;}
};
struct T:OriginalManagerType2Calls_57c554 {
    int child=13;
    void* call_original_30c360(OriginalManagerChild70&) override {
        return &child;
    }
    void call_original_child_vtable1_delete(void*) override {assert(false);}
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        return 1;
    }
    bool call_original_variant_vector_equal_57c5d4(
        const OriginalManagerChild70&) override{return true;}
};
int main(){
    O outer;R r;G g;F f;T t;
    OriginalManagerNativeMethodOwner methods{r,g,f,t,outer};
    OriginalManagerChild70 c;
    c.parent04=&outer;outer.parent=&outer;r.parent=c.parent04;
    c.word14=0x89;c.flag1c=1;
    // Real outer 0x57C620 dispatch -> original 0x57C27C.
    c.word08=0x57c27c;
    assert(originalManagerDispatchNativeMethods_57c620(c,0,methods)==1);
    assert(c.word08==0x57c388 && c.word0c==0 && c.child10==nullptr);
    // Follow original method pair into 0x57C388 -> 0x57C48C.
    assert(originalManagerDispatchNativeMethods_57c620(c,0,methods)==1);
    assert(c.word08==0x57c48c && c.child10==&g.child && g.route==1);
    // Existing child forces the real method even with a1 bits set.
    assert(originalManagerDispatchNativeMethods_57c620(c,3,methods)==0x89);
    assert(c.word08==0x57c48c && outer.unknown==0);
    c.word08=0x57c554;
    assert(originalManagerDispatchNativeMethods_57c620(c,3,methods)==0x89);
    c.originalPendingPolygon50=&outer.polygon;
    assert(originalManagerDispatchNativeMethods_57c620(c,3,methods)==0x89);
    assert(outer.adopted==1 && c.originalPendingPolygon50==nullptr);
    // Unknown method, nonzero original subobject offset and tagged
    // virtual calls must go to the REAL owner; no fake native target.
    bool refused=false;c.word08=0x57bf5c;
    try{(void)originalManagerDispatchNativeMethods_57c620(c,0,methods);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && outer.unknown==1);
    c.word08=0x57c27c;c.word0c=2;refused=false;
    try{(void)originalManagerDispatchNativeMethods_57c620(c,0,methods);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && outer.unknown==2);
    c.word0c=1;refused=false;
    try{(void)originalManagerDispatchNativeMethods_57c620(c,0,methods);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && outer.virtuals==1);
    c.word08=0;
    assert(originalManagerDispatchNativeMethods_57c620(c,0,methods)==2);
    std::cout<<"ORIGINAL_MANAGER_NATIVE_METHOD_ROUTER_57C620_PASSED\n";
}
