#pragma once
#include "ae3/original_render_instances.hpp"
#include "ae3/original_render_selector.hpp"
#include <cstdint>
#include <functional>
#include <vector>

namespace ae3 {
// Decoded original object fields from the USA I3D render owner, not guest
// memory. The integrator must derive these from authentic owner instances.
struct OriginalRetailDrawObject {
    OriginalRenderId id=0;
    std::uint64_t groupMask20=0;
    std::uint8_t flag30=0,flag32=0,flag33=0,flag34=0,flag35=0;
    const OriginalRenderSelector* selector=nullptr; // actor +0x110 handle
    bool hasResourceHandle=false;
};
// Unrecovered 0x26C9B8 / 0x26C4B0 and 0x26B3D0 resource decisions are
// delegated to source-backed providers. Never silently guess their return.
struct OriginalRetailDrawProviders {
    std::function<bool(OriginalRenderId)> rejectByOriginalBounds;
    std::function<bool(OriginalRenderId)> originalResourceEffectEnabled;
};
enum class OriginalRetailDrawOp {
    SelectResource,        // 0x2802D4
    BindResource,          // 0x3B76F8
    RestoreResource,       // 0x3B76E8
    DrawActor,             // 0x2803B4
    ApplyResourceEffect,   // 0x2804F4
    DrawActorMode,         // 0x26831C
};
// The original call's second/third arguments are NOT interchangeable:
// savedSlot=caller s5; savedState=caller s7; zero=null payload.
enum class OriginalRetailBindArg {none,savedSlot,savedState};
struct OriginalRetailDrawEvent {
    OriginalRenderId id=0;
    OriginalRetailDrawOp op=OriginalRetailDrawOp::DrawActor;
    std::uint32_t sourceVa=0;
    std::uint32_t selectorIndex=0;
    OriginalRetailBindArg argument=OriginalRetailBindArg::none;
    std::uint8_t mode=0; // actual 0x26831C a2 argument, 0 or 1
};
struct OriginalRetailPassTrace {
    std::vector<OriginalRetailDrawEvent> events;
    std::size_t considered=0,accepted=0;
    std::vector<OriginalRenderId> acceptedIds; // source traversal order
};
// Original first render scan: 0x280810-0x280958 (one original list at a
// time). Emits exact *calls and their order*, NOT synthesized GPU draws.
OriginalRetailPassTrace traceOriginalPrimaryRenderPass(
    const std::vector<OriginalRetailDrawObject>& originalList,
    std::uint64_t sceneMask58,
    const OriginalRetailDrawProviders& providers);
// Later genuine render scan: 0x280E20-0x280EB8. Important differences:
// original byte 0x32 is NOT checked, and source cull is unconditional once
// bytes 0x30 and 0x35 pass. Call follows different draw mode (0).
OriginalRetailPassTrace traceOriginalSecondaryRenderPass(
    const std::vector<OriginalRetailDrawObject>& originalList,
    const OriginalRetailDrawProviders& providers);
} // namespace ae3
