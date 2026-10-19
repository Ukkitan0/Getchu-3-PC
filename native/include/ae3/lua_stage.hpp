#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct lua_State;

namespace ae3 {
// The original AE3 stage scripts are Lua 5.0. Original-style bindings
// translate script declarations directly to host-native world descriptors.
// No PlayStation CPU, IOP, GS, DMA or other hardware simulation is involved.
struct ScriptValue {
    enum class Kind { nil, boolean, number, text, object, array };
    Kind kind = Kind::nil;
    bool boolean = false;
    double number = 0;
    std::string text;
    std::map<std::string,ScriptValue> object;
    std::vector<ScriptValue> array;
    const ScriptValue* member(const std::string& key) const {
        auto it=object.find(key);
        return it==object.end()?nullptr:&it->second;
    }
    std::string stringOr(const std::string& fallback={}) const {
        return kind==Kind::text?text:fallback;
    }
    double numberOr(double fallback=0) const {
        return kind==Kind::number?number:fallback;
    }
};
struct StageEntity {
    std::string entityClass;
    std::string name;
    std::string tagName;
    ScriptValue fields;
    ScriptValue controllers;
};
class NativeStageWorld {
public:
    const std::vector<StageEntity>& entities() const noexcept {return entities_;}
    const std::vector<ScriptValue>& parameters() const noexcept {return parameters_;}
    bool flag(const std::string& scope,const std::string& key) const;
    void setFlag(const std::string& scope,const std::string& key,bool value);
    void addEntity(StageEntity entity);
    void addParameters(ScriptValue params);
    void clear();
private:
    std::vector<StageEntity> entities_;
    std::vector<ScriptValue> parameters_;
    std::map<std::string,bool> flags_;
};

// Executes Lua 5.0 SOURCE on the host CPU through an embedded Lua interpreter.
// This DOES NOT translate, interpret or emulate Sony's PS2 machine instructions.
// Direct retail .luc PS2 bytecode is rejected until conversion/architecture
// compatibility is validated; use recover_lua50.py for .lua source instead.
class LuaStage {
public:
    explicit LuaStage(NativeStageWorld& world);
    ~LuaStage();
    LuaStage(const LuaStage&)=delete;
    LuaStage& operator=(const LuaStage&)=delete;
    void executeSource(std::string_view source,const std::string& chunkName="area.lua");
    void callSetupArea(const std::string& spawnTag);
    std::string lastError() const {return error_;}
private:
    lua_State* state_ = nullptr;
    NativeStageWorld& world_;
    std::string error_;
    void registerCallback(const char* name,int (*callback)(lua_State*));
    static LuaStage* self(lua_State* L);
    static ScriptValue takeValue(lua_State* L,int stackIndex,int depth=0);
    static int newEntity(lua_State* L);
    static int luaParam(lua_State* L);
    static int control(lua_State* L);
    static int order(lua_State* L);
    static int setFlag(lua_State* L);
    static int getFlag(lua_State* L);
    static int getStageInt(lua_State* L);
    static int isSurvival(lua_State* L);
    static int costumePossess(lua_State* L);
};
} // namespace ae3
