#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ae3::runtime {
enum class BootCheckpoint {
    windows_window, graphics_device, original_disc, original_static_constructors, original_stage_lua,
    original_stage_geometry, original_collision, original_lua_setup,
    original_actors, original_engine_loop
};
enum class BootResult { passed, blocked };
struct BootObservation {
    BootCheckpoint checkpoint{};
    BootResult result{};
    std::string detail;
};
// Only host-native startup evidence; successful original game boot is
// never inferred from successful Lua execution or mesh loading.
class BootJournal {
public:
    void reset() { observations_.clear(); blocked_ = false; }
    bool record(BootCheckpoint checkpoint, BootResult result, std::string detail) {
        if (blocked_) return false;
        observations_.push_back({checkpoint,result,std::move(detail)});
        blocked_ = result == BootResult::blocked;
        return true;
    }
    bool blocked() const noexcept { return blocked_; }
    bool originalEngineLoopReached() const noexcept {
        for (const auto& step : observations_)
            if (step.checkpoint == BootCheckpoint::original_engine_loop &&
                step.result == BootResult::passed) return true;
        return false;
    }
    const std::vector<BootObservation>& observations() const noexcept { return observations_; }
    std::string lastBlocker() const {
        for (auto it=observations_.rbegin(); it!=observations_.rend(); ++it)
            if (it->result == BootResult::blocked) return it->detail;
        return {};
    }
    static std::string_view label(BootCheckpoint c) noexcept {
        switch(c) {
        case BootCheckpoint::windows_window: return "WINDOWS_WINDOW";
        case BootCheckpoint::graphics_device: return "D3D11_DEVICE";
        case BootCheckpoint::original_disc: return "RETAIL_DISC";
        case BootCheckpoint::original_static_constructors: return "RETAIL_STATIC_CONSTRUCTORS";
        case BootCheckpoint::original_stage_lua: return "RETAIL_STAGE_LUA";
        case BootCheckpoint::original_stage_geometry: return "STAGE_GEOMETRY";
        case BootCheckpoint::original_collision: return "STAGE_COLLISION";
        case BootCheckpoint::original_lua_setup: return "RETAIL_LUA_SETUP";
        case BootCheckpoint::original_actors: return "RETAIL_ACTOR_BINDING";
        case BootCheckpoint::original_engine_loop: return "RETAIL_ENGINE_LOOP";
        }
        return "UNKNOWN";
    }
    std::string trace() const {
        std::string out;
        for(const auto& step : observations_) {
            out += "AE3_NATIVE_BOOT|";
            out += label(step.checkpoint);
            out += step.result == BootResult::passed ? "|PASS|" : "|BLOCKED|";
            for(char ch : step.detail) {
                out += (ch=='\r'||ch=='\n'||ch=='|') ? ' ' : ch;
            }
            out += '\n';
        }
        return out;
    }
private:
    std::vector<BootObservation> observations_;
    bool blocked_ = false;
};
} // namespace ae3::runtime
