#include "ae3/original_manager_route_child_57c27c.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
struct Calls final : OriginalManagerRouteChildCalls_57c27c {
    std::vector<std::string> events;
    std::uint32_t ready=1, updated=1, done=0xDEAD1234;
    std::uint32_t seenA1=0;
    int parent=0,created=1,prior=2;
    bool injectedOldDuringCreation=false;
    bool throwAtDone=false;
    void* expectedParent=nullptr;
    void* current=nullptr;
    std::uint32_t call_original_3176ac(void* p) override {
        assert(p==expectedParent); events.emplace_back("3176AC");return ready;
    }
    void* call_original_30c1d4(void* p,std::uint32_t a1) override {
        assert(p==expectedParent);seenA1=a1;events.emplace_back("30C1D4");
        if(injectedOldDuringCreation)
            *static_cast<void**>(current)=&prior;
        return &created;
    }
    void call_original_child_vtable1_delete(void* p) override {
        assert(p==&prior);events.emplace_back("destroy");
    }
    std::uint32_t call_original_3161b8(void*& p,std::uint32_t a1) override {
        assert(a1==seenA1 || seenA1==0);events.emplace_back("3161B8");
        assert(p==&created || p==&prior || p==nullptr);
        return updated;
    }
    std::uint32_t call_original_316174(OriginalManagerChild70& c,std::uint32_t a1) override {
        events.emplace_back("316174");assert(a1==seenA1 || seenA1==0);
        assert(c.word08==0x57c388 && c.word0c==0);
        if (throwAtDone) throw std::runtime_error("actual original 316174 not available");
        return done;
    }
};
int main() {
    OriginalManagerChild70 c{}; Calls cb{};c.parent04=&cb.parent;
    cb.expectedParent=c.parent04;cb.current=&c.child10;
    c.word08=0xDEAD;c.word0c=2;
    // Source 0x3176AC false: writes next method + calls actual 316174,
    // no construction and no 3161B8.
    cb.ready=0;assert(originalManagerRouteChildUpdate_57c27c(c,0xABCD,cb)==cb.done);
    assert(c.child10==nullptr && c.word08==0x57c388 && c.word0c==0);
    assert(cb.events==std::vector<std::string>({"3176AC","316174"}));
    cb.events.clear(); cb.ready=1;cb.seenA1=0;
    c.word08=0x1234;c.word0c=6;
    assert(originalManagerRouteChildUpdate_57c27c(c,0xC011,cb)==cb.done);
    assert(c.child10==&cb.created);
    assert(cb.events==std::vector<std::string>({"3176AC","30C1D4","3161B8","316174"}));
    cb.events.clear();cb.seenA1=0;
    // Already-present child never calls source readiness/constructor.
    assert(originalManagerRouteChildUpdate_57c27c(c,0x77,cb)==cb.done);
    assert(cb.events==std::vector<std::string>({"3161B8","316174"}));
    cb.events.clear();cb.updated=0;c.word08=0x12345678;c.word0c=0x1234;
    assert(originalManagerRouteChildUpdate_57c27c(c,0xA1,cb)==0);
    assert(c.word08==0x12345678 && c.word0c==0x1234);
    assert(cb.events==std::vector<std::string>({"3161B8"}));
    cb.events.clear();cb.updated=1;cb.injectedOldDuringCreation=true;
    cb.seenA1=0;c.child10=nullptr;
    assert(originalManagerRouteChildUpdate_57c27c(c,0xFE,cb)==cb.done);
    assert(c.child10==&cb.created);
    assert(cb.events==std::vector<std::string>({"3176AC","30C1D4","destroy","3161B8","316174"}));
    cb.events.clear();cb.seenA1=0;cb.throwAtDone=true;
    bool failed=false;try {(void)originalManagerRouteChildUpdate_57c27c(c,0xF1,cb);}
    catch(const std::runtime_error&){failed=true;}
    assert(failed && c.word08==0x57c388 && cb.events==
           std::vector<std::string>({"3161B8","316174"}));
    std::cout<<"ORIGINAL_MANAGER_ROUTE_CHILD_57C27C_PASSED\n";
}
