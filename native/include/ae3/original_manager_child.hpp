#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
namespace ae3 {
// Original concrete controller vtable at 0x6B1820, allocated as 0x70
// guest bytes at 0x317538/0x31759C. Native C++ owns host pointers.
// Suffixes refer to observed ORIGINAL game field offsets.
struct OriginalManagerChild70 {
    static constexpr std::uint32_t original_vtable=0x6b1820;
    void* parent04=nullptr;
    std::uint32_t word08=0,word0c=0;
    void* child10=nullptr;
    std::uint32_t word14=0;
    // 0x57BAF8 writes all 128 bits of original a3 at +0x20.
    // 0x57BA98 does NOT initialize this field. Absence is not zero.
    std::optional<std::array<std::byte,16>> controller20;
    // The source +0x40 SQC2 is published by genuine 0x57BCB8
    // type-two target selection, or subsequently by the real actor
    // owner. Constructor 0x57BA98/0x57BAF8 does NOT establish this
    // value; missing must never be mistaken for an all-zero SQ.
    std::optional<std::array<std::byte,16>> controller40;
    void* context18=nullptr;
    std::uint8_t flag1c=0,flag1d=0;
    std::uint8_t alternative30=0;
    // Original 0x57C620 consumes the game-owned candidate polygon pointer at
    // controller+0x50. Neither 0x57BA98 nor 0x57BAF8 is verified to
    // write this field: preserve any owner-provided value at construction.
    void* originalPendingPolygon50=nullptr;
};
class OriginalManagerChildCalls {
public:
    virtual ~OriginalManagerChildCalls()=default;
    virtual void call_57bcb8(OriginalManagerChild70&)=0;
    virtual void call_child_virtual1_delete(void*)=0;
    virtual std::uint32_t call_child_virtual2(void*)=0;
    virtual std::uint32_t call_child_virtual4(void*,std::uint32_t,std::uint32_t)=0;
    virtual void call_child_virtual3(void*)=0;
    virtual void call_41a3b0(void*)=0;
};
// 0x57BCB8 is translated in original_manager_initializer.hpp. Bind this
// callback to genuine actor/stage ownership and that native initializer;
// its general projection/update dependencies still require real providers.
void originalManagerChildCtor_57ba98(OriginalManagerChild70&,
    void* parent,void* context,std::int32_t context_word04,
    std::uint8_t mode1d,OriginalManagerChildCalls&);
void originalManagerChildCtor_57baf8(OriginalManagerChild70&,
    void* parent,void* context,std::int32_t context_word04,
    const std::array<std::byte,16>& original_controller20,
    std::uint8_t mode1d,OriginalManagerChildCalls&);
std::unique_ptr<OriginalManagerChild70> originalManagerChildNew_317538(
    void* parent,void* context,std::int32_t context_word04,
    std::uint8_t mode1d,OriginalManagerChildCalls&);
std::unique_ptr<OriginalManagerChild70> originalManagerChildNew_31759c(
    void* parent,void* context,std::int32_t context_word04,
    const std::array<std::byte,16>& original_controller20,
    std::uint8_t mode1d,OriginalManagerChildCalls&);
void originalManagerChildDestroy_57bb60(OriginalManagerChild70&,OriginalManagerChildCalls&);
void originalManagerChildDeletingDestroy_57bbb4(OriginalManagerChild70&,OriginalManagerChildCalls&);
std::uint32_t originalManagerChildVirtual2_57bc0c(
    const OriginalManagerChild70&,std::uint32_t parent_word40,OriginalManagerChildCalls&);
std::uint32_t originalManagerChildVirtual4_57bc54(
    const OriginalManagerChild70&,std::uint32_t secondArg,std::uint32_t thirdArg,
    OriginalManagerChildCalls&);
void originalManagerChildVirtual3_57bc88(
    const OriginalManagerChild70&,OriginalManagerChildCalls&);
} // namespace ae3
