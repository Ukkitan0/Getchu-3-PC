#pragma once
#include "ae3/controller_timeline.hpp"
#include "ae3/original_audio_descriptor.hpp"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace ae3 {
// These addresses are confirmed command dispatch J targets, NOT proved
// decoding/playback entrypoints. Verified in original_entity_controller_routes.
enum class OriginalSoundCommand : std::uint8_t {
    se, se_stop, add_entity_se, kick_entity_se
};
struct OriginalSoundRoute {
    OriginalSoundCommand command;
    std::uint32_t originalDispatchVa;
};
std::optional<OriginalSoundRoute> originalSoundRoute(std::string_view opcode) noexcept;

// Retail "se" parser @ 0x2191D4 accepts these table keys; missing fields
// are NOT synthesized. Opaque/raw parameters remain in OriginalSoundEvent.
struct OriginalSeLuaFields {
    std::optional<std::string> name;
    std::optional<std::string> targetName;
    float volume=1.0f; // original literal @ 0x6E2DF4
    bool volumeSupplied=false;
};
// Numeric Lua conversion is direct; original 0x3E5BD0 string coercion
// remains an open dependency, so raw event parameters are also kept.
std::optional<OriginalSeLuaFields> originalSeLuaFields_2191d4(
    const NativeControllerEvent& event) noexcept;

struct OriginalSoundEvent {
    // Original Lua c_ctrl event is kept intact: name, target, timing,
    // volume and other fields are NOT inferred, renamed or dropped.
    NativeControllerEvent originalControllerEvent;
    OriginalSoundRoute sourceRoute{};
    std::string originalEntityName;
    // Optional resolved ORIGINAL descriptor from asset team. None means
    // unresolved, not a request to play an invented fallback sound.
    std::optional<OriginalAudioDescriptorValues> sourceDescriptor;
};
class OriginalSoundEventSink {
public:
    virtual ~OriginalSoundEventSink()=default;
    // True means the actual original event has been accepted. False means
    // retry later (e.g. decoder/resource/backend not ready).
    virtual bool submitOriginalSoundEvent(const OriginalSoundEvent& event)=0;
};
class OriginalSoundEventRuntime {
public:
    // Unrelated stage commands return false, and must go to their own
    // original engine dispatcher. Events stay ordered even when blocked.
    bool enqueue(const NativeControllerEvent& stageCommand,
                 std::string originalEntityName={},
                 std::optional<OriginalAudioDescriptorValues> sourceDescriptor=std::nullopt);
    // Stop on first rejection; never discard commands during device loss.
    std::size_t pump(OriginalSoundEventSink& receiver,
                     std::size_t maxToPump=4096);
    std::size_t pending() const noexcept {return pending_.size();}
    void clearOnOriginalStageTeardown() noexcept {pending_.clear();}
private:
    std::deque<OriginalSoundEvent> pending_;
};
} // namespace ae3
