#include "ae3/lua_stage.hpp"
#include "ae3/retail_lua50.hpp"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace ae3 {
bool NativeStageWorld::flag(const std::string& scope,const std::string& key) const {
    auto it=flags_.find(scope+":"+key);
    return it!=flags_.end() && it->second;
}
void NativeStageWorld::setFlag(const std::string& scope,const std::string& key,bool value){
    flags_[scope+":"+key]=value;
}
void NativeStageWorld::addEntity(StageEntity entity){
    if(entities_.size()>=65536)throw std::runtime_error("native stage entity limit reached");
    entities_.push_back(std::move(entity));
}
void NativeStageWorld::addParameters(ScriptValue params){
    if(parameters_.size()>=65536)throw std::runtime_error("native stage parameter limit reached");
    parameters_.push_back(std::move(params));
}
void NativeStageWorld::clear(){
    entities_.clear();parameters_.clear();flags_.clear();playerWeaponSelection_.reset();
}

LuaStage::LuaStage(NativeStageWorld& world,const PlayerProgress* progress)
    :world_(world),progress_(progress){
    state_=lua_open();
    if(!state_)throw std::runtime_error("native Lua 5.0 state allocation failed");
    // No IO, OS, debug or external-module libraries in game stage environment.
    // Original scripts get math/table/string and safe baseline Lua syntax.
    luaopen_base(state_);
    luaopen_table(state_);
    luaopen_string(state_);
    luaopen_math(state_);
    // Disable host-source execution/file access through core base functions.
    for(const char* name:{"dofile","loadfile","loadstring","load","require",
                           "loadlib","collectgarbage","_ALERT"}){
        lua_pushnil(state_);
        lua_setglobal(state_,name);
    }
    registerCallback("new_ent",&newEntity);
    registerCallback("lua_param",&luaParam);
    registerCallback("c_ctrl",&control);
    registerCallback("c_order",&order);
    registerCallback("gw_set_flag",&setFlag);
    registerCallback("lw_set_flag",&setFlag);
    registerCallback("area_set_flag",&setFlag);
    registerCallback("gw_get_flag",&getFlag);
    registerCallback("lw_get_flag",&getFlag);
    registerCallback("area_get_flag",&getFlag);
    registerCallback("get_capture_flag",&getFlag);
    registerCallback("stage_var_get_int",&getStageInt);
    registerCallback("is_survival_mode",&isSurvival);
    registerCallback("get_costume_possess",&costumePossess);
    registerCallback("get_costume_possess_ninja",&costumePossessNinja);
    registerCallback("get_sex_distinction",&sexDistinction);
    registerCallback("set_player_weapon",&setPlayerWeapon);
}
LuaStage::~LuaStage(){if(state_)lua_close(state_);}
void LuaStage::setNativeBindings(NativeLuaStageBindings bindings){
    nativeBindings_=std::move(bindings);
}
void LuaStage::registerOriginalFunction(std::string name,std::uint32_t originalVA,
                                       OriginalLuaNativeCallback callback){
    if(name.empty() || name.size()>128 || !callback || originalVA==0 ||
       (originalVA&3u)!=0)
        throw std::invalid_argument("verified original Lua function needs a name, aligned retail VA and implementation");
    if(originalFunctions_.count(name))
        throw std::invalid_argument("original Lua function registered twice: "+name);
    lua_getglobal(state_,name.c_str());
    const bool collision=!lua_isnil(state_,-1);
    lua_pop(state_,1);
    if(collision)
        throw std::invalid_argument("refusing to overwrite an existing Lua global: "+name);
    const std::string key=name;
    originalFunctions_.emplace(std::move(name),RegisteredOriginal{originalVA,std::move(callback)});
    registerCallback(key.c_str(),&dispatchOriginal);
}
void LuaStage::registerOriginalRawFunction(std::string name,std::uint32_t originalVA,
                                          OriginalLuaRawCallback callback){
    if(name.empty() || name.size()>128 || !callback || originalVA==0 ||
       (originalVA&3u)!=0)
        throw std::invalid_argument(
            "original raw Lua callback needs a name, aligned source VA and implementation");
    if(originalFunctions_.count(name))
        throw std::invalid_argument("original Lua function registered twice: "+name);
    lua_getglobal(state_,name.c_str());
    const bool collision=!lua_isnil(state_,-1);
    lua_pop(state_,1);
    if(collision)
        throw std::invalid_argument("refusing to overwrite an existing Lua global: "+name);
    const std::string key=name;
    originalFunctions_.emplace(std::move(name),
        RegisteredOriginal{originalVA,{},std::move(callback)});
    registerCallback(key.c_str(),&dispatchOriginal);
}
void LuaStage::pushValue(lua_State* L,const ScriptValue& value,int depth){
    if(depth>24)throw std::runtime_error("native Lua result exceeds table nesting limit");
    switch(value.kind){
    case ScriptValue::Kind::nil:lua_pushnil(L);break;
    case ScriptValue::Kind::boolean:lua_pushboolean(L,value.boolean);break;
    case ScriptValue::Kind::number:
        if(!std::isfinite(value.number))
            throw std::runtime_error("native Lua result contains nonfinite number");
        lua_pushnumber(L,value.number);break;
    case ScriptValue::Kind::text:
        lua_pushlstring(L,value.text.data(),value.text.size());break;
    case ScriptValue::Kind::object:
        lua_newtable(L);
        for(const auto& [key,child]:value.object){
            lua_pushlstring(L,key.data(),key.size());
            pushValue(L,child,depth+1);
            lua_settable(L,-3);
        }
        break;
    case ScriptValue::Kind::array:
        lua_newtable(L);
        for(std::size_t i=0;i<value.array.size();++i){
            pushValue(L,value.array[i],depth+1);
            lua_rawseti(L,-2,static_cast<int>(i+1));
        }
        break;
    }
}
int LuaStage::dispatchOriginal(lua_State* L){
    auto* stage=self(L);
    const char* name=lua_tostring(L,lua_upvalueindex(2));
    const auto it=stage->originalFunctions_.find(name?name:"");
    if(it==stage->originalFunctions_.end())
        return luaL_error(L,"unregistered original Lua native callback");
    try{
        const int count=lua_gettop(L);
        if(count>64)throw std::runtime_error("too many original Lua call arguments");
        if(it->second.rawCallback){
            // The full original Lua stack is supplied unchanged. No table
            // coercion, copied closure or fake game-object handle occurs.
            const int results=it->second.rawCallback(L);
            if(results<0 || results>64 || results>lua_gettop(L))
                throw std::runtime_error("original raw Lua binding returned invalid result count");
            return results;
        }
        std::vector<ScriptValue> arguments;
        arguments.reserve(static_cast<std::size_t>(count));
        for(int i=1;i<=count;++i){
            const int kind=lua_type(L,i);
            if(kind!=LUA_TNIL && kind!=LUA_TBOOLEAN && kind!=LUA_TNUMBER &&
               kind!=LUA_TSTRING && kind!=LUA_TTABLE)
                throw std::runtime_error("unsupported original Lua argument type; no substitute conversion");
            arguments.push_back(takeValue(L,i));
        }
        auto results=it->second.callback(arguments);
        if(results.size()>64)throw std::runtime_error("too many original Lua results");
        for(const auto& result:results)pushValue(L,result);
        return static_cast<int>(results.size());
    }catch(const std::exception& error){
        // Lua 5.0 lua_pushvfstring (used by luaL_error) does NOT implement
        // C printf width/hex specifiers such as %08X. Keep the source VA
        // in decimal so this error remains safe under the real Lua 5.0 VM.
        return luaL_error(L,"original Lua binding %s at VA %d failed: %s",
                          name,static_cast<int>(it->second.originalVA),error.what());
    }
}
bool LuaStage::invokeCallback(const std::string& name,
                              const std::vector<ScriptValue>& arguments){
    if(name.empty() || name.size()>128 || arguments.size()>64)
        throw std::invalid_argument("invalid original Lua callback name or argument count");
    error_.clear();
    const int initialTop=lua_gettop(state_);
    lua_getglobal(state_,name.c_str());
    if(lua_isnil(state_,-1)){
        lua_pop(state_,1);
        return false;
    }
    if(!lua_isfunction(state_,-1)){
        lua_pop(state_,1);
        throw std::runtime_error("original Lua callback "+name+" is not a function");
    }
    try{
        for(const auto& argument:arguments)pushValue(state_,argument);
    }catch(...){
        // Do not leave a half-built table/function on Lua's stack when
        // the host rejects an unsupported result representation.
        lua_settop(state_,initialTop);
        throw;
    }
    const int status=lua_pcall(state_,static_cast<int>(arguments.size()),0,0);
    if(status){
        const char* message=lua_tostring(state_,-1);
        error_=message?message:"unknown original Lua callback error";
        lua_pop(state_,1);
        throw std::runtime_error("original Lua callback "+name+" failed: "+error_);
    }
    return true;
}
void LuaStage::registerCallback(const char* name,int (*callback)(lua_State*)){
    lua_pushlightuserdata(state_,this);
    lua_pushstring(state_,name);
    lua_pushcclosure(state_,callback,2);
    lua_setglobal(state_,name);
}
LuaStage* LuaStage::self(lua_State* L){
    return static_cast<LuaStage*>(lua_touserdata(L,lua_upvalueindex(1)));
}
ScriptValue LuaStage::takeValue(lua_State* L,int index,int depth,
                                std::vector<const void*>* activeTables,
                                std::size_t* remainingNodes){
    // Unlike a descriptor preview, a real engine binding must never
    // silently drop or rewrite original Lua arguments. Reject any shape
    // the ScriptValue transport cannot represent losslessly.
    if(depth>24)
        throw std::runtime_error("original Lua argument exceeds table nesting limit");
    std::size_t rootBudget=65536;
    if(!remainingNodes)remainingNodes=&rootBudget;
    if(*remainingNodes==0)
        throw std::runtime_error("original Lua argument exceeds node budget");
    --*remainingNodes;
    ScriptValue value;
    const int kind=lua_type(L,index);
    if(kind==LUA_TNIL){
        return value;
    }else if(kind==LUA_TBOOLEAN){
        value.kind=ScriptValue::Kind::boolean;
        value.boolean=lua_toboolean(L,index)!=0;
    }else if(kind==LUA_TNUMBER){
        value.kind=ScriptValue::Kind::number;
        value.number=lua_tonumber(L,index);
        if(!std::isfinite(value.number))
            throw std::runtime_error("nonfinite original Lua numeric argument");
    }else if(kind==LUA_TSTRING){
        const std::size_t n=lua_strlen(L,index);
        if(n>65536)
            throw std::runtime_error("original Lua string argument exceeds transport budget");
        value.kind=ScriptValue::Kind::text;
        value.text.assign(lua_tostring(L,index),n);
    }else if(kind==LUA_TTABLE){
        const int absolute=index<0?lua_gettop(L)+index+1:index;
        const void* identity=lua_topointer(L,absolute);
        std::vector<const void*> rootActive;
        if(!activeTables)activeTables=&rootActive;
        if(std::find(activeTables->begin(),activeTables->end(),identity)!=activeTables->end())
            throw std::runtime_error("cyclic original Lua table cannot be copied into ScriptValue");
        activeTables->push_back(identity);
        // The C++ stack unwinds this guard before the caller reports the
        // exception to Lua. No stale pointer remains in shared recursion state.
        struct PopIdentity {
            std::vector<const void*>& active;
            ~PopIdentity(){active.pop_back();}
        } pop{*activeTables};
        value.kind=ScriptValue::Kind::object;
        std::map<int,ScriptValue> numeric;
        lua_pushnil(L);
        std::size_t visited=0;
        while(lua_next(L,absolute)!=0){
            if(++visited>4096)
                throw std::runtime_error("original Lua table entry limit exceeded");
            const int keyType=lua_type(L,-2);
            if(keyType==LUA_TSTRING){
                const std::size_t n=lua_strlen(L,-2);
                if(n>256)
                    throw std::runtime_error("original Lua table key exceeds transport budget");
                const char* key=lua_tostring(L,-2);
                value.object[std::string(key,n)]=
                    takeValue(L,-1,depth+1,activeTables,remainingNodes);
            }else if(keyType==LUA_TNUMBER){
                const double n=lua_tonumber(L,-2);
                if(!std::isfinite(n) || n<1 || n>4096 || n!=std::floor(n))
                    throw std::runtime_error(
                        "original Lua numeric table key cannot be represented faithfully");
                numeric[static_cast<int>(n)]=
                    takeValue(L,-1,depth+1,activeTables,remainingNodes);
            }else{
                throw std::runtime_error(
                    "unsupported original Lua table key; use a raw Lua C API binding");
            }
            lua_pop(L,1);
        }
        if(value.object.empty()&&!numeric.empty()){
            value.kind=ScriptValue::Kind::array;
            value.array.resize(static_cast<std::size_t>(numeric.rbegin()->first));
            for(auto& [at,entry]:numeric)
                value.array[static_cast<std::size_t>(at-1)]=std::move(entry);
        }else if(!numeric.empty()){
            // ScriptValue has either a numeric array OR a string-keyed map.
            // Converting numbers into string keys would change original
            // table indexing/identity, even without a direct collision.
            throw std::runtime_error(
                "mixed-key original Lua table requires a raw Lua C API binding");
        }
    }else{
        throw std::runtime_error(
            "unsupported original Lua value (userdata/function/thread); raw Lua binding required");
    }
    return value;
}
void LuaStage::executeSource(std::string_view source,const std::string& chunkName){
    if(source.size()>16*1024*1024)throw std::runtime_error("Lua stage source exceeds native load limit");
    // Retail AE3's Lua 5.0 is still Lua, not Sony machine code.
    // Its binary dump uses float32 and 32-bit size_t; Windows x64 Lua
    // uses double and 64-bit size_t. Repack only the original storage
    // representation. Do not change or replace a single Lua opcode.
    std::vector<std::uint8_t> hostChunk;
    const bool binaryChunk=source.size()>=4 &&
        source.substr(0,4)==std::string_view("\x1bLua",4);
    // A stage may load several Lua chunks. Only a successfully validated
    // ORIGINAL retail header enables strict data-provider requirements.
    // A malformed/untrusted chunk must not poison a reusable interpreter.
    if(binaryChunk){
        hostChunk=retargetRetailLua50(source);
        source=std::string_view(
            reinterpret_cast<const char*>(hostChunk.data()),hostChunk.size());
    }
    error_.clear();
    const int initialTop=lua_gettop(state_);
    int status=luaL_loadbuffer(state_,source.data(),source.size(),chunkName.c_str());
    if(status==0){
        // Format validation alone does not establish an executable retail
        // script: Lua 5.0's undumper/bytecode verifier must also accept it.
        // Activate authentic manager requirements before executing the
        // first instruction, and retain them across later source chunks.
        if(binaryChunk)retailChunk_=true;
        status=lua_pcall(state_,0,0,0);
    }
    if(status){
        const char* what=lua_tostring(state_,-1);
        error_=what?what:"unknown Lua 5.0 error";
        lua_pop(state_,1);
        throw std::runtime_error("native Lua 5.0 stage failed: "+error_);
    }
}
void LuaStage::callSetupArea(const std::string& spawnTag){
    ScriptValue tag;
    tag.kind=ScriptValue::Kind::text;
    tag.text=spawnTag;
    ScriptValue options;
    options.kind=ScriptValue::Kind::object;
    options.object.emplace("spawn_tag",std::move(tag));
    if(!invokeCallback("setupArea",{options}))
        throw std::runtime_error("area.lua did not define setupArea");
}
int LuaStage::newEntity(lua_State* L){
    auto* game=self(L);
    const char* type=luaL_checkstring(L,1);
    luaL_checktype(L,2,LUA_TTABLE);
    // Native entity descriptor only; asset loading and AI are separate stages.
    try{
        ScriptValue fields=takeValue(L,2);
        const ScriptValue* name=fields.member("name");
        const ScriptValue* tag=fields.member("tag_name");
        StageEntity entry;
        entry.entityClass=type;
        if(name)entry.name=name->stringOr();
        if(tag)entry.tagName=tag->stringOr();
        entry.fields=std::move(fields);
        if(lua_gettop(L)>=3)entry.controllers=takeValue(L,3);
        // This callback is the injection point for a recovered actor manager.
        // Do not substitute a made-up actor factory when it is unavailable.
        if(game->nativeBindings_.createEntity)
            game->nativeBindings_.createEntity(entry);
        game->world_.addEntity(std::move(entry));
    }catch(const std::exception& error){
        return luaL_error(L,"native new_ent binding failed: %s",error.what());
    }
    lua_pushnumber(L,static_cast<double>(game->world_.entities().size()));
    return 1;
}
int LuaStage::luaParam(lua_State* L){
    luaL_checktype(L,1,LUA_TTABLE);
    auto* stage=self(L);
    try{
        ScriptValue parameters=takeValue(L,1);
        if(stage->nativeBindings_.submitParameters)
            stage->nativeBindings_.submitParameters(parameters);
        stage->world_.addParameters(std::move(parameters));
    }catch(const std::exception& error){
        return luaL_error(L,"native lua_param binding failed: %s",error.what());
    }
    return 0;
}
static void field(lua_State* L,const char* name){
    lua_pushstring(L,name);lua_insert(L,-2);lua_settable(L,-3);
}
int LuaStage::control(lua_State* L){
    const char* op=luaL_checkstring(L,1);
    lua_newtable(L);
    lua_pushstring(L,"control");field(L,"kind");
    lua_pushstring(L,op);field(L,"opcode");
    if(lua_istable(L,2)){lua_pushvalue(L,2);field(L,"params");}
    return 1;
}
int LuaStage::order(lua_State* L){
    const int argc=lua_gettop(L);
    const double repeat=luaL_checknumber(L,1);
    lua_newtable(L);
    lua_pushstring(L,"order");field(L,"kind");
    lua_pushnumber(L,repeat);field(L,"repeat");
    lua_newtable(L);
    for(int i=2;i<=argc;++i){
        lua_pushvalue(L,i);
        lua_rawseti(L,-2,i-1);
    }
    field(L,"children");
    return 1;
}
int LuaStage::setFlag(lua_State* L){
    const char* key=luaL_checkstring(L,1);
    const char* binding=lua_tostring(L,lua_upvalueindex(2));
    std::string scope=binding?binding:"area_set_flag";
    scope=scope.substr(0,scope.find('_'));
    // Derived from real USA SCUS_975.01 virtual callbacks:
    // gw_set_flag @ 0x6175E8 passes the constant TRUE to 0x38545C.
    // lw_set_flag @ 0x617728 and area_set_flag @ 0x6177E4 default TRUE
    // and accept a second argument only when it is a Lua boolean.
    // The world_ flag store is still temporary native stage state;
    // retail persistence and global flag-manager semantics remain open.
    bool value=true;
    if(scope!="gw" && lua_gettop(L)>=2 && lua_type(L,2)==LUA_TBOOLEAN)
        value=lua_toboolean(L,2)!=0;
    auto* stage=self(L);
    if(stage->retailChunk_ && !stage->nativeBindings_.writeFlag)
        return luaL_error(L,
            "original %s requires real game flag-manager write binding",binding);
    try{
        if(stage->nativeBindings_.writeFlag)
            stage->nativeBindings_.writeFlag(scope,key,value);
        stage->world_.setFlag(scope,key,value);
    }catch(const std::exception& error){
        return luaL_error(L,"original %s failed: %s",binding,error.what());
    }
    return 0;
}
int LuaStage::getFlag(lua_State* L){
    auto* stage=self(L);
    const char* binding=lua_tostring(L,lua_upvalueindex(2));
    const std::string name=binding?binding:"";
    if(name=="get_capture_flag"){
        // Original game Data::capture_flag[570] at offset 204,
        // verified by recovered save-data decompilation.
        // Never invent a captured-monkey state for a retail script.
        const double n=luaL_checknumber(L,1);
        if(!std::isfinite(n)||n<0||n>=570||n!=std::floor(n))
            return luaL_error(L,"original capture flag index outside 0..569");
        if(!stage->progress_){
            if(stage->retailChunk_)
                return luaL_error(L,"get_capture_flag needs actual native PlayerProgress/new game data");
            lua_pushboolean(L,0);
        }else{
            const auto index=static_cast<std::size_t>(n);
            lua_pushboolean(L,stage->progress_->capture_flag[index]!=0);
        }
        return 1;
    }
    const char* key=luaL_checkstring(L,1);
    const std::string scope=name.substr(0,name.find('_'));
    try{
        if(stage->nativeBindings_.readFlag){
            const auto value=stage->nativeBindings_.readFlag(scope,key);
            if(!value.has_value())
                return luaL_error(L,"original %s: game flag value unavailable",name.c_str());
            lua_pushboolean(L,*value);
        }else{
            if(stage->retailChunk_)
                return luaL_error(L,"original %s needs real game flag-manager read binding",
                                  name.c_str());
            lua_pushboolean(L,stage->world_.flag(scope,key));
        }
        return 1;
    }catch(const std::exception& error){
        return luaL_error(L,"original %s failed: %s",name.c_str(),error.what());
    }
}
int LuaStage::getStageInt(lua_State* L){
    auto* stage=self(L);
    if(!stage->nativeBindings_.readStageInteger)
        return luaL_error(L,"original stage_var_get_int has no recovered native provider");
    try{
        std::vector<ScriptValue> arguments;
        const int count=lua_gettop(L);
        if(count>64)return luaL_error(L,"original stage_var_get_int argument budget exceeded");
        for(int i=1;i<=count;++i)arguments.push_back(takeValue(L,i));
        const auto value=stage->nativeBindings_.readStageInteger(arguments);
        if(!value)
            return luaL_error(L,"original stage_var_get_int value unavailable");
        lua_pushnumber(L,static_cast<double>(*value));
        return 1;
    }catch(const std::exception& error){
        return luaL_error(L,"original stage_var_get_int failed: %s",error.what());
    }
}
int LuaStage::isSurvival(lua_State* L){
    auto* stage=self(L);
    if(!stage->nativeBindings_.readSurvival)
        return luaL_error(L,"original is_survival_mode has no recovered native provider");
    try{
        const auto value=stage->nativeBindings_.readSurvival();
        if(!value)
            return luaL_error(L,"original is_survival_mode state unavailable");
        lua_pushboolean(L,*value);
        return 1;
    }catch(const std::exception& error){
        return luaL_error(L,"original is_survival_mode failed: %s",error.what());
    }
}
int LuaStage::costumePossess(lua_State* L){
    auto* stage=self(L);
    if(!stage->progress_)
        return luaL_error(L,"get_costume_possess requires original Data save state");
    // USA ELF callback 0x617924 reads [0x649948] and tests == 2.
    // Original Data base is 0x649910: +0x38 is costume_possess[7].
    // The original callback does not read a Lua argument.
    lua_pushboolean(L,stage->progress_->costume_possess[7]==2);
    return 1;
}
int LuaStage::costumePossessNinja(lua_State* L){
    auto* stage=self(L);
    if(!stage->progress_)
        return luaL_error(L,"get_costume_possess_ninja requires original Data save state");
    // USA ELF callback 0x617958 reads [0x649938] and tests == 2.
    // Original Data+0x28 is costume_possess[3]; no Lua argument.
    lua_pushboolean(L,stage->progress_->costume_possess[3]==2);
    return 1;
}
int LuaStage::sexDistinction(lua_State* L){
    auto* stage=self(L);
    if(!stage->progress_)
        return luaL_error(L,"original get_sex_distinction requires Data save state");
    // SCUS_975.01 @ 0x219C8C: load Data.player_type at 0x649910,
    // then push exact original string "boy" (0), "girl" (1), or "nil"
    // (all other values). The last is a STRING, not the Lua nil value.
    const int kind=stage->progress_->player_type;
    const char* value=kind==0?"boy":kind==1?"girl":"nil";
    lua_pushstring(L,value);
    return 1;
}
int LuaStage::setPlayerWeapon(lua_State* L){
    // SCUS_975.01 @ 0x512000 compares arg one against original strings
    // "empty" (0), "net" (1), "club" (2), default (3), then calls
    // 0x266CF4 which stores the value in the game-owned object at
    // [0x6496D8]+0xBC. The original callback returns zero Lua values.
    const char* weapon=luaL_checkstring(L,1);
    int selection=3;
    if(std::strcmp(weapon,"empty")==0)selection=0;
    else if(std::strcmp(weapon,"net")==0)selection=1;
    else if(std::strcmp(weapon,"club")==0)selection=2;
    // Native temporary state, not a fabricated active gadget or player.
    auto* stage=self(L);
    try{
        if(stage->nativeBindings_.selectWeapon)
            stage->nativeBindings_.selectWeapon(selection);
        stage->world_.setPlayerWeaponSelection(selection);
    }catch(const std::exception& error){
        return luaL_error(L,"original set_player_weapon failed: %s",error.what());
    }
    return 0;
}
} // namespace ae3
