#pragma once
#include "ae3/original_spatial_index.hpp"
#include <cstdint>

namespace ae3 {
// Original 0x30FFD0 actor+0x30 polygon adoption / nearest fallback.
// Verified against the owner-private USA executable (entire 51-word
// source range 0x30FFD0..0x31009C). This is NOT a generic resource
// destructor: it reads/writes original owner polygon at +0x30 and
// delegates to genuine 0x21B788 eligibility and 0x21D898 nearest.
// The original candidate is the original 0x57C620 owner+0x50 input.
class OriginalActorPolygonSelectorCalls_30ffd0 {
public:
    virtual ~OriginalActorPolygonSelectorCalls_30ffd0()=default;
    // Original 0x706304 is source constant 0.1f (f12).
    // Actual actor query vector is from source owner+0x14.
    virtual bool call_original_21b788(
        const OriginalSpatialRecord70& sourcePolygon,
        float originalMarginF12)=0;
    // Original 0x21D898 queries registry at owner+0x28.
    virtual const OriginalSpatialRecord70* call_original_21d898()=0;
};

// Mutates source actor's current polygon+0x30 in the original order.
// Returns current polygon (which may remain nullptr). Genuine source
// pointer identities must belong to the caller-owned original stage.
// Any needed original predicate/movement operand remains explicit.
// Source pointer/array ownership is NEVER synthesized.
const OriginalSpatialRecord70* originalActorPolygonSelect_30ffd0(
    const OriginalSpatialRecord70*& originalCurrentPolygon30,
    const OriginalSpatialRecord70* originalIncomingPolygon,
    OriginalActorPolygonSelectorCalls_30ffd0& originalCalls);
} // namespace ae3
