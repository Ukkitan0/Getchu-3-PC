#include "ae3/original_attachment_followup.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool p){if(!p)throw std::runtime_error("original executable method translation mismatch");}
struct Direct:OriginalAttachmentDirectCalls{
 std::vector<std::string> calls;bool ok=false;
 bool call_313a88(void* p)override{check(p==reinterpret_cast<void*>(0x30));calls.push_back("313a88");return ok;}
 void call_3102d4(void* p)override{check(p==reinterpret_cast<void*>(0x40));calls.push_back("3102d4");}
 void call_27679c(void* a,void* b)override{check(a==reinterpret_cast<void*>(0x50)&&b==reinterpret_cast<void*>(0x60));calls.push_back("27679c");}
};
struct Indirect:OriginalAttachmentIndirectCalls{
 std::vector<int> called;
 std::uint32_t call_child_virtual_2(void* p)override{check(p==reinterpret_cast<void*>(0x70));called.push_back(2);return 42;}
 std::uint32_t call_child_virtual_4(void* p,std::uint32_t n)override{check(p==reinterpret_cast<void*>(0x70)&&n==99);called.push_back(4);return 123;}
 void call_child_virtual_3(void* p)override{check(p==reinterpret_cast<void*>(0x70));called.push_back(3);}
};
struct Release:OriginalAttachmentReleaser{
 std::vector<std::string> called;
 void mark_child_record_a1(void* p)override{check(p==reinterpret_cast<void*>(0x80));called.push_back("mark");}
 void call_41a3b0(void* p)override{check(p==reinterpret_cast<void*>(0x80));called.push_back("free");}
};
int main(){
 OriginalAttachedControlFields s{};s.word_40=88;
 check(originalAttachmentSetWord40_56b3b4(s,123,456)==0&&s.word_40==88);
 s.flag_3d=true;
 check(originalAttachmentSetWord40_56b3b4(s,123,456)==2&&s.word_40==123);
 check(originalAttachmentSetWord40_56b3b4(s,0,456)==2&&s.word_40==456);
 check(originalAttachmentReturnThirdArg_56b3e4(31)==31);
 originalAttachmentNoop_56b3ec();
 check(originalSecondAttachmentZero_56ba98()==0);
 check(originalSecondAttachmentReturnThirdArg_56baa0(0xffffffffu)==0xffffffffu);
 originalSecondAttachmentNoop_56baa8();
 Direct d;
 check(originalAttachmentQuery_56b9a8(reinterpret_cast<void*>(0x30),d)==0);
 d.ok=true;check(originalAttachmentQuery_56b9a8(reinterpret_cast<void*>(0x30),d)==2);
 check(d.calls==std::vector<std::string>({"313a88","313a88"}));d.calls.clear();
 check(originalAttachmentFinishPhase_56b28c(reinterpret_cast<void*>(0x40),0,d)==2&&d.calls.empty());
 check(originalAttachmentFinishPhase_56b28c(reinterpret_cast<void*>(0x40),1,d)==2);
 check(d.calls==std::vector<std::string>({"3102d4"}));d.calls.clear();
 check(originalAttachmentUsesInitialPhase_56af44(0.f));
 check(originalAttachmentUsesInitialPhase_56af44(-0.f));
 check(!originalAttachmentUsesInitialPhase_56af44(0.1f));
 check(!originalAttachmentUsesInitialPhase_56af44(std::numeric_limits<float>::quiet_NaN()));
 check(originalAttachmentModeVectorDataVA_56b000(1)==0x707110);
 check(originalAttachmentModeVectorDataVA_56b000(2)==0x707130);
 check(originalAttachmentModeVectorDataVA_56b000(3)==0x707150);
 check(originalAttachmentModeVectorDataVA_56b000(0)==0x707170);
 check(originalAttachmentModeVectorDataVA_56b000(0xffffffffu)==0x707170);
 using Phase=OriginalAttachmentTimerPhase;
 auto t=originalAttachmentAdvanceTime_56b07c(0.1f,0.1f);
 check(t.time54==0.2f&&t.phase==Phase::below_0_3);
 check(originalAttachmentAdvanceTime_56b07c(0.3f,0).phase==Phase::below_0_9);
 check(originalAttachmentAdvanceTime_56b07c(0.9f,0).phase==Phase::below_1_4);
 check(originalAttachmentAdvanceTime_56b07c(1.4f,0).phase==Phase::at_or_above_1_4);
 check(originalAttachmentAdvanceTime_56b07c(0,std::numeric_limits<float>::quiet_NaN()).phase==Phase::at_or_above_1_4);
 for(std::uint32_t n=0;n<16;n++)check(originalSecondAttachmentHasFastPath_56b9d0(n)==((n&3u)!=0));
 OriginalSecondAttachmentQuickFields q{true};
 check(originalSecondAttachmentFastPath_56ba00(q,reinterpret_cast<void*>(0x50),reinterpret_cast<void*>(0x60),d)==1);
 check(!q.flag_3c&&d.calls==std::vector<std::string>({"27679c"}));d.calls.clear();
 check(originalSecondAttachmentFastPath_56ba00(q,reinterpret_cast<void*>(0x50),reinterpret_cast<void*>(0x60),d)==1&&d.calls.empty());
 Indirect v;
 check(originalAttachmentForwardVirtual2_56bfe8(nullptr,v)==0);
 check(originalAttachmentForwardVirtual2_56bfe8(reinterpret_cast<void*>(0x70),v)==42);
 check(originalAttachmentForwardVirtual4_56c020(nullptr,99,v)==99);
 check(originalAttachmentForwardVirtual4_56c020(reinterpret_cast<void*>(0x70),99,v)==123);
 originalAttachmentForwardVirtual3_56c054(nullptr,v);
 originalAttachmentForwardVirtual3_56c054(reinterpret_cast<void*>(0x70),v);
 check(v.called==std::vector<int>({2,4,3}));
 Release release;void* child=nullptr;
 originalAttachmentReleaseAndClear_315db0(child,release);
 check(!child&&release.called.empty());
 child=reinterpret_cast<void*>(0x80);
 originalAttachmentReleaseAndClear_315db0(child,release);
 check(!child&&release.called==std::vector<std::string>({"mark","free"}));
}
