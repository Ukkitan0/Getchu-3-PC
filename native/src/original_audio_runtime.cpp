#include "ae3/original_audio_runtime.hpp"
#include <stdexcept>
#include <utility>

namespace ae3 {
std::optional<OriginalSoundRoute> originalSoundRoute(std::string_view opcode) noexcept {
    // Original SCUS_975.01 command comparisons and their direct J targets.
    // 0x2191D4 se, 0x2190D8 se_stop, 0x219454 add_entity_se,
    // 0x2195D0 kick_entity_se. Do not treat kick_entity_effect as sound.
    if(opcode=="se")return OriginalSoundRoute{OriginalSoundCommand::se,0x2191d4};
    if(opcode=="se_stop")return OriginalSoundRoute{OriginalSoundCommand::se_stop,0x2190d8};
    if(opcode=="add_entity_se")return OriginalSoundRoute{OriginalSoundCommand::add_entity_se,0x219454};
    if(opcode=="kick_entity_se")return OriginalSoundRoute{OriginalSoundCommand::kick_entity_se,0x2195d0};
    return std::nullopt;
}
bool OriginalSoundEventRuntime::enqueue(
    const NativeControllerEvent& stageCommand,std::string originalEntityName,
    std::optional<OriginalAudioDescriptorValues> sourceDescriptor) {
    const auto route=originalSoundRoute(stageCommand.opcode);
    if(!route)return false;
    if(pending_.size()>=65536)
        throw std::length_error("original sound command queue reached host safety limit");
    pending_.push_back({stageCommand,*route,std::move(originalEntityName),
                        std::move(sourceDescriptor)});
    return true;
}
std::size_t OriginalSoundEventRuntime::pump(
    OriginalSoundEventSink& receiver,std::size_t maxToPump) {
    std::size_t sent=0;
    while(sent<maxToPump&&!pending_.empty()){
        if(!receiver.submitOriginalSoundEvent(pending_.front()))break;
        pending_.pop_front();
        ++sent;
    }
    return sent;
}
} // namespace ae3
