#include "ae3/original_manager_child.hpp"
#include <memory>
namespace ae3 {
namespace {
void originalInitialFields(OriginalManagerChild70& obj,void* parent,void* context,
                           std::int32_t at04,std::uint8_t mode,bool alternative,
                           OriginalManagerChildCalls& calls) {
    // 0x57BA98/0x57BAF8 original stores +0,+4,+8,+C,+10,+14,+18,
    // +1C,+1D,+30, then calls the real 0x57BCB8 initializer.
    obj.parent04=parent;
    obj.word08=0;obj.word0c=0;obj.child10=nullptr;obj.word14=0;
    obj.context18=context;
    // Original SLTI signed config[+4],2; XORI result with 1.
    obj.flag1c=at04>=2 ? 1 : 0;
    obj.flag1d=mode;
    obj.alternative30=alternative?1:0;
    calls.call_57bcb8(obj);
}
}
void originalManagerChildCtor_57ba98(OriginalManagerChild70& obj,void* parent,
    void* context,std::int32_t word04,std::uint8_t mode,OriginalManagerChildCalls& calls) {
    originalInitialFields(obj,parent,context,word04,mode,false,calls);
}
void originalManagerChildCtor_57baf8(OriginalManagerChild70& obj,void* parent,
    void* context,std::int32_t word04,std::uint8_t mode,OriginalManagerChildCalls& calls) {
    originalInitialFields(obj,parent,context,word04,mode,true,calls);
}
std::unique_ptr<OriginalManagerChild70> originalManagerChildNew_317538(
    void* parent,void* context,std::int32_t word04,std::uint8_t mode,OriginalManagerChildCalls& calls) {
    // Original allocation via 0x41B800(0x70); host uses native C++.
    auto obj=std::make_unique<OriginalManagerChild70>();
    originalManagerChildCtor_57ba98(*obj,parent,context,word04,mode,calls);
    return obj;
}
std::unique_ptr<OriginalManagerChild70> originalManagerChildNew_31759c(
    void* parent,void* context,std::int32_t word04,std::uint8_t mode,OriginalManagerChildCalls& calls) {
    auto obj=std::make_unique<OriginalManagerChild70>();
    originalManagerChildCtor_57baf8(*obj,parent,context,word04,mode,calls);
    return obj;
}
void originalManagerChildDestroy_57bb60(OriginalManagerChild70& obj,OriginalManagerChildCalls& calls) {
    if(obj.child10)calls.call_child_virtual1_delete(obj.child10);
    // Host object lifetime/vtable is native; original base-vtable write
    // has no guest-ABI equivalent and is not simulated.
}
void originalManagerChildDeletingDestroy_57bbb4(OriginalManagerChild70& obj,OriginalManagerChildCalls& calls) {
    originalManagerChildDestroy_57bb60(obj,calls);
    calls.call_41a3b0(&obj);
}
std::uint32_t originalManagerChildVirtual2_57bc0c(
    const OriginalManagerChild70& obj,std::uint32_t parent_word40,OriginalManagerChildCalls& calls) {
    if(parent_word40)return 3;
    return obj.child10?calls.call_child_virtual2(obj.child10):0;
}
std::uint32_t originalManagerChildVirtual4_57bc54(const OriginalManagerChild70& obj,
    std::uint32_t secondArg,std::uint32_t thirdArg,OriginalManagerChildCalls& calls) {
    return obj.child10?calls.call_child_virtual4(obj.child10,secondArg,thirdArg):thirdArg;
}
void originalManagerChildVirtual3_57bc88(const OriginalManagerChild70& obj,OriginalManagerChildCalls& calls) {
    if(obj.child10)calls.call_child_virtual3(obj.child10);
}
} // namespace ae3
