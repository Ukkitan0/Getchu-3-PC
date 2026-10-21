#pragma once
#include <cstdint>
namespace ae3 {
struct OriginalAttachedControlFields {
 bool flag_3d=false;
 std::uint32_t word_40=0;
};
std::uint32_t originalAttachmentSetWord40_56b3b4(OriginalAttachedControlFields&,std::uint32_t,std::uint32_t) noexcept;
std::uint32_t originalAttachmentReturnThirdArg_56b3e4(std::uint32_t) noexcept;
void originalAttachmentNoop_56b3ec() noexcept;
std::uint32_t originalSecondAttachmentZero_56ba98() noexcept;
std::uint32_t originalSecondAttachmentReturnThirdArg_56baa0(std::uint32_t) noexcept;
void originalSecondAttachmentNoop_56baa8() noexcept;
class OriginalAttachmentDirectCalls {
public:
 virtual ~OriginalAttachmentDirectCalls()=default;
 virtual bool call_313a88(void* originalObjectPlus14)=0;
 virtual void call_3102d4(void* originalOwner)=0;
 virtual void call_27679c(void* ownerChild,void* resolvedPointerPlus80)=0;
};
std::uint32_t originalAttachmentQuery_56b9a8(void*,OriginalAttachmentDirectCalls&);
std::uint32_t originalAttachmentFinishPhase_56b28c(void*,std::uint32_t,OriginalAttachmentDirectCalls&);
bool originalAttachmentUsesInitialPhase_56af44(float timer54) noexcept;
// Data addresses in the owner-owned original ELF, NEVER host pointers.
std::uint32_t originalAttachmentModeVectorDataVA_56b000(std::uint32_t mode) noexcept;
enum class OriginalAttachmentTimerPhase:std::uint8_t {
 below_0_3,below_0_9,below_1_4,at_or_above_1_4
};
struct OriginalAttachmentTimerResult {
 float time54;
 OriginalAttachmentTimerPhase phase;
};
OriginalAttachmentTimerResult originalAttachmentAdvanceTime_56b07c(float originalTime54,float originalFrameDelta635488) noexcept;
struct OriginalSecondAttachmentQuickFields {bool flag_3c=false;};
// Flags==0 fast-path is UNKNOWN because it enters original indirect dispatch.
bool originalSecondAttachmentHasFastPath_56b9d0(std::uint32_t flags) noexcept;
std::uint32_t originalSecondAttachmentFastPath_56ba00(OriginalSecondAttachmentQuickFields&,void*,void*,OriginalAttachmentDirectCalls&);
class OriginalAttachmentIndirectCalls {
public:
 virtual ~OriginalAttachmentIndirectCalls()=default;
 virtual std::uint32_t call_child_virtual_2(void*)=0;
 virtual std::uint32_t call_child_virtual_4(void*,std::uint32_t)=0;
 virtual void call_child_virtual_3(void*)=0;
};
std::uint32_t originalAttachmentForwardVirtual2_56bfe8(void*,OriginalAttachmentIndirectCalls&);
std::uint32_t originalAttachmentForwardVirtual4_56c020(void*,std::uint32_t,OriginalAttachmentIndirectCalls&);
void originalAttachmentForwardVirtual3_56c054(void*,OriginalAttachmentIndirectCalls&);
class OriginalAttachmentReleaser {
public:
 virtual ~OriginalAttachmentReleaser()=default;
 virtual void mark_child_record_a1(void*)=0;
 virtual void call_41a3b0(void*)=0;
};
void originalAttachmentReleaseAndClear_315db0(void*&,OriginalAttachmentReleaser&);
} // namespace ae3
