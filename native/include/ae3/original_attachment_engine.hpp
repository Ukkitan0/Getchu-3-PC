#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace ae3 {
// Values are native C++ structures holding fields recovered from the original
// ELF. No PS2 interpreter, guest memory ABI, or invented object behaviors.
struct OriginalAttachment60 {
 static constexpr std::uint32_t source_vtable=0x6B1220;
 void* owner=nullptr;
 std::array<std::uint8_t,0x40> payload{};
 std::uint32_t parameter50=0,field54=0;
};
struct OriginalAttachment48 {
 static constexpr std::uint32_t source_vtable=0x6B1280;
 void* owner=nullptr;
 std::array<std::uint8_t,8> value08{};
 void* parameter10=nullptr;
 std::uint32_t field14=0;
 std::uint8_t flag3c=0,flag3d=0;
 std::uint32_t field40=0,field44=0;
};
struct OriginalAttachment40 {
 static constexpr std::uint32_t source_vtable=0x6B1260;
 void* owner=nullptr;
 void* parameter08=nullptr;
 std::uint32_t field30=0,field34=0;
};
struct OriginalActorController24 {
 static constexpr std::uint32_t source_vtable=0x6B1500;
 void* owner=nullptr;
 std::array<std::uint8_t,8> value08{};
 void* parameter18=nullptr;
 std::uint32_t field10=0,field14=0,field20=0;
};
OriginalAttachment60 originalAttachmentCtor_312760(void*,std::uint32_t,const std::array<std::uint8_t,0x40>&) noexcept;
OriginalAttachment48 originalAttachmentCtor_3127d8(void*,void*,const std::array<std::uint8_t,8>&) noexcept;
OriginalAttachment40 originalAttachmentCtor_312854(void*,void*) noexcept;
OriginalActorController24 originalActorControllerCtor_571e44(void*,void*,const std::array<std::uint8_t,8>&) noexcept;
std::uint32_t originalSmallChildVirtual2_56af0c() noexcept;
std::uint32_t originalSmallChildVirtual4_56af14() noexcept;
void originalSmallChildVirtual3_56af1c() noexcept;
std::uint32_t originalSmallChildVirtual5_56af24() noexcept;
std::uint32_t originalParamChildVirtual2_56af2c() noexcept;
std::uint32_t originalParamChildVirtual4_56af34() noexcept;
void originalParamChildVirtual3_56af3c() noexcept;
void originalNoop_315bac() noexcept;
std::uint32_t originalControllerVirtual2_5725d0() noexcept;
std::uint32_t originalControllerVirtual4_5725d8(std::uint32_t) noexcept;
std::uint32_t originalControllerVirtual2_572800() noexcept;
std::uint32_t originalControllerVirtual4_572808(std::uint32_t) noexcept;
class OriginalChildCallBackend {
public:
 virtual ~OriginalChildCallBackend()=default;
 virtual std::uint32_t callVirtual5(void*,std::uint32_t)=0;
 virtual void callVirtual1Delete(void*)=0;
 virtual void callVirtual3(void*)=0;
};
void originalControllerForwardVirtual3_5725e0(void*,OriginalChildCallBackend&);
void originalControllerForwardVirtual3_572810(void*,OriginalChildCallBackend&);
void originalControllerReleaseChild_572524(void*,OriginalChildCallBackend&);
struct OriginalAttachmentCallContext {
 void* child=nullptr;
 std::uint32_t lastResult=0;
};
class OriginalNestedCallCounter {
public:
 virtual ~OriginalNestedCallCounter()=default;
 virtual void beginVirtualCall()=0;
 virtual void endVirtualCall()=0;
};
std::uint32_t originalAttachmentProcess_3161b8(OriginalAttachmentCallContext&,std::uint32_t,bool,bool,OriginalNestedCallCounter&,OriginalChildCallBackend&);
class OriginalChildReleaseBackend {
public:
 virtual ~OriginalChildReleaseBackend()=default;
 virtual std::uint8_t& flagA1OfChildRecord(void*)=0;
 virtual void freeOriginalChild(void*)=0;
};
void originalReleaseChild_315c08(void*,OriginalChildReleaseBackend&);
void originalReleaseChild_315c40(void*,OriginalChildReleaseBackend&);
} // namespace ae3
