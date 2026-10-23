#include "ae3/original_manager_child.hpp"
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool b){if(!b)throw std::runtime_error("Original controller differs from ELF");}
struct Calls final:OriginalManagerChildCalls {
  std::vector<std::string> calls;
  OriginalManagerChild70* expectedCtor=nullptr;
  std::array<std::byte,16> expectedVector20{};
  bool expectPreservedVariantZero=false;
  void call_57bcb8(OriginalManagerChild70& obj)override {
    calls.emplace_back("57bcb8");
    if(expectedCtor)check(expectedCtor==&obj);
    check(obj.word08==0&&obj.word0c==0&&!obj.child10&&obj.word14==0);
    if (obj.alternative30 || expectPreservedVariantZero)
      check(obj.controller20 && *obj.controller20==expectedVector20);
    else check(!obj.controller20);
    // Neither original constructor stores +0x40. Unknown source SQ
    // must remain absent, or previously owned genuine bytes survive.
    if (expectPreservedVariantZero)
      check(obj.controller40 && *obj.controller40==expectedVector20);
    else check(!obj.controller40);
  }
  void call_child_virtual1_delete(void* p)override{check(p==this);calls.emplace_back("childdelete");}
  std::uint32_t call_child_virtual2(void* p)override{check(p==this);calls.emplace_back("childv2");return 17;}
  std::uint32_t call_child_virtual4(void* p,std::uint32_t a,std::uint32_t b)override{
    check(p==this&&a==9&&b==38);calls.emplace_back("childv4");return 73;
  }
  void call_child_virtual3(void* p)override{check(p==this);calls.emplace_back("childv3");}
  void call_41a3b0(void* p)override{check(p==expectedCtor);calls.emplace_back("free");}
};
int main(){
 Calls calls;
 for(std::size_t i=0;i<calls.expectedVector20.size();++i)
   calls.expectedVector20[i]=std::byte((i*13u+7u)&255u);
 for(int alternative=0;alternative<2;alternative++){
   for(std::int32_t at04:{-3,0,1,2,8}){
     OriginalManagerChild70 obj{};
     calls.expectedCtor=&obj;
     if(!alternative)originalManagerChildCtor_57ba98(obj,&calls,&calls,at04,0xf9,calls);
     else originalManagerChildCtor_57baf8(obj,&calls,&calls,at04,
                                           calls.expectedVector20,0xf9,calls);
     check(obj.parent04==&calls&&obj.context18==&calls);
     check(obj.flag1c==(at04>=2?1:0)&&obj.flag1d==0xf9&&obj.alternative30==alternative);
     check(calls.calls==std::vector<std::string>({"57bcb8"}));calls.calls.clear();
     check(originalManagerChildVirtual2_57bc0c(obj,0,calls)==0&&calls.calls.empty());
     check(originalManagerChildVirtual2_57bc0c(obj,7,calls)==3&&calls.calls.empty());
     check(originalManagerChildVirtual4_57bc54(obj,9,38,calls)==38&&calls.calls.empty());
     originalManagerChildVirtual3_57bc88(obj,calls);check(calls.calls.empty());
     obj.child10=&calls;
     check(originalManagerChildVirtual2_57bc0c(obj,1,calls)==3&&calls.calls.empty());
     check(originalManagerChildVirtual2_57bc0c(obj,0,calls)==17);
     check(originalManagerChildVirtual4_57bc54(obj,9,38,calls)==73);
     originalManagerChildVirtual3_57bc88(obj,calls);
     check(calls.calls==std::vector<std::string>({"childv2","childv4","childv3"}));calls.calls.clear();
     originalManagerChildDestroy_57bb60(obj,calls);
     check(calls.calls==std::vector<std::string>({"childdelete"}));calls.calls.clear();
     originalManagerChildDeletingDestroy_57bbb4(obj,calls);
     check(calls.calls==std::vector<std::string>({"childdelete","free"}));calls.calls.clear();
   }
 }
 calls.expectedCtor=nullptr;
 auto first=originalManagerChildNew_317538(&calls,&calls,2,7,calls);
 check(first->flag1c==1&&first->alternative30==0);
 auto second=originalManagerChildNew_31759c(
   &calls,&calls,1,calls.expectedVector20,1,calls);
 check(second->flag1c==0&&second->alternative30==1);
 check(second->controller20 &&
       *second->controller20==calls.expectedVector20);
 check(calls.calls==std::vector<std::string>({"57bcb8","57bcb8"}));
 calls.calls.clear();
 // Original 0x57BA98 leaves +0x20 unmodified rather than writing
 // synthetic zeros over an already owned controller vector.
 OriginalManagerChild70 reused{};
 reused.controller20=calls.expectedVector20;
 reused.controller40=calls.expectedVector20;
 calls.expectedCtor=&reused;
 calls.expectPreservedVariantZero=true;
 originalManagerChildCtor_57ba98(reused,&calls,&calls,3,4,calls);
 check(reused.controller20 &&
       *reused.controller20==calls.expectedVector20 &&
       reused.controller40 &&
       *reused.controller40==calls.expectedVector20 &&
       calls.calls==std::vector<std::string>({"57bcb8"}));
}
