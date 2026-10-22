#include "ae3/ae3_unreal_lua_bridge.h"
#include "ae3/lua_stage.hpp"

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
std::string hexBytes(std::string_view bytes) {
    static constexpr char alphabet[] = "0123456789abcdef";
    std::string out;
    if (bytes.size() > 8 * 1024 * 1024)
        throw std::runtime_error("original Lua byte string exceeds bridge limit");
    out.reserve(bytes.size() * 2);
    for (const unsigned char value : bytes) {
        out.push_back(alphabet[value >> 4]);
        out.push_back(alphabet[value & 0x0f]);
    }
    return out;
}

void appendChecked(std::string& output, std::string_view text) {
    constexpr std::size_t limit = 16 * 1024 * 1024;
    if (text.size() > limit || output.size() > limit - text.size())
        throw std::runtime_error("original Lua typed argument exceeds bridge data limit");
    output.append(text);
}

void appendValue(std::string& out, const ae3::ScriptValue& value,
                 unsigned depth, std::size_t& remaining) {
    if (depth > 24 || remaining-- == 0)
        throw std::runtime_error("original Lua argument table exceeds safe limits");
    using K = ae3::ScriptValue::Kind;
    switch (value.kind) {
    case K::nil:
        appendChecked(out, R"({"kind":"nil"})");
        break;
    case K::boolean:
        appendChecked(out, value.boolean ? R"({"kind":"boolean","value":true})"
                                         : R"({"kind":"boolean","value":false})");
        break;
    case K::number: {
        if (!std::isfinite(value.number))
            throw std::runtime_error("nonfinite original Lua value");
        std::ostringstream formatted;
        formatted.imbue(std::locale::classic());
        formatted << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << value.number;
        appendChecked(out, R"({"kind":"number","value":)");
        appendChecked(out, formatted.str());
        appendChecked(out, "}");
        break;
    }
    case K::text:
        appendChecked(out, R"({"kind":"text","hex":")");
        appendChecked(out, hexBytes(value.text));
        appendChecked(out, R"("})");
        break;
    case K::array:
        appendChecked(out, R"({"kind":"array","items":[)");
        for (const auto& item : value.array) {
            if (&item != &value.array.front()) appendChecked(out, ",");
            appendValue(out, item, depth + 1, remaining);
        }
        appendChecked(out, "]}");
        break;
    case K::object: {
        appendChecked(out, R"({"kind":"object","entries":[)");
        bool first = true;
        for (const auto& item : value.object) {
            if (!first) appendChecked(out, ",");
            first = false;
            appendChecked(out, R"({"key_hex":")");
            appendChecked(out, hexBytes(item.first));
            appendChecked(out, R"(","value":)");
            appendValue(out, item.second, depth + 1, remaining);
            appendChecked(out, "}");
        }
        appendChecked(out, "]}");
        break;
    }
    }
}

std::string typed(const ae3::ScriptValue& value) {
    std::string out;
    std::size_t budget = 16384;
    appendValue(out, value, 0, budget);
    return out;
}
std::string typedArgs(const std::vector<ae3::ScriptValue>& args) {
    if (args.size() > 64)
        throw std::runtime_error("too many original Lua native callback arguments");
    std::string out = "[";
    std::size_t budget = 16384;
    bool first = true;
    for (const auto& item : args) {
        if (!first) appendChecked(out, ",");
        first = false;
        appendValue(out, item, 0, budget);
    }
    appendChecked(out, "]");
    return out;
}
void requireManagerSuccess(int accepted, const char* operation) {
    if (accepted != 1)
        throw std::runtime_error(
            std::string("original game manager rejected ") + operation);
}
} // namespace

struct Ae3LuaRuntime {
    Ae3LuaManagerV2 callbacks{};
    ae3::NativeStageWorld stageWorld;
    std::optional<ae3::PlayerProgress> sourceProgress;
    std::unique_ptr<ae3::LuaStage> stage;
    std::string error;
    bool busy = false;

    explicit Ae3LuaRuntime(const Ae3LuaManagerV2& input) : callbacks(input) {
        resetStage();
    }
    void resetStage() {
        sourceProgress.reset();
        if (callbacks.read_original_player_progress) {
            ae3::PlayerProgress original{};
            if (callbacks.read_original_player_progress(
                    callbacks.user, &original, sizeof(original)) == 1)
                sourceProgress = original;
        }
        auto created = std::make_unique<ae3::LuaStage>(
            stageWorld, sourceProgress ? &*sourceProgress : nullptr);
        ae3::NativeLuaStageBindings bindings;
        if (callbacks.create_entity) {
            bindings.createEntity = [this](const ae3::StageEntity& original) {
                const std::string fields = typed(original.fields);
                const std::string controllers = typed(original.controllers);
                requireManagerSuccess(callbacks.create_entity(
                    callbacks.user, hexBytes(original.entityClass).c_str(),
                    hexBytes(original.name).c_str(), hexBytes(original.tagName).c_str(),
                    fields.c_str(), controllers.c_str()), "new_ent");
            };
        }
        if (callbacks.submit_parameters) {
            bindings.submitParameters = [this](const ae3::ScriptValue& params) {
                const auto body = typed(params);
                requireManagerSuccess(
                    callbacks.submit_parameters(callbacks.user, body.c_str()),
                    "lua_param");
            };
        }
        if (callbacks.write_flag) {
            bindings.writeFlag = [this](
                std::string_view scope, std::string_view key, bool value) {
                const std::string s = hexBytes(scope), k = hexBytes(key);
                requireManagerSuccess(callbacks.write_flag(
                    callbacks.user, s.c_str(), k.c_str(), value ? 1 : 0),
                    "original flag write");
            };
        }
        if (callbacks.read_flag) {
            bindings.readFlag = [this](std::string_view scope,
                                       std::string_view key) -> std::optional<bool> {
                int result = 0;
                const std::string s = hexBytes(scope), k = hexBytes(key);
                if (callbacks.read_flag(callbacks.user, s.c_str(), k.c_str(),
                                        &result) != 1)
                    return {};
                return result != 0;
            };
        }
        if (callbacks.read_stage_int) {
            bindings.readStageInteger = [this](
                const std::vector<ae3::ScriptValue>& params) -> std::optional<int> {
                const std::string bytes = typedArgs(params);
                int result = 0;
                if (callbacks.read_stage_int(callbacks.user, bytes.c_str(),
                                             &result) != 1) return {};
                return result;
            };
        }
        if (callbacks.read_survival) {
            bindings.readSurvival = [this]() -> std::optional<bool> {
                int result = 0;
                if (callbacks.read_survival(callbacks.user, &result) != 1) return {};
                return result != 0;
            };
        }
        if (callbacks.select_weapon) {
            bindings.selectWeapon = [this](int value) {
                requireManagerSuccess(
                    callbacks.select_weapon(callbacks.user, value),
                    "original player weapon selection");
            };
        }
        created->setNativeBindings(std::move(bindings));
        stage = std::move(created);
    }
};

namespace {
template <typename Action>
int callSafely(Ae3LuaRuntime* runtime, Action&& action) noexcept {
    if (!runtime) return 0;
    if (runtime->busy) {
        runtime->error = "reentrant original Lua game manager call is forbidden";
        return 0;
    }
    runtime->busy = true;
    runtime->error.clear();
    try {
        action();
        runtime->busy = false;
        return 1;
    } catch (const std::exception& e) {
        runtime->error = e.what();
    } catch (...) {
        runtime->error = "unknown native Lua 5.0 host exception";
    }
    runtime->busy = false;
    return 0;
}
} // namespace

extern "C" uint32_t ae3_lua_api_version(void) {
    return AE3_LUA_BRIDGE_API_VERSION;
}
extern "C" Ae3LuaRuntime* ae3_lua_create(const Ae3LuaManagerV2* manager) {
    if (!manager || manager->api_version != AE3_LUA_BRIDGE_API_VERSION ||
        manager->struct_size != sizeof(Ae3LuaManagerV2))
        return nullptr;
    // Original stage gameplay requires a real actor AND parameter manager.
    // A stub accepting records would be a false gameplay implementation.
    if (!manager->create_entity || !manager->submit_parameters)
        return nullptr;
    try {
        return new Ae3LuaRuntime(*manager);
    } catch (...) { return nullptr; }
}
extern "C" void ae3_lua_destroy(Ae3LuaRuntime* runtime) {
    delete runtime;
}
extern "C" int ae3_lua_reset(Ae3LuaRuntime* runtime) {
    return callSafely(runtime, [&] {
        runtime->stage.reset();
        runtime->stageWorld.clear();
        runtime->resetStage();
    });
}
extern "C" int ae3_lua_execute_original_chunk(
    Ae3LuaRuntime* runtime, const void* bytes, size_t length,
    const char* original_chunk_path) {
    return callSafely(runtime, [&] {
        if (!runtime->stage)
            throw std::runtime_error("original Lua 5.0 VM not initialized after reset");
        if (!bytes || !original_chunk_path || !*original_chunk_path ||
            length < 5 || length > 16 * 1024 * 1024)
            throw std::invalid_argument("invalid original Lua 5.0 stage chunk");
        const auto* data = static_cast<const char*>(bytes);
        if (std::string_view(data, 5) != std::string_view("\x1bLua\x50", 5))
            throw std::invalid_argument(
                "refusing non-retail Lua source or wrong bytecode version");
        runtime->stage->executeSource(
            std::string_view(data, length), original_chunk_path);
    });
}
extern "C" int ae3_lua_setup_area(Ae3LuaRuntime* runtime,
                                   const char* original_spawn_tag) {
    return callSafely(runtime, [&] {
        if (!runtime->stage)
            throw std::runtime_error("original Lua 5.0 VM not initialized after reset");
        if (!original_spawn_tag || !*original_spawn_tag)
            throw std::invalid_argument("missing source-backed spawn tag");
        runtime->stage->callSetupArea(original_spawn_tag);
    });
}
extern "C" const char* ae3_lua_last_error(const Ae3LuaRuntime* runtime) {
    return runtime ? runtime->error.c_str() : "null original Lua runtime";
}
extern "C" size_t ae3_lua_declared_entity_count(const Ae3LuaRuntime* runtime) {
    return runtime ? runtime->stageWorld.entities().size() : 0;
}
extern "C" size_t ae3_lua_parameter_count(const Ae3LuaRuntime* runtime) {
    return runtime ? runtime->stageWorld.parameters().size() : 0;
}
