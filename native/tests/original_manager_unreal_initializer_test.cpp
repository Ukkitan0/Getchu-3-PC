#include "ae3/original_manager_initializer.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ae3;
namespace {
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
bool same(const OriginalVU0Vec4& x, const OriginalVU0Vec4& y) {
    return std::memcmp(&x, &y, sizeof(x)) == 0;
}
void write16(std::vector<std::byte>& d, std::size_t at, std::uint16_t value) {
    d.at(at) = std::byte(value & 255u);
    d.at(at+1) = std::byte(value >> 8);
}
void write32(std::vector<std::byte>& d, std::size_t at, std::uint32_t value) {
    for (unsigned i = 0; i != 4; ++i)
        d.at(at + i) = std::byte((value >> (8u*i)) & 255u);
}
struct OriginalCalls final : OriginalManagerInitCalls_57bcb8 {
    bool projectionAvailable = true;
    unsigned projections = 0;
    unsigned followed = 0;
    const OriginalSpatialRecord70* expectedPolygon = nullptr;
    OriginalVU0Vec4 expectedStart{}, expectedEnd{};
    OriginalVU0Vec4 originalProjection{30,40,50,60};
    OriginalVU0Vec4 call_original_21b854(
        const OriginalSpatialRecord70& polygon,
        const OriginalVU0Vec4& start,
        const OriginalVU0Vec4& end) override {
        ++projections;
        require(&polygon == expectedPolygon &&
                same(start, expectedStart) && same(end, expectedEnd),
                "Original 0x21B854 operands lost their source identity");
        if (!projectionAvailable)
            throw std::runtime_error("original source projection 0x21B854 missing");
        return originalProjection;
    }
    void call_original_57c7cc(
        OriginalManagerInitState_57bcb8& state,
        const OriginalVU0Vec4& incomingA1) override {
        ++followed;
        require(same(state.midpoint60, incomingA1),
                "Original incoming A1 midpoint changed at route callback");
        require(state.lifecycle.word08 == 0x1234 &&
                state.lifecycle.word0c == 0x5678,
                "Original method pointer written before route callback");
    }
};
void checkNearOwnerReturn() {
    OriginalManagerChild70 child{};
    child.alternative30 = 1;
    child.word08 = 0x1234;
    child.word0c = 0x5678;
    OriginalManagerInitState_57bcb8 state{child,{1,2,3,4},{7,8,9,10}};
    std::array<std::byte,16> originalSQ{};
    std::memcpy(originalSQ.data(),&state.controller20,sizeof(state.controller20));
    child.controller20=originalSQ;
    OriginalManagerInitSource_57bcb8 source{};
    source.ownerPosition40 = {1.5f,2,3,4};
    OriginalGameRandom_36f3c0 gameRng(5489);
    const auto before = gameRng.originalCursor();
    OriginalCalls calls{};
    const auto branch = originalManagerInitialize_57bcb8(
        state,source,gameRng,calls);
    require(branch == OriginalManagerInitBranch_57bcb8::nearOwnerReturn,
            "Original 0x57BCB8 near-owner branch was not taken");
    require(child.word08 == std::bit_cast<std::uint32_t>(-0.5f) &&
            child.word0c == std::bit_cast<std::uint32_t>(0.0f),
            "Original controller low64 near-owner delta changed");
    require(state.target40.x == 7 && state.target40.y == 8 &&
            calls.projections == 0 && calls.followed == 0 &&
            gameRng.originalCursor() == before,
            "Near-owner branch consumed stage resources or altered RNG");
}
void checkVariantOneRejectsMissingOrContradictorySQ() {
    OriginalManagerChild70 child{};
    child.alternative30=1;
    child.word08=0x1234;
    child.word0c=0x5678;
    OriginalManagerInitState_57bcb8 state{child,{1,2,3,4}};
    OriginalManagerInitSource_57bcb8 source{};
    source.ownerPosition40={1.5f,2,3,4};
    OriginalGameRandom_36f3c0 rng(5489);
    OriginalCalls calls{};
    const auto initialCursor=rng.originalCursor();
    auto mustReject=[&] {
        bool rejected=false;
        try {
            (void)originalManagerInitialize_57bcb8(state,source,rng,calls);
        } catch(const std::invalid_argument&) { rejected=true; }
        require(rejected && child.word08==0x1234 &&
                child.word0c==0x5678 && calls.projections==0 &&
                calls.followed==0 && rng.originalCursor()==initialCursor,
                "Missing or mismatched original SQ changed native actor/RNG state");
    };
    mustReject(); // Source 0x57BAF8 SQ not provided.
    std::array<std::byte,16> copied{};
    std::memcpy(copied.data(),&state.controller20,sizeof(state.controller20));
    copied[0]^=std::byte{0x01};
    child.controller20=copied;
    mustReject(); // Different vector cannot impersonate the constructor.
    copied[0]^=std::byte{0x01};
    child.controller20=copied;
    require(originalManagerInitialize_57bcb8(state,source,rng,calls)==
            OriginalManagerInitBranch_57bcb8::nearOwnerReturn,
            "Genuine source SQ must enter recovered original initializer");
}
// Calls the real recovered 0x57BAF8 constructor, which MUST invoke the
// real translated 0x57BCB8 initializer after installing the source SQ.
void checkSourceConstructorToInitializerChain() {
    struct ConstructorCalls final : OriginalManagerChildCalls {
        std::array<std::byte,16> sourceVector{};
        unsigned initializations=0;
        OriginalManagerInitBranch_57bcb8 originalBranch=
            OriginalManagerInitBranch_57bcb8::generalProjection;
        void call_57bcb8(OriginalManagerChild70& child) override {
            ++initializations;
            require(child.controller20 && *child.controller20==sourceVector &&
                    child.alternative30==1 && child.flag1c==0 &&
                    child.flag1d==0xF2, "Source constructor SQ or flags missing BEFORE native 0x57BCB8");
            OriginalManagerInitState_57bcb8 state{
                child,{1,2,3,4},{7,8,9,10}};
            OriginalManagerInitSource_57bcb8 source{};
            source.ownerPosition40={1.5f,2,3,4};
            OriginalGameRandom_36f3c0 rng(5489);
            OriginalCalls originalCalls{};
            const auto before=rng.originalCursor();
            originalBranch=originalManagerInitialize_57bcb8(
                state,source,rng,originalCalls);
            require(originalBranch==
                    OriginalManagerInitBranch_57bcb8::nearOwnerReturn &&
                    child.word08==std::bit_cast<uint32_t>(-0.5f) &&
                    child.word0c==0 &&
                    rng.originalCursor()==before &&
                    originalCalls.projections==0 &&
                    originalCalls.followed==0,
                    "Real original constructor-to-initializer near-owner chain differed");
        }
        void call_child_virtual1_delete(void*) override {
            throw std::runtime_error("unexpected original child destructor");
        }
        uint32_t call_child_virtual2(void*) override {
            throw std::runtime_error("unexpected original child status");
        }
        uint32_t call_child_virtual4(void*,uint32_t,uint32_t) override {
            throw std::runtime_error("unexpected original child forward");
        }
        void call_child_virtual3(void*) override {
            throw std::runtime_error("unexpected original child notify");
        }
        void call_41a3b0(void*) override {
            throw std::runtime_error("unexpected original allocator free");
        }
    } hook;
    const OriginalVU0Vec4 controller{1,2,3,4};
    std::memcpy(hook.sourceVector.data(),&controller,sizeof(controller));
    OriginalManagerChild70 child{};
    originalManagerChildCtor_57baf8(child,&hook,&hook,1,
                                    hook.sourceVector,0xF2,hook);
    require(hook.initializations==1 &&
            child.word08==std::bit_cast<uint32_t>(-0.5f) &&
            child.word0c==0 &&
            child.controller20 &&
            *child.controller20==hook.sourceVector,
            "Recovered 0x57BAF8 did not complete the exact native 0x57BCB8 chain");
}
void checkGeneralProjectionAndUnavailableSource() {
    OriginalSpatialRecord70 polygon{};
    std::array<OriginalSpatialRecord70,1> polygonOwner{polygon};
    OriginalManagerChild70 child{};
    child.word08 = 0x1234;
    child.word0c = 0x5678;
    OriginalManagerInitState_57bcb8 state{child,{1,2,3,4}};
    OriginalManagerInitSource_57bcb8 source{};
    source.actorPolygon30 = polygonOwner.data();
    source.stageRecords = polygonOwner;
    source.ownerDirection50 = {1,2,3,4};
    OriginalGameRandom_36f3c0 gameRng(5489);
    const auto before = gameRng.originalCursor();
    OriginalCalls calls{};
    calls.expectedPolygon = polygonOwner.data();
    calls.expectedStart = {1,2,3,4};
    calls.expectedEnd = {26,52,78,104};
    const auto branch = originalManagerInitialize_57bcb8(
        state,source,gameRng,calls);
    require(branch == OriginalManagerInitBranch_57bcb8::generalProjection,
            "Original 0x57BCB8 general projection branch changed");
    require(same(state.midpoint60,{15.5f,21,26.5f,32}) &&
            child.word08 == 0x57bf5c && child.word0c == 0 &&
            calls.projections == 1 && calls.followed == 1 &&
            gameRng.originalCursor() == before,
            "Original general branch VU0 midpoint/route ordering changed");
    child.word08 = 0x1234;
    child.word0c = 0x5678;
    calls.projectionAvailable = false;
    bool rejected = false;
    try {
        (void)originalManagerInitialize_57bcb8(
            state,source,gameRng,calls);
    } catch (const std::runtime_error& e) {
        rejected = std::string(e.what()).find("0x21B854") != std::string::npos;
    }
    require(rejected && child.word08 == 0x1234 && child.word0c == 0x5678 &&
            calls.followed == 1 && gameRng.originalCursor() == before,
            "Unrecovered original projection was bypassed/committed a method");
}
void checkRealOriginalTypeTwoRouteAndRng() {
    // Precisely the source 0x24/count, 0x3c/descriptor pointer and 0x1e
    // polygon header layout, with a single genuine kind-2 action.
    std::vector<std::byte> sourceNav(0x80+2*0x1e);
    write32(sourceNav,0x24,2);
    write32(sourceNav,0x3c,0x80);
    write16(sourceNav,0x80+2,1);
    write16(sourceNav,0x80+4,1);
    write16(sourceNav,0x80+0x0a,2);
    std::array<OriginalSpatialRecord70,2> polygons{};
    const std::array<OriginalVU0Vec4,3> originalPoints{{
        {1,2,3,4}, {11,12,13,14}, {21,22,23,24}
    }};
    for (std::size_t i=0; i<3; ++i)
        std::memcpy(polygons[1].originalFields.data() + 0x40 + 16*i,
                    &originalPoints[i],sizeof(OriginalVU0Vec4));
    const std::array<std::uint32_t,2> route{0,1};
    OriginalManagerChild70 child{};
    child.flag1c = 1;
    OriginalManagerInitState_57bcb8 state{child};
    OriginalManagerInitSource_57bcb8 source{};
    source.actorPolygon30 = &polygons[0];
    source.stageRecords = polygons;
    source.routePolygonIds = route;
    source.originalNavigation = sourceNav;
    OriginalGameRandom_36f3c0 gameRng(5489), referenceRng(5489);
    const auto expected = originalActorRandomTriangle_57c98c(originalPoints,referenceRng);
    OriginalCalls calls{};
    const auto branch = originalManagerInitialize_57bcb8(
        state,source,gameRng,calls);
    require(branch == OriginalManagerInitBranch_57bcb8::typeTwoTarget,
            "Original kind-2 actor route branch was not executed");
    require(state.target50 == &polygons[1] &&
            same(state.target40,expected) &&
            child.word08 == 0x57c554 && child.word0c == 0 &&
            gameRng.originalCursor() == referenceRng.originalCursor() &&
            calls.projections == 0 && calls.followed == 0,
            "Source polygon target, RNG draw order or method pair changed");
    // A corrupted stage route must fail rather than pick a fake polygon.
    const std::array<std::uint32_t,2> malformedRoute{0,5};
    source.routePolygonIds = malformedRoute;
    bool rejected = false;
    try {
        (void)originalManagerInitialize_57bcb8(
            state,source,gameRng,calls);
    } catch (const std::out_of_range&) { rejected = true; }
    require(rejected, "Invalid original route was accepted for actor initialization");
}
} // namespace
int main() {
    try {
        checkNearOwnerReturn();
        checkVariantOneRejectsMissingOrContradictorySQ();
        checkSourceConstructorToInitializerChain();
        checkGeneralProjectionAndUnavailableSource();
        checkRealOriginalTypeTwoRouteAndRng();
        std::cout << "ORIGINAL_MANAGER_INIT_57BCB8_SOURCE_BRANCHES_PASSED\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ORIGINAL_MANAGER_INIT_57BCB8_FAILED: " << e.what() << '\n';
        return 1;
    }
}
