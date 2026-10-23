#include "ae3/original_actor_polygon_selector_30ffd0.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <bit>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;

namespace {
struct OriginalGameCalls final:OriginalActorPolygonSelectorCalls_30ffd0 {
    const OriginalSpatialRecord70 *eligible=nullptr,*nearest=nullptr;
    std::vector<std::string> calls;
    bool rejectEligibility=false,rejectNearest=false;
    bool call_original_21b788(
        const OriginalSpatialRecord70& polygon,float f12) override {
        assert(std::bit_cast<std::uint32_t>(f12)==0x3dcccccdu);
        calls.push_back("eligibility");
        if (rejectEligibility)
            throw std::runtime_error("genuine original polygon eligibility unavailable");
        return &polygon==eligible;
    }
    const OriginalSpatialRecord70* call_original_21d898() override {
        calls.push_back("nearest");
        if (rejectNearest)
            throw std::runtime_error("original registry lookup unavailable");
        return nearest;
    }
};
}
int main() {
    OriginalSpatialRecord70 a{},b{},c{};
    OriginalGameCalls game{};
    const OriginalSpatialRecord70* current=&a;

    // A valid incoming a1 wins and writes current +0x30 immediately.
    game.eligible=&b;
    assert(originalActorPolygonSelect_30ffd0(current,&b,game)==&b);
    assert(current==&b && game.calls==std::vector<std::string>{"eligibility"});

    // Rejected input falls back to existing eligible current polygon
    // instead of doing a needless original 0x21D898 lookup.
    game.calls.clear();game.eligible=&b;
    assert(originalActorPolygonSelect_30ffd0(current,&c,game)==&b);
    assert(game.calls==std::vector<std::string>({"eligibility","eligibility"}));

    // Rejected incoming + no current: source returns NULL WITHOUT
    // a new nearest lookup (original 0x31008C BEQZ path).
    game.calls.clear();game.eligible=nullptr;current=nullptr;
    assert(originalActorPolygonSelect_30ffd0(current,&c,game)==nullptr);
    assert(game.calls==std::vector<std::string>{"eligibility"});

    // With incoming rejected and a bad current polygon, original
    // falls back to the source grid nearest and publishes only if hit.
    game.calls.clear();current=&a;game.nearest=&b;
    assert(originalActorPolygonSelect_30ffd0(current,&c,game)==&b);
    assert(current==&b);
    assert(game.calls==std::vector<std::string>(
        {"eligibility","eligibility","nearest"}));

    // When nearest fails, original +0x30 retains its prior polygon.
    game.calls.clear();current=&a;game.nearest=nullptr;
    assert(originalActorPolygonSelect_30ffd0(current,&c,game)==&a);
    assert(current==&a);

    // Incoming NULL with empty current DOES trigger source nearest.
    game.calls.clear();current=nullptr;game.nearest=&c;
    assert(originalActorPolygonSelect_30ffd0(current,nullptr,game)==&c);
    assert(game.calls==std::vector<std::string>{"nearest"});

    // Incoming NULL with eligible current returns source pointer.
    game.calls.clear();current=&b;game.eligible=&b;game.nearest=&c;
    assert(originalActorPolygonSelect_30ffd0(current,nullptr,game)==&b);
    assert(game.calls==std::vector<std::string>{"eligibility"});

    // Native fail-closed behavior: missing original eligibility must
    // never overwrite the real actor's current+0x30 pointer.
    game.calls.clear();game.rejectEligibility=true;current=&a;
    bool rejected=false;
    try{(void)originalActorPolygonSelect_30ffd0(current,&b,game);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && current==&a);
    game.rejectEligibility=false;game.rejectNearest=true;
    game.eligible=nullptr;game.calls.clear();
    rejected=false;
    try{(void)originalActorPolygonSelect_30ffd0(current,nullptr,game);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && current==&a);
    std::cout<<"ORIGINAL_ACTOR_POLYGON_30FFD0_SOURCE_BRANCHES_PASSED\n";
}
