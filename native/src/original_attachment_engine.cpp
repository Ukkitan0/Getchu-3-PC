#include "ae3/original_attachment_engine.hpp"
#include <stdexcept>
namespace ae3 {
OriginalAttachment60 originalAttachmentCtor_312760(void* owner,std::uint32_t key,const std::array<std::uint8_t,0x40>& src) noexcept {
 // 0x312780 alloc 0x60; 4 original LQ/SQ transfers a2+0x00..0x3F to
 // output+0x10..0x4F; store a1 at +0x50; clear +0x54.
 return {owner,src,key,0};
}
OriginalAttachment48 originalAttachmentCtor_3127d8(void* owner,void* parameter,const std::array<std::uint8_t,8>& sourceConstant) noexcept {
 // Original 0x3127F0 alloc 0x48; source constant comes from retail 0x707200.
 OriginalAttachment48 out{};out.owner=owner;out.parameter10=parameter;out.value08=sourceConstant;return out;
}
OriginalAttachment40 originalAttachmentCtor_312854(void* owner,void* parameter) noexcept {
 return {owner,parameter,0,0};
}
OriginalActorController24 originalActorControllerCtor_571e44(void* owner,void* parameter,const std::array<std::uint8_t,8>& sourceConstant) noexcept {
 // 0x571E44: original constant 0x707F10, owner+4, parameter+0x18,
 // and clear fields at +0x10, +0x14, +0x20.
 OriginalActorController24 out{};out.owner=owner;out.parameter18=parameter;out.value08=sourceConstant;return out;
}
std::uint32_t originalSmallChildVirtual2_56af0c() noexcept{return 0;}
std::uint32_t originalSmallChildVirtual4_56af14() noexcept{return 0;}
void originalSmallChildVirtual3_56af1c() noexcept{}
std::uint32_t originalSmallChildVirtual5_56af24() noexcept{return 0;}
std::uint32_t originalParamChildVirtual2_56af2c() noexcept{return 0;}
std::uint32_t originalParamChildVirtual4_56af34() noexcept{return 0;}
void originalParamChildVirtual3_56af3c() noexcept{}
void originalNoop_315bac() noexcept{}
std::uint32_t originalControllerVirtual2_5725d0() noexcept{return 3;}
std::uint32_t originalControllerVirtual4_5725d8(std::uint32_t a2) noexcept{return a2;}
std::uint32_t originalControllerVirtual2_572800() noexcept{return 3;}
std::uint32_t originalControllerVirtual4_572808(std::uint32_t a2) noexcept{return a2;}
void originalControllerForwardVirtual3_5725e0(void* child,OriginalChildCallBackend& calls){
 if(child)calls.callVirtual3(child);
}
void originalControllerForwardVirtual3_572810(void* child,OriginalChildCallBackend& calls){
 if(child)calls.callVirtual3(child);
}
void originalControllerReleaseChild_572524(void* child,OriginalChildCallBackend& calls){
 // Original destructor also resets its own vtable, which the native host
 // object's lifetime is responsible for; no fake parent allocator here.
 if(child)calls.callVirtual1Delete(child);
}
std::uint32_t originalAttachmentProcess_3161b8(OriginalAttachmentCallContext& context,std::uint32_t flags,bool bit0,bool bit1,OriginalNestedCallCounter& counter,OriginalChildCallBackend& calls){
 const auto originalFlags=flags|(bit0?1u:0u)|(bit1?2u:0u);
 if(!context.child)throw std::runtime_error("Original 0x3161B8 requires a genuine attachment");
 counter.beginVirtualCall();
 std::uint32_t result;
 try{result=calls.callVirtual5(context.child,originalFlags);}
 catch(...){counter.endVirtualCall();throw;}
 counter.endVirtualCall();
 context.lastResult=result;
 if(result){
   if(context.child)calls.callVirtual1Delete(context.child);
   context.child=nullptr;
   return 1;
 }
 return 0;
}
void originalReleaseChild_315c08(void* child,OriginalChildReleaseBackend& backend){
 if(!child)return;
 backend.flagA1OfChildRecord(child)=1;
 backend.freeOriginalChild(child);
}
void originalReleaseChild_315c40(void* child,OriginalChildReleaseBackend& backend){
 originalReleaseChild_315c08(child,backend);
}
} // namespace ae3
