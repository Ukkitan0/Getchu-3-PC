#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG // Keep all real ABI assertions active in Windows Release builds.
#endif
#include <cassert>
#include <cstring>

namespace {
int rejectEntity(void*, const char*, const char*, const char*,
                 const char*, const char*) { return 0; }
int rejectParams(void*, const char*) { return 0; }
int acceptEntity(void*, const char*, const char*, const char*,
                 const char*, const char*) { return 1; }
int acceptParams(void*, const char*) { return 1; }
int rejectPackedProgress(void*, void*, size_t bytes) { return bytes == 1033 ? 0 : 0; }
int acceptTestPackedProgress(void*, void* destination, size_t bytes) {
    if (!destination || bytes != 1033) return 0;
    std::memset(destination, 0, bytes); // Test fixture only, never a game save.
    return 1;
}
Ae3LuaManagerV2 callbacks() {
    Ae3LuaManagerV2 manager{};
    manager.api_version = AE3_LUA_BRIDGE_API_VERSION;
    manager.struct_size = sizeof(manager);
    manager.create_entity = rejectEntity;
    manager.submit_parameters = rejectParams;
    return manager;
}
}
int main() {
    assert(ae3_lua_api_version() == AE3_LUA_BRIDGE_API_VERSION);
    assert(ae3_lua_create(nullptr) == nullptr);
    auto manager = callbacks();
    manager.api_version = 999;
    assert(ae3_lua_create(&manager) == nullptr);
    manager = callbacks();
    manager.struct_size = 1;
    assert(ae3_lua_create(&manager) == nullptr);
    manager = callbacks();
    manager.create_entity = nullptr;
    assert(ae3_lua_create(&manager) == nullptr);
    manager = callbacks();
    manager.submit_parameters = nullptr;
    assert(ae3_lua_create(&manager) == nullptr);

    manager = callbacks();
    auto* runtime = ae3_lua_create(&manager);
    assert(runtime != nullptr);
    assert(ae3_lua_declared_entity_count(runtime) == 0);
    assert(ae3_lua_parameter_count(runtime) == 0);
    assert(!ae3_lua_execute_original_chunk(runtime, nullptr, 0, "area.luc"));
    assert(std::strstr(ae3_lua_last_error(runtime), "invalid original") != nullptr);
    const char source[] = "new_ent('ape', {})";
    assert(!ae3_lua_execute_original_chunk(runtime, source,
             sizeof(source) - 1, "area.luc"));
    assert(std::strstr(ae3_lua_last_error(runtime), "non-retail") != nullptr);

    // Wrong/incomplete retail stream must fail with no manager calls.
    const unsigned char invalid[] = {0x1b, 'L', 'u', 'a', 0x50, 0, 0};
    assert(!ae3_lua_execute_original_chunk(runtime, invalid,
                                           sizeof(invalid), "area.luc"));
    assert(std::strstr(ae3_lua_last_error(runtime), "original Lua 5.0") != nullptr);

    assert(!ae3_lua_setup_area(runtime, nullptr));
    assert(std::strstr(ae3_lua_last_error(runtime), "spawn tag") != nullptr);
    assert(!ae3_lua_setup_area(runtime, "spawn_default"));
    assert(std::strstr(ae3_lua_last_error(runtime), "did not define setupArea") != nullptr);

    assert(ae3_lua_reset(runtime) == 1);
    assert(ae3_lua_declared_entity_count(runtime) == 0);
    assert(ae3_lua_parameter_count(runtime) == 0);

    ae3_lua_destroy(runtime);
    // Also verify accepting manager pointers do not imply playability; the
    // original game chunk must still be source-verified and executable.
    manager.create_entity = acceptEntity;
    manager.submit_parameters = acceptParams;
    runtime = ae3_lua_create(&manager);
    assert(runtime != nullptr);
    assert(!ae3_lua_execute_original_chunk(runtime, source,
             sizeof(source) - 1, "area.luc"));
    ae3_lua_destroy(runtime);

    // Missing original save state must not be replaced by a synthetic save.
    manager.read_original_player_progress = rejectPackedProgress;
    runtime = ae3_lua_create(&manager);
    assert(runtime != nullptr);
    assert(!ae3_lua_execute_original_chunk(runtime, source,
             sizeof(source) - 1, "area.luc"));
    ae3_lua_destroy(runtime);
    // Only the exact 1033-byte source-backed ABI can be supplied by a manager.
    manager.read_original_player_progress = acceptTestPackedProgress;
    runtime = ae3_lua_create(&manager);
    assert(runtime != nullptr);
    assert(ae3_lua_reset(runtime));
    ae3_lua_destroy(runtime);
    return 0;
}
