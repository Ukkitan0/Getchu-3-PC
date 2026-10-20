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
    // Retail AE3's Lua 5.0 is still Lua, not Sony machine code.
    // Its binary dump uses float32 and 32-bit size_t; Windows x64 Lua
    // uses double and 64-bit size_t. Repack only the original storage
    // representation. Do not change or replace a single Lua opcode.
    std::vector<std::uint8_t> hostChunk;
    retailChunk_=source.size()>=4 && source.substr(0,4)==std::string_view("\x1bLua",4);
    if(retailChunk_){
        hostChunk=retargetRetailLua50(source);
        source=std::string_view(
            reinterpret_cast<const char*>(hostChunk.data()),hostChunk.size());
    }
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
    // Derived from real USA SCUS_975.01 virtual callbacks:
    // gw_set_flag @ 0x6175E8 passes the constant TRUE to 0x38545C.
    // lw_set_flag @ 0x617728 and area_set_flag @ 0x6177E4 default TRUE
    // and accept a second argument only when it is a Lua boolean.
    // The world_ flag store is still temporary native stage state;
    // retail persistence and global flag-manager semantics remain open.
    bool value=true;
    if(scope!="gw" && lua_gettop(L)>=2 && lua_type(L,2)==LUA_TBOOLEAN)
        value=lua_toboolean(L,2)!=0;
    self(L)->world_.setFlag(scope,key,value);
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
    lua_pushboolean(L,stage->world_.flag(scope,key));
    return 1;
}
int LuaStage::getStageInt(lua_State* L){
    if(self(L)->retailChunk_)
        return luaL_error(L,
            "original stage_var_get_int semantics not yet reconstructed from SCUS_975.01");
    lua_pushnumber(L,0);
    return 1;
}
int LuaStage::isSurvival(lua_State* L){
    if(self(L)->retailChunk_)
        return luaL_error(L,
            "original is_survival_mode runtime flag not initialized from retail data");
    lua_pushboolean(L,0);
    return 1;
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
    self(L)->world_.setPlayerWeaponSelection(selection);
    return 0;
}
} // namespace ae3
