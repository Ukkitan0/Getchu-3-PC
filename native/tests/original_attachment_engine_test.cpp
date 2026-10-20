#include "ae3/original_attachment_engine.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool value){if(!value)throw std::runtime_error("original attachment operation mismatch");}
struct Counter:OriginalNestedCallCounter {
 int level=0;
 void beginVirtualCall()override{++level;}
 void endVirtualCall()override{--level;}
};
struct Child:OriginalChildCallBackend {
 std::vector<std::string> log;
 Counter* counter=nullptr;
 std::uint32_t result=0,flags=0;
 std::uint32_t callVirtual5(void* p,std::uint32_t n)override{
  check(p==(void*)0x17);check(counter&&counter->level==1);
  log.push_back("v5");flags=n;return result;
 }
 void callVirtual1Delete(void* p)override{
  check(p==(void*)0x17);log.push_back("delete");
 }
 void callVirtual3(void* p)override{
  check(p==(void*)0x17);log.push_back("v3");
 }
};
struct Release:OriginalChildReleaseBackend {
 int frees=0;std::uint8_t flag=0;
 std::uint8_t& flagA1OfChildRecord(void* p)override{
  check(p==(void*)0x17);return flag;
 }
 void freeOriginalChild(void* p)override{
  check(p==(void*)0x17);++frees;
 }
};
int main(){
 std::array<std::uint8_t,64> bytes{};
 for(unsigned i=0;i<bytes.size();++i)bytes[i]=std::uint8_t(i^0x6a);
 auto a=originalAttachmentCtor_312760((void*)0x11,0x12345678,bytes);
 check(a.owner==(void*)0x11&&a.parameter50==0x12345678&&a.field54==0&&a.payload==bytes);
 std::array<std::uint8_t,8> known{};
 for(unsigned i=0;i<known.size();++i)known[i]=std::uint8_t(i+1);
 auto b=originalAttachmentCtor_3127d8((void*)0x12,(void*)0x13,known);
 check(b.owner==(void*)0x12&&b.parameter10==(void*)0x13&&b.value08==known&&b.flag3c==0&&b.flag3d==0&&b.field14==0&&b.field44==0);
 auto c=originalAttachmentCtor_312854((void*)0x14,(void*)0x15);
 check(c.owner==(void*)0x14&&c.parameter08==(void*)0x15&&c.field30==0&&c.field34==0);
 auto d=originalActorControllerCtor_571e44((void*)0x16,(void*)0x17,known);
 check(d.owner==(void*)0x16&&d.parameter18==(void*)0x17&&d.value08==known&&d.field10==0&&d.field14==0&&d.field20==0);
 check(originalSmallChildVirtual2_56af0c()==0&&originalSmallChildVirtual4_56af14()==0);
 originalSmallChildVirtual3_56af1c();check(originalSmallChildVirtual5_56af24()==0);
 check(originalParamChildVirtual2_56af2c()==0&&originalParamChildVirtual4_56af34()==0);
 originalParamChildVirtual3_56af3c();originalNoop_315bac();
 check(originalControllerVirtual2_5725d0()==3&&originalControllerVirtual2_572800()==3);
 check(originalControllerVirtual4_5725d8(0xabcd)==0xabcd&&originalControllerVirtual4_572808(0xffffffffu)==0xffffffffu);
 Child child;Counter counter;child.counter=&counter;
 originalControllerForwardVirtual3_5725e0(nullptr,child);
 originalControllerForwardVirtual3_572810((void*)0x17,child);
 originalControllerReleaseChild_572524((void*)0x17,child);
 check(child.log==std::vector<std::string>({"v3","delete"}));child.log.clear();
 OriginalAttachmentCallContext context{(void*)0x17,0};
 // Original 0x6B1240 vtable[5] -> 0x56AF24 returns zero.
 child.result=originalSmallChildVirtual5_56af24();
 check(originalAttachmentProcess_3161b8(context,0x80,true,true,counter,child)==0);
 check(context.child&&context.lastResult==0&&child.flags==0x83&&counter.level==0);
 child.result=7;child.log.clear();
 check(originalAttachmentProcess_3161b8(context,0x80,false,true,counter,child)==1);
 check(!context.child&&context.lastResult==7&&child.flags==0x82&&counter.level==0);
 check(child.log==std::vector<std::string>({"v5","delete"}));
 bool rejected=false;
 try{originalAttachmentProcess_3161b8(context,0,false,false,counter,child);}
 catch(const std::runtime_error&){rejected=true;}
 check(rejected);
 Release release;originalReleaseChild_315c08(nullptr,release);check(release.frees==0);
 originalReleaseChild_315c08((void*)0x17,release);check(release.frees==1&&release.flag==1);
 release.flag=0;originalReleaseChild_315c40((void*)0x17,release);check(release.frees==2&&release.flag==1);
}
