#include "ae3/original_actor_nav_controller.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool b,const char* m){if(!b)throw std::runtime_error(m);}
struct Child final: OriginalActorControllerChildCalls {
    std::vector<std::string> events;
    void* expected=nullptr;
    std::int32_t status=29;
    void check(void* p){ck(p==expected,"original child identity");}
    void call_virtual_04_destroy(void* p) override {check(p);events.emplace_back("destroy");}
    std::int32_t call_virtual_08_status(void* p) override {check(p);events.emplace_back("status");return status;}
    void call_virtual_0c_notify(void* p) override {check(p);events.emplace_back("notify");}
    std::int32_t call_virtual_10_forward(void* p,std::uint32_t a5,std::int32_t a6) override {
        check(p);ck(a5==11&&a6==-7,"original forward method arguments");
        events.emplace_back("forward");return 123;
    }
};
struct Free final: OriginalActorControllerDeallocateCalls {
    Child& child;
    void* expected;
    explicit Free(Child& c,void* p):child(c),expected(p){}
    void call_original_41a3b0(void* ptr) override {
        ck(ptr==expected,"original allocator identity");
        ck(!child.events.empty()&&child.events.back()=="destroy","child must be destroyed first");
        child.events.emplace_back("free");
    }
};
int main(){
    int actor=1,route=2,child=3,controller=4;
    for(std::int32_t count:std::array<std::int32_t,7>{
        std::numeric_limits<std::int32_t>::min(),-1,0,1,2,3,
        std::numeric_limits<std::int32_t>::max()}) {
        auto a=originalActorControllerPrepare_57ba98(&actor,&route,count,0xabcd);
        ck(a.originalActor==&actor&&a.originalRoute==&route,"original pointer identity");
        ck(a.routeHasAtLeastTwo==(count>=2),"original signed SLTI");
        ck(a.originalRequestedFlag==0xcd,"original SB low-byte flag");
        ck(a.variantAt30==0 && !a.vectorAt20.has_value(),"variant zero must leave +0x20 unchanged");
        ck(a.wordsAt08And0c[0]==0 && a.wordsAt08And0c[1]==0 &&
           a.childAt10==nullptr && a.wordAt14==0,"original cleared fields");
        ck(a.requiresOriginalPostConstructor_57bcb8,"full movement initializer cannot be assumed");
    }
    std::array<std::byte,16> source{};
    for(std::size_t i=0;i<source.size();++i)source[i]=std::byte(i*11);
    const auto b=originalActorControllerPrepare_57baf8(&actor,&route,2,source,0x100);
    ck(b.variantAt30==1&&b.routeHasAtLeastTwo==1&&b.originalRequestedFlag==0,
       "original variant-one flags");
    ck(b.vectorAt20 && *b.vectorAt20==source,"original 16-byte SQ at +0x20");
    bool failed=false;
    try{(void)originalActorControllerPrepare_57ba98(&actor,nullptr,0,0);}
    catch(const std::invalid_argument&){failed=true;}
    ck(failed,"host cannot dereference missing route");
    Child callbacks;callbacks.expected=&child;
    ck(originalActorControllerStatus_57bc0c(true,&child,callbacks)==3,"actor+0x40 priority");
    ck(callbacks.events.empty(),"actor status should bypass child");
    ck(originalActorControllerStatus_57bc0c(false,nullptr,callbacks)==0,"null child status");
    ck(originalActorControllerStatus_57bc0c(false,&child,callbacks)==29,"virtual+8 status");
    ck(originalActorControllerForward_57bc54(nullptr,11,-7,callbacks)==-7,"missing child returns a2");
    ck(originalActorControllerForward_57bc54(&child,11,-7,callbacks)==123,"virtual+10 forwarded");
    originalActorControllerNotify_57bc88(nullptr,callbacks);
    originalActorControllerNotify_57bc88(&child,callbacks);
    originalActorControllerDestroy_57bb60(nullptr,callbacks);
    originalActorControllerDestroy_57bb60(&child,callbacks);
    Free allocator(callbacks,&controller);
    originalActorControllerDelete_57bbb4(&controller,&child,callbacks,allocator);
    ck(callbacks.events==std::vector<std::string>({"status","forward","notify","destroy","destroy","free"}),
       "original child dispatch and deletion order");
}
