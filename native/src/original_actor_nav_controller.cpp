#include "ae3/original_actor_nav_controller.hpp"
#include <stdexcept>

namespace ae3 {
namespace {
OriginalActorRouteControllerPrepared_57ba98 prepare(
    const void* actor, const void* route, std::int32_t count,
    std::uint32_t flag, bool variant) {
    if(!route)throw std::invalid_argument("Original 0x57BA98/0x57BAF8 dereferences route+4");
    OriginalActorRouteControllerPrepared_57ba98 output{};
    output.originalActor=actor;
    output.originalRoute=route;
    // Retail SLTI signed < 2 followed by XORI 1 and SB +0x1C.
    output.routeHasAtLeastTwo=static_cast<std::uint8_t>(count>=2);
    output.originalRequestedFlag=static_cast<std::uint8_t>(flag);
    output.variantAt30=static_cast<std::uint8_t>(variant);
    return output;
}
}
OriginalActorRouteControllerPrepared_57ba98 originalActorControllerPrepare_57ba98(
    const void* actor,const void* route,std::int32_t count,std::uint32_t flag){
    return prepare(actor,route,count,flag,false);
}
OriginalActorRouteControllerPrepared_57ba98 originalActorControllerPrepare_57baf8(
    const void* actor,const void* route,std::int32_t count,
    const std::array<std::byte,16>& originalVector,std::uint32_t flag){
    auto output=prepare(actor,route,count,flag,true);
    output.vectorAt20=originalVector;
    return output;
}
std::int32_t originalActorControllerStatus_57bc0c(
    bool originalActorWord40Nonzero,void* child,OriginalActorControllerChildCalls& calls){
    if(originalActorWord40Nonzero)return 3;
    return child ? calls.call_virtual_08_status(child) : 0;
}
std::int32_t originalActorControllerForward_57bc54(
    void* child,std::uint32_t arg5,std::int32_t arg6,OriginalActorControllerChildCalls& calls){
    return child ? calls.call_virtual_10_forward(child,arg5,arg6) : arg6;
}
void originalActorControllerNotify_57bc88(
    void* child, OriginalActorControllerChildCalls& calls){
    if(child)calls.call_virtual_0c_notify(child);
}
void originalActorControllerDestroy_57bb60(
    void* child,OriginalActorControllerChildCalls& calls){
    if(child)calls.call_virtual_04_destroy(child);
}
void originalActorControllerDelete_57bbb4(
    void* object,void* child,OriginalActorControllerChildCalls& calls,
    OriginalActorControllerDeallocateCalls& allocator){
    originalActorControllerDestroy_57bb60(child,calls);
    allocator.call_original_41a3b0(object);
}
} // namespace ae3
