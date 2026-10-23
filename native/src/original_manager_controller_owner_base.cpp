#include "ae3/original_manager_controller_owner_base.hpp"

namespace ae3 {
OriginalManagerControllerOwnerBase::OriginalManagerControllerOwnerBase(
    OriginalManagerChild70& controller,OriginalUpdateDepth_696c98& depth,
    OriginalManagerUpdateOwner_316174& owner) noexcept
    :tick_(controller,depth,owner) {}

std::uint32_t OriginalManagerControllerOwnerBase::call_original_3161b8(
    void*& child,std::uint32_t originalA1) {
    return tick_.call_original_3161b8(child,originalA1);
}
std::uint32_t OriginalManagerControllerOwnerBase::call_original_316174(
    OriginalManagerChild70& controller,std::uint32_t originalA1) {
    return tick_.call_original_316174(controller,originalA1);
}
void OriginalManagerControllerOwnerBase::
call_original_child_vtable1_delete(void* child) {
    tick_.call_original_child_vtable1_delete(child);
}
OriginalManagerNativeMethodOwner
OriginalManagerControllerOwnerBase::nativeMethodOwner(
    OriginalManagerDispatchCalls_57c620& owner) noexcept {
    return OriginalManagerNativeMethodOwner{
        *this,*this,*this,*this,owner,this};
}
} // namespace ae3
