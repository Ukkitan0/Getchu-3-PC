#include "ae3/original_retail_call_chains.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
void check(bool v){if(!v)throw std::runtime_error("Original retail game call sequence differs");}
using Log=std::vector<std::string>;
struct Monkey final:ae3::OriginalMonkeyNativeCalls {
 Log log;
 void call_child_virtual_1(void* p)override{check(p!=(void*)0);log.push_back("destroy");}
 void* call_3126e8(void* p)override{check(p==(void*)0x10);log.push_back("factory8");return (void*)0x30;}
 void* call_312760(void* p,std::uint32_t a,std::uint32_t b)override{check(p==(void*)0x10&&a==17&&b==29);log.push_back("factory60");return (void*)0x40;}
 void call_child_virtual_3(void* p)override{check(p==(void*)0x40);log.push_back("virtual3");}
 void call_310730(void* p)override{check(p==(void*)0x10);log.push_back("310730");}
 void call_3168d0(void* p)override{check(p==(void*)0x50);log.push_back("3168d0");}
 void call_30fda0(void* p)override{check(p==(void*)0x10);log.push_back("30fda0");}
};
struct Lifecycle final:ae3::OriginalMonkeyLifecycleCalls {
 Log log; bool release=false;ae3::OriginalMonkeyCallGlobals* global=nullptr;
 void call_310890(void* p)override{check(p==(void*)0x10);log.push_back("310890");}
 bool call_attached_virtual_slot_5(void* p,std::uint32_t flags)override{
   check(p==(void*)0x20&&flags==0);
   check(global && global->active_nested_calls_696c98==1);
   log.push_back("virtual5");return release;
 }
 void call_3108d4(void* p)override{check(p==(void*)0x10);log.push_back("3108d4");}
 void call_attached_virtual_1(void* p)override{check(p==(void*)0x20);log.push_back("virtual1");}
 void call_30fc4c(void* p)override{check(p==(void*)0x10);log.push_back("30fc4c");}
 void call_41a3b0(void* p)override{check(p==(void*)0x10);log.push_back("41a3b0");}
};
struct Player final:ae3::OriginalPlayerNativeCalls {
 Log log;
 void call_33f240(void* p)override{check(p==(void*)0x80);log.push_back("33f240");}
 void call_33f26c(void* p)override{check(p==(void*)0x80);log.push_back("33f26c");}
};
struct Cleanup final:ae3::OriginalPlayerCleanupCalls {
 Log log;
 void call_34310c(void* p)override{check(p==(void*)0x110);log.push_back("34310c");}
 void call_3511d4(void* p)override{check(p==(void*)0x120);log.push_back("3511d4");}
};
struct Small final:ae3::OriginalSmallFactoryCalls {
 Log log;
 void call_2762bc(void* p,std::uint32_t n)override{check(p==(void*)0x230&&n==13);log.push_back("2762bc");}
 void call_277118(void* p,std::uint32_t n,float x)override{check(p==(void*)0x230&&n==0&&x==0.2f);log.push_back("277118");}
};
struct Chips final:ae3::OriginalChipSideEffectCalls {
 Log log;void* scratch=nullptr;
 void call_29164c(void* p,std::uint32_t n)override{check(p==(void*)0x70&&n==1);log.push_back("29164c");}
 void call_35ab88(void* p)override{scratch=p;log.push_back("35ab88");}
 void call_362178(void* p)override{check(p==scratch&&p!=nullptr);log.push_back("362178");}
 void call_2acc6c(void* p)override{check(p==(void*)0x40);log.push_back("2acc6c");}
};
struct EventSingletonFactory final:ae3::OriginalGameEventFactoryCalls {
 Log log;bool fail=false;
 void* call_41b800(std::uint32_t n)override{check(n==0x18);log.push_back("alloc18");return fail?nullptr:(void*)0x100;}
 void call_291560(void* p)override{check(p==(void*)0x100);log.push_back("291560");}
 void call_41a3b0(void* p)override{check(p==(void*)0x100);log.push_back("free");}
};
}
int main(){
 Monkey m;ae3::OriginalMonkeyOwnedObjects o{(void*)0x10,(void*)0x20};
 ae3::originalMonkeyReplaceAttachment_57173c(o,m);
 check(o.attached_at_64==(void*)0x30&&m.log==Log({"destroy","factory8"}));
 m.log.clear();
 ae3::originalMonkeyReplaceAttachment_57178c(o,17,29,m);
 check(o.attached_at_64==(void*)0x40&&m.log==Log({"destroy","factory60"}));
 m.log.clear();ae3::originalMonkeyRunAttachment_5718cc(o,m);
 check(m.log==Log({"virtual3","310730"}));
 m.log.clear();ae3::originalMonkeyForward_571668((void*)0x50,m);
 ae3::originalMonkeyForward_5716a8(o,m);
 check(m.log==Log({"3168d0","30fda0"}));
 o.attached_at_64=nullptr;m.log.clear();
 ae3::originalMonkeyReplaceAttachment_57173c(o,m);
 check(m.log==Log({"factory8"}));
 o.attached_at_64=nullptr;
 bool failed=false;try{ae3::originalMonkeyRunAttachment_5718cc(o,m);}catch(const std::runtime_error&){failed=true;}
 check(failed);
 Lifecycle l;ae3::OriginalMonkeyOwnedObjects state{(void*)0x10,(void*)0x20};
 ae3::OriginalMonkeyCallGlobals globals{};l.global=&globals;
 for(std::uint32_t a: {0u,1u,0x100u,0x101u})
   for(std::uint32_t b: {0u,1u,0x100u,0x101u})
     check(ae3::originalMonkeyComposeFlags_571e10(a,b)==((a&0xff?1u:0u)|(b&0xff?2u:0u)));
 check(!ae3::originalMonkeyUpdateLifecycle_571904(state,globals,l));
 check(l.log==Log({"310890","virtual5","3108d4"}));
 check(state.primary_at_60&&(state.attached_at_64));
 l.release=true;l.log.clear();
 check(ae3::originalMonkeyUpdateLifecycle_571904(state,globals,l));
 check(l.log==Log({"310890","virtual5","virtual1","30fc4c","41a3b0"}));
 check(!state.primary_at_60&&!state.attached_at_64&&state.source_state_at_4c==2);
 check(globals.active_nested_calls_696c98==0);
 check(!ae3::originalMonkeyTransitionToMode2_20af68(state));
 EventSingletonFactory fc;ae3::OriginalGameEventSingleton stored{};
 check(ae3::originalGameEventGet_29157c(stored)==nullptr);
 ae3::originalGameEventCreate_291588(stored,fc);
 check(ae3::originalGameEventGet_29157c(stored)==(void*)0x100);
 check(fc.log==Log({"alloc18","291560"}));
 ae3::originalGameEventDestroy_2915c0(stored,fc);
 check(ae3::originalGameEventGet_29157c(stored)==nullptr);
 check(fc.log==Log({"alloc18","291560","free"}));
 fc.fail=true;bool allocFailure=false;
 try{ae3::originalGameEventCreate_291588(stored,fc);}catch(const std::runtime_error&){allocFailure=true;}
 check(allocFailure);
 Player p;ae3::originalPlayerForward_349fe4((void*)0x80,p);
 ae3::originalPlayerForward_349ffc((void*)0x80,p);
 check(p.log==Log({"33f240","33f26c"}));
 Cleanup c;ae3::originalPlayerCleanup_33f240((void*)0x110,(void*)0x120,c);
 check(c.log==Log({"34310c","3511d4"}));
 Small sm;auto obj=ae3::originalSmallMonkeyFactory_3126e8((void*)0x200,(void*)0x230,sm);
 check(obj.parent==(void*)0x200&&obj.original_vtable_va==0x6b1240);
 check(sm.log==Log({"2762bc","277118"}));
 Chips ch;ae3::PlayerProgress progress{};progress.player_left=98;
 ae3::OriginalGameEventSingleton singleton{(void*)0x70};
 ae3::originalChipSideEffect_33fd90(progress,singleton,ch);
 check(progress.player_left==99&&ch.log==Log({"29164c"}));
 ch.log.clear();ae3::originalChipSideEffect_33fd90(progress,singleton,ch);
 check(progress.player_left==99&&ch.log==Log({"29164c"}));
 check(ae3::originalAddPlayerLives_26fa1c(progress,-1000)==0);
 progress.player_left=std::numeric_limits<std::int32_t>::max();
 check(ae3::originalAddPlayerLives_26fa1c(progress,1)==0);
 ch.log.clear();ae3::originalChipSideEffect_349b2c(ch);
 check(ch.log==Log({"35ab88","362178"}));
 ch.log.clear();check(!ae3::originalChipNotification_2915ec(nullptr,ch)&&ch.log.empty());
 check(ae3::originalChipNotification_2915ec((void*)0x40,ch));
 check(ch.log==Log({"2acc6c"}));
}
