#pragma once
#include "ae3/lua_stage.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace ae3 {
// Deterministic native event sequencing for c_order/c_ctrl trees produced
// by ORIGINAL Lua stage scripts. Does NOT simulate PS2 hardware or AI.
struct NativeControllerEvent {
    double gameSeconds = 0;
    std::string opcode;
    ScriptValue parameters;
};

class NativeControllerTimeline {
public:
    explicit NativeControllerTimeline(const ScriptValue& order);
    // Advance with elapsed native game time. Discrete events returned in
    // order. No sleeps, timers, frame-rate dependency or guest CPU state.
    std::vector<NativeControllerEvent> advance(double elapsedSeconds);
    // Trig_wait becomes ready only when the native world explicitly signals.
    bool signal(const std::string& name,bool active=true);
    bool isWaitingForTrigger() const noexcept {return !trigger_.empty();}
    const std::string& waitingTriggerName() const noexcept {return trigger_;}
    bool expectedTriggerActive() const noexcept {return triggerActive_;}
    bool finished() const noexcept {return finished_;}
    double elapsed() const noexcept {return elapsed_;}
private:
    struct Instruction {
        enum class Kind { begin, end, control } kind=Kind::control;
        std::size_t link=0;
        int repeat=1;
        std::string opcode;
        ScriptValue args;
    };
    void compile(const ScriptValue& root,unsigned depth);
    std::vector<Instruction> program_;
    std::map<std::size_t,int> remaining_;
    std::size_t pc_=0;
    double elapsed_=0;
    double wait_=0;
    std::string trigger_;
    bool triggerActive_=true;
    bool finished_=false;
};
}
