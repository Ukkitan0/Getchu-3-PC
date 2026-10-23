#include "ae3/original_actor_polygon_selector_30ffd0.hpp"

namespace ae3 {
const OriginalSpatialRecord70* originalActorPolygonSelect_30ffd0(
    const OriginalSpatialRecord70*& current30,
    const OriginalSpatialRecord70* incoming,
    OriginalActorPolygonSelectorCalls_30ffd0& calls) {
    // 0x30FFF4 branches on incoming source polygon (original a1).
    // 0x706304 is the EXACT original .1f eligibility margin.
    constexpr float sourceMargin=0.1f;
    if (incoming) {
        // 0x31005C..0x310088: prefer original incoming polygon when
        // 0x21B788 accepts it, then store it to actor+0x30.
        if (calls.call_original_21b788(*incoming,sourceMargin)) {
            current30=incoming;
            return current30;
        }
        // 0x31008C: when incoming is rejected AND actor+0x30 is
        // null, source returns null WITHOUT calling nearest.
        if (!current30) return nullptr;
    }

    // 0x30FFFC..0x310020 / 0x31008C..98: preserve the old actor
    // polygon if the original 0x21B788 accepts it.
    if (current30 && calls.call_original_21b788(*current30,sourceMargin))
        return current30;

    // 0x310028..0x31003C: source 0x21D898 nearest lookup is made
    // with actor+0x28 registry/owner and source VU0 query. A null
    // nearest result DOES NOT clear the existing current polygon.
    const auto* replacement=calls.call_original_21d898();
    if (replacement)current30=replacement;
    return current30;
}
} // namespace ae3
