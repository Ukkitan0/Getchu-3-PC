#include "ae3/original_audio_runtime.hpp"
#include <stdexcept>
#include <utility>

namespace ae3 {
std::optional<OriginalSoundRoute> originalSoundRoute(std::string_view opcode) noexcept {
    // Original SCUS_975.01 command comparisons and their direct J targets.
    // 0x2191D4 se, 0x2190D8 se_stop, 0x219454 add_entity_se,
    // 0x2195D0 kick_entity_se. Do not treat kick_entity_effect as sound.
    if(opcode=="se")return OriginalSoundRoute{OriginalSoundCommand::se,0x2191d4,0x373264,0xa0};
    if(opcode=="se_stop")return OriginalSoundRoute{OriginalSoundCommand::se_stop,0x2190d8,0x3732fc,0x70};
    if(opcode=="add_entity_se")return OriginalSoundRoute{OriginalSoundCommand::add_entity_se,0x219454,0x3738dc,0xd0};
    if(opcode=="kick_entity_se")return OriginalSoundRoute{OriginalSoundCommand::kick_entity_se,0x2195d0,0x373960,0xb0};
    return std::nullopt;
}
std::optional<OriginalSeLuaFields> originalSeLuaFields_2191d4(
    const NativeControllerEvent& event) noexcept {
    if(event.opcode!="se")return std::nullopt;
    OriginalSeLuaFields fields;
    const auto& args=event.parameters;
    if(args.kind!=ScriptValue::Kind::object)return fields;
    if(const auto* name=args.member("name"); name && name->kind==ScriptValue::Kind::text)
        fields.name=name->text;
    if(const auto* target=args.member("target_name"); target && target->kind==ScriptValue::Kind::text)
        fields.targetName=target->text;
    if(const auto* volume=args.member("vol"); volume && volume->kind==ScriptValue::Kind::number){
        fields.volume=static_cast<float>(volume->number);
        fields.volumeSupplied=true;
    }
    return fields;
}
std::optional<OriginalEntitySoundLuaFields> originalEntitySoundLuaFields(
    const NativeControllerEvent& event) noexcept {
    const bool add=event.opcode=="add_entity_se";
    if(!add && event.opcode!="kick_entity_se")return std::nullopt;
    OriginalEntitySoundLuaFields f;
    const auto& params=event.parameters;
    if(params.kind!=ScriptValue::Kind::object)return f;
    auto text=[&](const char* key)->std::optional<std::string>{
        const auto* v=params.member(key);
        return v && v->kind==ScriptValue::Kind::text ?
            std::optional<std::string>(v->text):std::nullopt;
    };
    f.exdbSeId=text("exdb_se_id");f.entityName=text("entity_name");
    if(add) {
        f.watchRollerhandleName=text("watch_rollerhandle_name");
        if(const auto* v=params.member("sign");v && v->kind==ScriptValue::Kind::number)
            f.sign=static_cast<float>(v->number);
        if(const auto* v=params.member("is_2d");v && v->kind==ScriptValue::Kind::boolean)
            f.is2d=v->boolean;
        if(const auto* v=params.member("is_3d");v && v->kind==ScriptValue::Kind::boolean)
            f.is3d=v->boolean;
    }else f.markName=text("mark_name");
    return f;
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
