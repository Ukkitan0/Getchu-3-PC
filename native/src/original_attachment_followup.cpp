#include "ae3/original_attachment_followup.hpp"
namespace ae3 {
std::uint32_t originalAttachmentSetWord40_56b3b4(OriginalAttachedControlFields& s,std::uint32_t arg,std::uint32_t fallback) noexcept {
 if(!s.flag_3d)return 0;
 s.word_40=arg?arg:fallback;
 return 2;
}
std::uint32_t originalAttachmentReturnThirdArg_56b3e4(std::uint32_t a2) noexcept{return a2;}
void originalAttachmentNoop_56b3ec() noexcept{}
std::uint32_t originalSecondAttachmentZero_56ba98() noexcept{return 0;}
std::uint32_t originalSecondAttachmentReturnThirdArg_56baa0(std::uint32_t a2) noexcept{return a2;}
void originalSecondAttachmentNoop_56baa8() noexcept{}
std::uint32_t originalAttachmentQuery_56b9a8(void* p,OriginalAttachmentDirectCalls& b){
 return b.call_313a88(p)?2u:0u;
}
std::uint32_t originalAttachmentFinishPhase_56b28c(void* owner,std::uint32_t word34,OriginalAttachmentDirectCalls& b){
 if(word34)b.call_3102d4(owner);
 return 2;
}
bool originalAttachmentUsesInitialPhase_56af44(float timer54) noexcept {
 // Original c.eq.s against f20=0.0f; NaN follows nonzero-timer branch.
 return timer54==0.0f;
}
std::uint32_t originalAttachmentModeVectorDataVA_56b000(std::uint32_t mode) noexcept {
 switch(mode){
 case 1:return 0x707110;
 case 2:return 0x707130;
 case 3:return 0x707150;
 default:return 0x707170;
 }
}
OriginalAttachmentTimerResult originalAttachmentAdvanceTime_56b07c(float time54,float delta) noexcept {
 // Original 0x56B07C..0x56B09C adds runtime global 0x635488,
 // ALWAYS stores +0x54, and branches at 0.3, 0.9 and 1.4.
 const float time=time54+delta;
 OriginalAttachmentTimerPhase phase;
 if(time<0.3f)phase=OriginalAttachmentTimerPhase::below_0_3;
 else if(time<0.9f)phase=OriginalAttachmentTimerPhase::below_0_9;
 else if(time<1.4f)phase=OriginalAttachmentTimerPhase::below_1_4;
 else phase=OriginalAttachmentTimerPhase::at_or_above_1_4;
 return {time,phase};
}
bool originalSecondAttachmentHasFastPath_56b9d0(std::uint32_t flags) noexcept {
 return (flags&3u)!=0;
}
std::uint32_t originalSecondAttachmentFastPath_56ba00(OriginalSecondAttachmentQuickFields& s,void* child,void* paramPlus80,OriginalAttachmentDirectCalls& b){
 if(s.flag_3c){b.call_27679c(child,paramPlus80);s.flag_3c=false;}
 return 1;
}
std::uint32_t originalAttachmentForwardVirtual2_56bfe8(void* child,OriginalAttachmentIndirectCalls& b){
 return child?b.call_child_virtual_2(child):0;
}
std::uint32_t originalAttachmentForwardVirtual4_56c020(void* child,std::uint32_t arg,OriginalAttachmentIndirectCalls& b){
 return child?b.call_child_virtual_4(child,arg):arg;
}
void originalAttachmentForwardVirtual3_56c054(void* child,OriginalAttachmentIndirectCalls& b){
 if(child)b.call_child_virtual_3(child);
}
void originalAttachmentReleaseAndClear_315db0(void*& child,OriginalAttachmentReleaser& b){
 if(child){b.mark_child_record_a1(child);b.call_41a3b0(child);}
 child=nullptr;
}
} // namespace ae3
