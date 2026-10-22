#include "ae3/controller_timeline.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace ae3 {
namespace {
const ScriptValue& required(const ScriptValue& value,const char* field){
    const ScriptValue* result=value.member(field);
    if(!result)throw std::runtime_error(std::string("native Lua controller missing ")+field);
    return *result;
}
std::string stringField(const ScriptValue& value,const char* field){
    const auto& found=required(value,field);
    if(found.kind!=ScriptValue::Kind::text)
        throw std::runtime_error(std::string("native Lua controller expects text ")+field);
    return found.text;
}
double numberFieldOr(const ScriptValue& value,const char* key,double fallback){
    const ScriptValue* f=value.member(key);
    return f && f->kind==ScriptValue::Kind::number?f->number:fallback;
}
}
NativeControllerTimeline::NativeControllerTimeline(const ScriptValue& root){
    compile(root,0);
}
void NativeControllerTimeline::compile(const ScriptValue& root,unsigned depth){
    if(depth>48 || program_.size()>8192)
        throw std::runtime_error("native Lua controller graph exceeds safe nesting/size limit");
    if(root.kind!=ScriptValue::Kind::object)
        throw std::runtime_error("native Lua c_ctrl/c_order node is not a table");
    auto kind=stringField(root,"kind");
    if(kind=="control"){
        Instruction command;
        command.kind=Instruction::Kind::control;
        command.opcode=stringField(root,"opcode");
        if(auto* args=root.member("params"))command.args=*args;
        program_.push_back(std::move(command));
    }else if(kind=="order"){
        double n=numberFieldOr(root,"repeat",1);
        if(!std::isfinite(n) || n!=std::floor(n) || n< -1 || n>100000)
            throw std::runtime_error("invalid original Lua c_order repeat count");
        const auto& children=required(root,"children");
        if(children.kind!=ScriptValue::Kind::array)
            throw std::runtime_error("native Lua c_order missing sequential children");
        const std::size_t begin=program_.size();
        Instruction first;
        first.kind=Instruction::Kind::begin;
        first.repeat=static_cast<int>(n);
        program_.push_back(std::move(first));
        for(const auto& child:children.array)compile(child,depth+1);
        const std::size_t end=program_.size();
        Instruction last;
        last.kind=Instruction::Kind::end;
        last.link=begin;
        program_.push_back(std::move(last));
        program_[begin].link=end;
    }else throw std::runtime_error("unknown Lua c_ctrl/c_order graph kind");
}
std::vector<NativeControllerEvent> NativeControllerTimeline::advance(double seconds){
    if(!std::isfinite(seconds)||seconds<0||seconds>3600)
        throw std::runtime_error("native controller delta must be finite 0..3600 seconds");
    std::vector<NativeControllerEvent> emitted;
    if(finished_)return emitted;
    if(!trigger_.empty()){
        // Events fire only when the native engine sends the specific trigger.
        elapsed_+=seconds;
        return emitted;
    }
    double budget=seconds;
    std::size_t visited=0;
    while(!finished_){
        if(++visited>32768)
            throw std::runtime_error("Lua controller loop has no wait/trigger; refusing native frame livelock");
        if(wait_>0){
            double used=std::min(wait_,budget);
            wait_-=used;
            budget-=used;
            elapsed_+=used;
            if(wait_>0)return emitted;
        }
        if(pc_>=program_.size()){finished_=true;return emitted;}
        const auto& action=program_[pc_];
        if(action.kind==Instruction::Kind::begin){
            if(action.repeat==0){pc_=action.link+1;continue;}
            remaining_.try_emplace(pc_,action.repeat);
            ++pc_;
        }else if(action.kind==Instruction::Kind::end){
            auto iter=remaining_.find(action.link);
            if(iter==remaining_.end())throw std::runtime_error("corrupt Lua repeat stack");
            if(iter->second==-1 || iter->second>1){
                if(iter->second>1)--iter->second;
                pc_=action.link+1;
            }else{
                remaining_.erase(iter);
                ++pc_;
            }
        }else{
            ++pc_;
            if(action.opcode=="wait"){
                // Retail 0x21937C loads the default 1.0f from 0x6E2E00
                // before the optional Lua "time" field (0x6E2DF8).
                const double delay=numberFieldOr(action.args,"time",1.0);
                if(!std::isfinite(delay)||delay<0||delay>3600)
                    throw std::runtime_error("invalid Lua wait duration");
                wait_=delay;
                if(wait_==0)continue;
            }else if(action.opcode=="trig_wait"){
                trigger_=stringField(action.args,"name");
                const auto* expected=action.args.member("flag");
                triggerActive_=expected?expected->boolean:true;
                return emitted;
            }else{
                emitted.push_back({elapsed_,action.opcode,action.args});
                if(emitted.size()>4096)
                    throw std::runtime_error("Lua controller emitted too many native events per tick");
            }
        }
        if(wait_>0 && budget==0)return emitted;
    }
    return emitted;
}
bool NativeControllerTimeline::signal(const std::string& name,bool active){
    if(trigger_.empty()||trigger_!=name||active!=triggerActive_)return false;
    trigger_.clear();
    return true;
}
} // namespace ae3
