#include "ae3/lua_stage.hpp"
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
    entities_.clear();parameters_.clear();flags_.clear();
}

LuaStage::LuaStage(NativeStageWorld& world):world_(world){
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
}
LuaStage::~LuaStage(){if(state_)lua_close(state_);}
void LuaStage::registerCallback(const char* name,int (*callback)(lua_State*)){
    lua_pushlightuserdata(state_,this);
    lua_pushstring(state_,name);
    lua_pushcclosure(state_,callback,2);
    lua_setglobal(state_,name);
}
LuaStage* LuaStage::self(lua_State* L){
    return static_cast<LuaStage*>(lua_touserdata(L,lua_upvalueindex(1)));
}
ScriptValue LuaStage::takeValue(lua_State* L,int index,int depth){
    ScriptValue value;
    if(depth>24)return value;
    const int kind=lua_type(L,index);
    if(kind==LUA_TBOOLEAN){
        value.kind=ScriptValue::Kind::boolean;
        value.boolean=lua_toboolean(L,index)!=0;
    }else if(kind==LUA_TNUMBER){
        value.kind=ScriptValue::Kind::number;
        value.number=lua_tonumber(L,index);
    }else if(kind==LUA_TSTRING){
        value.kind=ScriptValue::Kind::text;
        size_t n=lua_strlen(L,index);
        if(n<=65536) value.text.assign(lua_tostring(L,index),n);
    }else if(kind==LUA_TTABLE){
        int absolute=index<0?lua_gettop(L)+index+1:index;
        value.kind=ScriptValue::Kind::object;
        std::map<int,ScriptValue> numeric;
        lua_pushnil(L);
        std::size_t visited=0;
        while(lua_next(L,absolute)!=0){
            ++visited;
            if(visited<=4096){
                if(lua_type(L,-2)==LUA_TSTRING){
                    const char* key=lua_tostring(L,-2);
                    if(key && std::strlen(key)<=256)
                        value.object[key]=takeValue(L,-1,depth+1);
                }else if(lua_type(L,-2)==LUA_TNUMBER){
                    const double n=lua_tonumber(L,-2);
                    if(std::isfinite(n) && n>=1 && n<=4096 && n==std::floor(n))
                        numeric[int(n)]=takeValue(L,-1,depth+1);
                }
            }
            lua_pop(L,1); // pop value; lua_next retains key
        }
        if(value.object.empty()&&!numeric.empty()){
            value.kind=ScriptValue::Kind::array;
            value.array.resize(static_cast<std::size_t>(numeric.rbegin()->first));
            for(auto& [at,entry]:numeric)value.array[std::size_t(at-1)]=std::move(entry);
        }else if(!numeric.empty()){
            for(auto& [at,entry]:numeric)value.object[std::to_string(at)]=std::move(entry);
        }
    }
    return value;
}
void LuaStage::executeSource(std::string_view source,const std::string& chunkName){
    if(source.size()>16*1024*1024)throw std::runtime_error("Lua stage source exceeds native load limit");
    if(source.size()>=4 && source[0]=='\x1b' && source.substr(1,3)=="Lua")
        throw std::runtime_error("original PS2 Lua bytecode requires verified conversion to Lua source");
    error_.clear();
    int status=luaL_loadbuffer(state_,source.data(),source.size(),chunkName.c_str());
    if(status==0)status=lua_pcall(state_,0,0,0);
    if(status){
        const char* what=lua_tostring(state_,-1);
        error_=what?what:"unknown Lua 5.0 error";
        lua_pop(state_,1);
        throw std::runtime_error("native Lua 5.0 stage failed: "+error_);
    }
}
void LuaStage::callSetupArea(const std::string& spawnTag){
    lua_getglobal(state_,"setupArea");
    if(!lua_isfunction(state_,-1)){
        lua_pop(state_,1);
        throw std::runtime_error("area.lua did not define setupArea");
    }
    lua_newtable(state_);
    lua_pushstring(state_,"spawn_tag");
    lua_pushlstring(state_,spawnTag.data(),spawnTag.size());
    lua_settable(state_,-3);
    const int status=lua_pcall(state_,1,0,0);
    if(status){
        const char* msg=lua_tostring(state_,-1);
        error_=msg?msg:"unknown setupArea error";
        lua_pop(state_,1);
        throw std::runtime_error("native setupArea failed: "+error_);
    }
}
int LuaStage::newEntity(lua_State* L){
    auto* game=self(L);
    const char* type=luaL_checkstring(L,1);
    luaL_checktype(L,2,LUA_TTABLE);
    ScriptValue fields=takeValue(L,2);
    const ScriptValue* name=fields.member("name");
    const ScriptValue* tag=fields.member("tag_name");
    StageEntity entry;
    entry.entityClass=type;
    if(name)entry.name=name->stringOr();
    if(tag)entry.tagName=tag->stringOr();
    entry.fields=std::move(fields);
    if(lua_gettop(L)>=3)entry.controllers=takeValue(L,3);
    // Native entity descriptor only; asset loading and AI are separate stages.
    game->world_.addEntity(std::move(entry));
    lua_pushnumber(L,static_cast<double>(game->world_.entities().size()));
    return 1;
}
int LuaStage::luaParam(lua_State* L){
    luaL_checktype(L,1,LUA_TTABLE);
    self(L)->world_.addParameters(takeValue(L,1));
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
    self(L)->world_.setFlag(scope,key,lua_toboolean(L,2)!=0);
    return 0;
}
int LuaStage::getFlag(lua_State* L){
    const char* key=luaL_checkstring(L,1);
    const char* binding=lua_tostring(L,lua_upvalueindex(2));
    std::string scope=binding?binding:"area_get_flag";
    scope=scope.substr(0,scope.find('_'));
    lua_pushboolean(L,self(L)->world_.flag(scope,key));
    return 1;
}
int LuaStage::getStageInt(lua_State* L){ (void)L;lua_pushnumber(L,0);return 1; }
int LuaStage::isSurvival(lua_State* L){lua_pushboolean(L,0);return 1;}
int LuaStage::costumePossess(lua_State* L){lua_pushboolean(L,0);return 1;}
} // namespace ae3
