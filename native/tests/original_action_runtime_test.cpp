#include "ae3/original_action_runtime.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void require(bool b){if(!b)throw std::runtime_error("Original retail action runtime diverged");}
struct Calls final:OriginalActionRuntimeCalls {
  std::vector<std::string> calls;
  bool firstPresent=false,secondPresent=false,giveCandidate=true,acceptCandidate=true;
  void* call_factory_virtual2(void* factory,OriginalManagerSlot& slot)override{
    require(factory==this);calls.emplace_back("factory_v2");
    if(slot.key00==1)return firstPresent?this:nullptr;
    if(slot.key00==2)return secondPresent?this:nullptr;
    return giveCandidate?this:nullptr;
  }
  bool call_candidate_virtual11(void* p,OriginalManagerSlot&)override{
    require(p==this);calls.emplace_back("candidate_v11");return acceptCandidate;
  }
  void* call_21d898()override{calls.emplace_back("21d898");return this;}
  void call_5638f4(void* parent,std::uint32_t arg,std::uint32_t value)override{
    require(parent==this&&arg==7&&value==41);calls.emplace_back("5638f4");
  }
  void call_165220(void* p)override{require(p==this);calls.emplace_back("165220");}
  void call_26b310(void* p,float value)override{require(p==this&&value==5.f);calls.emplace_back("26b310");}
  void call_26b524(void* p,std::uint32_t value)override{require(p==this&&value==19);calls.emplace_back("26b524");}
  void call_359e48(void* p)override{require(p==this);calls.emplace_back("359e48");}
};
int main(){
  OriginalActionFields a{};OriginalActionChildFields child{};OriginalActionManagers managers{};Calls c;
  require(originalFactoryGet_221644(managers.factory_647bd0)==nullptr);
  originalFactorySet_221650(managers.factory_647bd0,&c);
  require(originalFactoryGet_221644(managers.factory_647bd0)==&c);
  require(originalFactoryIs_221668(managers.factory_647bd0,&c));
  require(!originalFactoryIs_221668(managers.factory_647bd0,nullptr));
  child.status45=0xf5;child.value48=0xaabbccdd;
  require(originalActionReadStatus_3168ac(child)==0xf5);
  require(originalActionReadValue_3168b8(child)==0xaabbccdd);
  originalActionWriteWord_3168c4(child,0x98765432);
  require(child.word0c==0x98765432);
  // Original readiness short-circuits BEFORE initializing managers.
  require(!originalActionReady_3102f0(a,child,managers,c)&&c.calls.empty());
  a.flags0c=1;a.word40=1;
  require(!originalActionReady_3102f0(a,child,managers,c)&&c.calls.empty());
  a.word40=0;a.word34=12;
  require(!originalActionReady_3102f0(a,child,managers,c)&&c.calls.empty());
  a.word34=0;child.status45=0;
  require(!originalActionReady_3102f0(a,child,managers,c)&&c.calls.empty());
  child.status45=1;
  managers.at_696ca0.key00=1;managers.at_696cd0.key00=2;
  require(!originalActionReady_3102f0(a,child,managers,c));
  require(c.calls==std::vector<std::string>({"factory_v2","factory_v2"}));
  require(managers.at_696ca0.initialized05&&managers.at_696cd0.initialized05);
  c.calls.clear();c.secondPresent=true;
  // These are one-time original managers. No invented automatic reinitialization.
  require(!originalActionReady_3102f0(a,child,managers,c)&&c.calls.empty());
  managers.at_696cd0.initialized05=0;
  require(originalActionReady_3102f0(a,child,managers,c));
  require(c.calls==std::vector<std::string>({"factory_v2","candidate_v11","21d898"}));
  c.calls.clear();c.firstPresent=true;
  managers.at_696ca0.initialized05=0;
  require(originalActionManagerReady_316fd0(managers,c));
  require(c.calls==std::vector<std::string>({"factory_v2","candidate_v11","21d898"}));
  c.calls.clear();
  OriginalManagerSlot disabled{};disabled.disabled04=1;disabled.instanceAt08=&c;
  originalManagerInitialize_57ba00(disabled,managers.factory_647bd0,c);
  require(disabled.initialized05&&!disabled.instanceAt08&&c.calls.empty());
  disabled.instanceAt08=&c;
  originalManagerInitialize_57ba00(disabled,managers.factory_647bd0,c);
  require(disabled.instanceAt08==&c&&c.calls.empty());
  c.acceptCandidate=false;OriginalManagerSlot denied{};
  originalManagerInitialize_57ba00(denied,managers.factory_647bd0,c);
  require(!denied.instanceAt08&&denied.initialized05);
  require(c.calls==std::vector<std::string>({"factory_v2","candidate_v11"}));
  c.calls.clear();c.acceptCandidate=true;
  OriginalFactoryGlobal missing{};OriginalManagerSlot pending{};
  bool missingRaised=false;
  try{originalManagerInitialize_57ba00(pending,missing,c);}
  catch(const std::runtime_error&){missingRaised=true;}
  require(missingRaised);
  originalFactorySet_221650(managers.factory_647bd0,nullptr);
  require(originalFactoryGet_221644(managers.factory_647bd0)==nullptr);
  originalFactorySet_221650(managers.factory_647bd0,&c);
  require(!originalActionThreeFlags_310358(a));
  a.word34=1;a.word30=1;
  require(originalActionThreeFlags_310358(a));
  a.word40=1;require(!originalActionThreeFlags_310358(a));
  require(originalActionNestedWord_310380(0xdeadbeef)==0xdeadbeef);
  a.originalParent00=&c;a.parameter14=7;a.word38=41;c.calls.clear();
  originalActionSetValue_3103a8(a,41,c);require(c.calls.empty());
  a.word38=0;originalActionSetValue_3103a8(a,41,c);
  require(c.calls==std::vector<std::string>({"5638f4"})&&a.word38==41);
  c.calls.clear();originalActionForwardOwned_3103f0(&c,c);
  originalChannelGuardedForward_276274(true,&c,5.f,c);
  originalChannelGuardedForward_276274(false,&c,5.f,c);
  originalChannelIntegerForward_2762bc(&c,19,c);
  originalVectorForward_276580(&c,c);
  require(c.calls==std::vector<std::string>({"165220","26b310","26b524","359e48"}));
}
