#include "ae3/original_manager_initializer.hpp"
#include "ae3/original_actor_ray_geometry.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace ae3 {
OriginalManagerInitBranch_57bcb8 originalManagerInitialize_57bcb8(
    OriginalManagerInitState_57bcb8& state,const OriginalManagerInitSource_57bcb8& source,
    OriginalGameRandom_36f3c0& random,OriginalManagerInitCalls_57bcb8& calls) {
    auto& child=state.lifecycle;
    // 0x57BCDC..0x57BD34: alternative constructor only. The source
    // subtracts controller+20 - parent+14->40 in all four lanes; length
    // uses the already recovered 0x4748AC XYZ dot, not the W lane.
    if(child.alternative30) {
        const OriginalVU0Vec4 delta{
            state.controller20.x-source.ownerPosition40.x,
            state.controller20.y-source.ownerPosition40.y,
            state.controller20.z-source.ownerPosition40.z,
            state.controller20.w-source.ownerPosition40.w};
        const float length=std::sqrt(originalVU0DotXYZ_4748ac(delta,delta));
        if(length<2.0f) { // original 0x708A20, ordered C.OLT.S
            // Original 0x57BD04 copies delta to $a1; 0x4748AC does not
            // clobber $a1. 0x57BD3C..54 stores its LOW 64 bits at +8/+C.
            // Preserve this observed write rather than assuming zeros.
            child.word08=std::bit_cast<std::uint32_t>(delta.x);
            child.word0c=std::bit_cast<std::uint32_t>(delta.y);
            return OriginalManagerInitBranch_57bcb8::nearOwnerReturn;
        }
    }
    // 0x57BD80 / 0x57BF54: a single-polygon controller bypasses the
    // route-kind query and enters the general projection path directly.
    if(child.flag1c) {
        const auto slot=originalActorRouteEdgeSlot_221104(
            source.routePolygonIds,0,source.originalNavigation);
        std::size_t actorIndex=source.stageRecords.size();
        for(std::size_t i=0;i<source.stageRecords.size();++i)
            if(&source.stageRecords[i]==source.actorPolygon30){actorIndex=i;break;}
        if(actorIndex==source.stageRecords.size())
            throw std::invalid_argument("Original 0x57BD94 actor+0x30 does not belong to the decoded stage registry");
        // Source 0x57BD90 first gets a slot from context route, then
        // 0x57BD9C queries CURRENT ACTOR polygon, not blindly route[0].
        const auto kind=originalActorRouteEdgeKind_21b444(
            static_cast<std::uint32_t>(actorIndex),slot,source.originalNavigation);
        if(kind==2) {
            const auto target=originalActorType2Target_57bdb8(
                source.stageRecords,source.routePolygonIds);
            state.target50=target.targetPolygonAt50; // 0x57BDC8, before RNG
            std::array<OriginalVU0Vec4,3> vertices{};
            for(std::size_t i=0;i<3;++i)
                std::memcpy(&vertices[i],target.rawOriginalVertices[i].data(),16);
            // 0x57BE00..0x57BECC has the SAME two RNG calls, source
            // constants and ordered VMUL/VADD sequence as recovered
            // 0x57C98C. Reuse that native math; do not decompile it twice.
            const auto point=originalActorRandomTriangle_57c98c(vertices,random);
            // Source 0x708A10 holds this method-pair metadata (VA,0).
            child.word08=0x57c554;child.word0c=0;
            state.target40=point; // source delay slot 0x57BED4 SQC2
            return OriginalManagerInitBranch_57bcb8::typeTwoTarget;
        }
    }
    if(!source.actorPolygon30)
        throw std::invalid_argument("Original 0x57BEE4 actor polygon+0x30 unavailable");
    // 0x57BED8..0x57BF04: direction*25 followed by controller+20.
    const OriginalVU0Vec4 end{
        state.controller20.x+source.ownerDirection50.x*25.0f,
        state.controller20.y+source.ownerDirection50.y*25.0f,
        state.controller20.z+source.ownerDirection50.z*25.0f,
        state.controller20.w+source.ownerDirection50.w*25.0f};
    const auto projected=calls.call_original_21b854(
        *source.actorPolygon30,state.controller20,end);
    // 0x57BF18 VADD, 0x57BF20 VMULx(0.5): a MIDPOINT, not a
    // displacement. Preserve all four source lanes and operation order.
    state.midpoint60={
        (state.controller20.x+projected.x)*.5f,
        (state.controller20.y+projected.y)*.5f,
        (state.controller20.z+projected.z)*.5f,
        (state.controller20.w+projected.w)*.5f};
    const auto originalIncomingA1=state.midpoint60;
    calls.call_original_57c7cc(state,originalIncomingA1);
    // 0x57BF30..0x57BF50 / source data 0x708A18: only publish the
    // new update method AFTER the original route callback returns.
    child.word08=0x57bf5c;child.word0c=0;
    return OriginalManagerInitBranch_57bcb8::generalProjection;
}
} // namespace ae3
