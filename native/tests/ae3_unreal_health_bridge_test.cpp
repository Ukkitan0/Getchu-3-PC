#include "ae3/ae3_unreal_lua_bridge.h"
#include <array>
#ifdef NDEBUG
#undef NDEBUG // CTest runs Windows Release; never compile the checks away.
#endif
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

// All data are synthetically constructed for tests; no retail game files.
struct OwnerProgressFixture {
    std::array<std::uint8_t, 1033> bytes{};
    bool readPermitted = true;
    bool writePermitted = true;
    int successfulWrites = 0;
    OwnerProgressFixture() {
        float health = 90.f;
        std::memcpy(bytes.data()+8, &health, sizeof health);
        bytes[204+569] = 1;
        bytes[1032] = 77;
    }
    float life() const {
        float result = 0;
        std::memcpy(&result, bytes.data()+8, sizeof result);
        return result;
    }
};
int originalRead(void* user, void* destination, size_t length) {
    auto* state = static_cast<OwnerProgressFixture*>(user);
    if (!state || !state->readPermitted || !destination || length != 1033)
        return 0;
    std::memcpy(destination, state->bytes.data(), length);
    return 1;
}
int originalWrite(void* user, const void* source, size_t length) {
    auto* state = static_cast<OwnerProgressFixture*>(user);
    if (!state || !state->writePermitted || !source || length != 1033)
        return 0;
    std::memcpy(state->bytes.data(), source, length);
    ++state->successfulWrites;
    return 1;
}
int rejectEntity(void*, const char*,const char*,const char*,const char*,const char*) {return 0;}
int rejectParameters(void*,const char*) {return 0;}

int main() {
    OwnerProgressFixture owned;
    Ae3LuaManagerV4 callbacks{};
    callbacks.api_version = AE3_LUA_BRIDGE_API_VERSION;
    callbacks.struct_size = sizeof callbacks;
    callbacks.user = &owned;
    callbacks.create_entity = rejectEntity;
    callbacks.submit_parameters = rejectParameters;
    callbacks.read_original_player_progress = originalRead;

    // No original write manager: fail, leave original bytes and output intact.
    Ae3LuaRuntime* vm = ae3_lua_create(&callbacks);
    assert(vm);
    float health = -1.f;
    assert(!ae3_lua_apply_original_health_delta(vm, -30.f, &health));
    assert(health == -1.f && owned.life() == 90.f);
    assert(owned.successfulWrites == 0);
    assert(ae3_lua_destroy(vm) == 1);

    callbacks.write_original_player_progress = originalWrite;
    vm = ae3_lua_create(&callbacks);
    assert(vm);
    assert(ae3_lua_apply_original_health_delta(vm, -30.f, &health));
    assert(health == 60.f && owned.life() == 60.f);
    assert(ae3_lua_apply_original_health_delta(vm, 90.f, &health));
    assert(health == 100.f && owned.life() == 100.f);
    assert(ae3_lua_apply_original_health_delta(vm, -300.f, &health));
    assert(health == 0.f && owned.life() == 0.f);
    assert(owned.successfulWrites == 3);
    assert(owned.bytes[204+569] == 1 && owned.bytes[1032] == 77);

    owned.writePermitted = false;
    health = -1.f;
    assert(!ae3_lua_apply_original_health_delta(vm, 12.f, &health));
    assert(health == -1.f && owned.life() == 0.f);
    owned.writePermitted = true;
    owned.readPermitted = false;
    assert(!ae3_lua_apply_original_health_delta(vm, 12.f, &health));
    assert(owned.life() == 0.f);
    owned.readPermitted = true;
    assert(!ae3_lua_apply_original_health_delta(
        vm, std::numeric_limits<float>::quiet_NaN(), &health));
    assert(!ae3_lua_apply_original_health_delta(vm, 1.f, nullptr));
    assert(ae3_lua_destroy(vm) == 1);
    return 0;
}
