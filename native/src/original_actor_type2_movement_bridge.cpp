#include "ae3/original_actor_type2_movement_bridge.hpp"
#include "ae3/original_actor_type2_target.hpp"
namespace ae3 {
OriginalActorType2BridgeResult_57bcb8 originalActorType2MovementBridge_57bcb8(
    bool variant30,bool hasMultiple1c,std::uint16_t sourceActionKind,
    const std::array<std::byte,16>& controller20,
    const std::array<std::byte,16>& owner14vector40,
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::uint32_t> route,
    OriginalGameRandom_36f3c0& sourceRng,
    std::uint32_t& field08,std::uint32_t& field0c,
    std::array<std::byte,16>& controller40,
    const OriginalSpatialRecord70*& target50) {
    // 0x57BCDC–0x57BD34: source-proven distance gate. A true
    // gate performs route-field clears at 0x57BD4C/54 and returns.
    const auto gate=originalActorDistanceGate_57bcdc(
        variant30,controller20,owner14vector40);
    if(gate.originalClearsControllerRoute08And0c) {
        originalActorApplyDistanceGate_57bd3c(gate,field08,field0c);
        return {OriginalActorType2BridgeOutcome_57bcb8::OriginalDistanceGateReset_57bd3c,
                gate,target50};
    }
    // 0x57BD80 checks controller+0x1C; 0x57BDA4/A8 checks
    // exact original action kind == 2. All other real branches
    // are outside this slice and MUST NOT acquire invented targets.
    if(!hasMultiple1c||sourceActionKind!=2)
        return {OriginalActorType2BridgeOutcome_57bcb8::OriginalOtherBranchUnrecovered,
                gate,target50};
    // 0x57BDB8–0x57BDF0 navigation-team original source route[1]
    // selection and its exact three 0x21B458 source vertex reads.
    const auto polygon=originalActorType2Target_57bdb8(records,route);
    // 0x57BDC8 writes controller+0x50 BEFORE the source VU0 operation.
    target50=polygon.targetPolygonAt50;
    // 0x57BE00..0x57BED4 authentic native VU0 + SQC2 write.
    originalActorType2VU0WriteController40_57bdcc(
        polygon.rawOriginalVertices,sourceRng,controller40);
    return {OriginalActorType2BridgeOutcome_57bcb8::OriginalType2TargetWritten_57bed4,
            gate,target50};
}
} // namespace ae3
