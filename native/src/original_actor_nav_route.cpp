#include "ae3/original_actor_nav_route.hpp"
#include <stdexcept>

namespace ae3 {
namespace {
std::uint16_t u16(std::span<const std::byte> data,std::size_t pos){
    if(pos>data.size()||data.size()-pos<2)
        throw std::out_of_range("Original actor navigation halfword outside source file");
    return std::uint16_t(std::to_integer<unsigned>(data[pos])|
        (std::to_integer<unsigned>(data[pos+1])<<8));
}
std::uint32_t u32(std::span<const std::byte> data,std::size_t pos){
    return std::uint32_t(u16(data,pos))|(std::uint32_t(u16(data,pos+2))<<16);
}
struct NavigationView {
    std::span<const std::byte> bytes;
    std::size_t count;
    std::size_t descriptors;
    explicit NavigationView(std::span<const std::byte> data):bytes(data),
        count(u32(data,0x24)),descriptors(u32(data,0x3c)) {
        if(descriptors>bytes.size() || count>(bytes.size()-descriptors)/0x1e)
            throw std::out_of_range("Original actor navigation polygon descriptors truncated");
    }
    std::size_t record(std::uint32_t sourceIndex) const {
        if(sourceIndex>=count)throw std::out_of_range("Original actor route polygon index invalid");
        return descriptors+std::size_t(sourceIndex)*0x1e;
    }
};
} // namespace
std::uint32_t originalActorRouteEdgeSlot_221104(
    std::span<const std::uint32_t> sourceRoute,std::size_t step,
    std::span<const std::byte> sourceNav) {
    if(step>=sourceRoute.size()||sourceRoute.size()-step<2)
        throw std::out_of_range("Original route requires consecutive source polygon entries");
    const NavigationView src(sourceNav);
    const auto at=src.record(sourceRoute[step]);
    (void)src.record(sourceRoute[step+1]);
    const auto degree=u16(sourceNav,at+2);
    if(degree>3)throw std::out_of_range("Original spatial polygon exceeds three edge slots");
    // 0x221140..0x221164: source iterates exactly THREE attempts. The
    // underlying 0x21B404 returns null after the recorded degree;
    // a valid non-null destination cannot match such an attempt.
    for(std::uint32_t slot=0;slot<3;++slot){
        if(slot<degree && u16(sourceNav,at+4+2*slot)==sourceRoute[step+1])
            return slot;
    }
    return 0; // actual source no-match behavior, not an error sentinel
}
std::uint16_t originalActorRouteEdgeKind_21b444(
    std::uint32_t polygon,std::uint32_t slot,std::span<const std::byte> sourceNav) {
    const NavigationView src(sourceNav);
    const auto at=src.record(polygon);
    if(slot>=3)throw std::out_of_range("Original spatial record has three action slots");
    return u16(sourceNav,at+0x0a+2*slot);
}
OriginalActorRouteEdgeDecision_57bd88 originalActorRouteEdgeDecision_57bd88(
    std::span<const std::uint32_t> sourceRoute,std::size_t segmentIndex,
    std::span<const std::byte> sourceNav){
    const auto slot=originalActorRouteEdgeSlot_221104(sourceRoute,segmentIndex,sourceNav);
    const auto src=sourceRoute[segmentIndex];
    const auto kind=originalActorRouteEdgeKind_21b444(src,slot,sourceNav);
    return {src,sourceRoute[segmentIndex+1],slot,kind,kind==2};
}
void originalActorReplaceRoute_575ec0(void*& stored,void* incoming,
    OriginalActorRouteLifetimeCalls_575ec0& lifetime) {
    if(stored==incoming)return;
    if(stored){
        lifetime.call_original_220a78(stored);
        lifetime.call_original_41a3b0(stored);
    }
    stored=incoming;
}
} // namespace ae3
