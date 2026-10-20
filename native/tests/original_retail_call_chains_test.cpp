#include "ae3/original_retail_call_chains.hpp"
#include <cstdint>
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
 Log log; bool release=false;
 void call_310890(void* p)override{check(p==(void*)0x10);log.push_back("310890");}
 void call_571e10(void* p,std::uint32_t a,std::uint32_t b)override{
   check(p&&a==0&&b==0);auto* x=static_cast<std::uint8_t*>(p);
   x[0]=0xcd;x[1]=0xab;x[2]=0x34;x[3]=0x12;log.push_back("571e10");
 }
 bool call_316174(void* p,std::uint32_t n)override{check(p==(void*)0x20&&n==0x1234abcd);log.push_back("316174");return release;}
 void call_3108d4(void* p)override{check(p==(void*)0x10);log.push_back("3108d4");}
 void call_20af68()override{log.push_back("20af68");}
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
 void call_26fa1c(void* p,std::uint32_t n)override{check(p==(void*)0x649910&&n==1);log.push_back("26fa1c");}
 void* call_29157c()override{log.push_back("29157c");return (void*)0x70;}
 void call_29164c(void* p,std::uint32_t n)override{check(p==(void*)0x70&&n==1);log.push_back("29164c");}
 void call_35ab88(void* p)override{scratch=p;log.push_back("35ab88");}
 void call_362178(void* p)override{check(p==scratch&&p!=nullptr);log.push_back("362178");}
 void call_2acc6c(void* p)override{check(p==(void*)0x40);log.push_back("2acc6c");}
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
 check(!ae3::originalMonkeyUpdateLifecycle_571904(state,l));
 check(l.log==Log({"310890","571e10","316174","3108d4"}));
 check(state.primary_at_60&&(state.attached_at_64));
 l.release=true;l.log.clear();
 check(ae3::originalMonkeyUpdateLifecycle_571904(state,l));
 check(l.log==Log({"310890","571e10","316174","20af68","virtual1","30fc4c","41a3b0"}));
 check(!state.primary_at_60&&!state.attached_at_64);
 Player p;ae3::originalPlayerForward_349fe4((void*)0x80,p);
 ae3::originalPlayerForward_349ffc((void*)0x80,p);
 check(p.log==Log({"33f240","33f26c"}));
 Cleanup c;ae3::originalPlayerCleanup_33f240((void*)0x110,(void*)0x120,c);
 check(c.log==Log({"34310c","3511d4"}));
 Small sm;auto obj=ae3::originalSmallMonkeyFactory_3126e8((void*)0x200,(void*)0x230,sm);
 check(obj.parent==(void*)0x200&&obj.original_vtable_va==0x6b1240);
 check(sm.log==Log({"2762bc","277118"}));
 Chips ch;ae3::originalChipSideEffect_33fd90((void*)0x649910,ch);
 check(ch.log==Log({"26fa1c","29157c","29164c"}));
 ch.log.clear();ae3::originalChipSideEffect_349b2c(ch);
 check(ch.log==Log({"35ab88","362178"}));
 ch.log.clear();check(!ae3::originalChipNotification_2915ec(nullptr,ch)&&ch.log.empty());
 check(ae3::originalChipNotification_2915ec((void*)0x40,ch));
 check(ch.log==Log({"2acc6c"}));
}
