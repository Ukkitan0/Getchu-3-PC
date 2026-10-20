#include "ae3/retail_lua50.hpp"
#include "ae3/lua_stage.hpp"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
void check(bool yes,const char* what){if(!yes)throw std::runtime_error(what);}
int collect(lua_State*,const void* source,std::size_t size,void* user){
    auto& bytes=*static_cast<std::vector<std::uint8_t>*>(user);
    const auto* p=static_cast<const std::uint8_t*>(source);
    bytes.insert(bytes.end(),p,p+size);
    return 0;
}
class HostToRetail {
public:
    explicit HostToRetail(const std::vector<std::uint8_t>& bytes):src_(bytes){}
    std::vector<std::uint8_t> convert(){
        auto h=take(14);
        check(std::memcmp(h,"\x1bLua",4)==0,
              "host binary was not Lua 5.0");
        check(h[4]==0x50 && h[5]==1 && h[6]==4 && h[7]==8 &&
              h[8]==4 && h[9]==6 && h[10]==8 && h[11]==9 &&
              h[12]==9 && h[13]==8,
              "host Lua CMake compilation uses unexpected bytecode header");
        for(int k=0;k<14;++k)
            output_.push_back((k==7||k==13)?4:h[k]);
        float test=static_cast<float>(number());
        append(&test,4);
        function(0);
        check(at_==src_.size(),"host Lua 5.0 dump was not fully consumed");
        return output_;
    }
private:
    const std::vector<std::uint8_t>& src_;
    std::size_t at_=0;
    std::vector<std::uint8_t> output_;
    const std::uint8_t* take(std::size_t n){
        if(n>src_.size()-at_)throw std::runtime_error("host Lua dump bounds");
        const auto* p=src_.data()+at_;
        at_+=n;
        return p;
    }
    void append(const void* p,std::size_t n){
        const auto* ptr=static_cast<const std::uint8_t*>(p);
        output_.insert(output_.end(),ptr,ptr+n);
    }
    void copy(std::size_t n){append(take(n),n);}
    std::uint8_t byte(){
        const auto v=*take(1);
        output_.push_back(v);
        return v;
    }
    std::uint32_t int32(){
        auto p=take(4);std::uint32_t v=0;std::memcpy(&v,p,4);
        if(v>1000000)throw std::runtime_error("native Lua dump count budget");
        append(p,4);
        return v;
    }
    double number(){
        double value;std::memcpy(&value,take(8),8);
        return value;
    }
    void string(){
        std::uint64_t len=0;
        std::memcpy(&len,take(8),8);
        if(len>1<<20)throw std::runtime_error("unreasonable test string length");
        const auto length=static_cast<std::uint32_t>(len);
        append(&length,4);
        copy(static_cast<std::size_t>(len));
    }
    void function(unsigned depth){
        if(depth>64)throw std::runtime_error("host Lua nesting too deep");
        string();
        int32();
        for(int i=0;i<4;++i)byte();
        copy(static_cast<std::size_t>(int32())*4);
        auto locals=int32();
        for(std::uint32_t i=0;i<locals;++i){
            string();copy(8);
        }
        auto upvals=int32();
        for(std::uint32_t i=0;i<upvals;++i)string();
        auto constants=int32();
        for(std::uint32_t i=0;i<constants;++i){
            const auto tag=byte();
            if(tag==3){
                const float n=static_cast<float>(number());
                append(&n,4);
            }else if(tag==4)string();
            else if(tag!=0)throw std::runtime_error("unexpected Lua constant in test");
        }
        const auto functions=int32();
        for(std::uint32_t i=0;i<functions;++i)function(depth+1);
        copy(static_cast<std::size_t>(int32())*4);
    }
};
void testRetailBytecodeExecution(){
    lua_State* compiler=lua_open();
    check(compiler!=nullptr,"native Lua5.0 test compile state missing");
    const char source[]=
        "script_description='original_lua_retail_test'\n"
        "function setupArea(opts)\n"
        " new_ent('Player',{name='keiyumi',pos={1.5,2,3}})\n"
        " new_ent('Monkey',{name='ape_01',tag_name='mark_a'})\n"
        "end\n";
    const int load=luaL_loadbuffer(compiler,source,sizeof(source)-1,"@area.lua");
    if(load){
        lua_close(compiler);
        throw std::runtime_error("native Lua5.0 test source compilation failed");
    }
    std::vector<std::uint8_t> nativeChunk;
    const int dumped=lua_dump(compiler,collect,&nativeChunk);
    lua_close(compiler);
    check(dumped==0 && nativeChunk.size()>100,
          "native Lua5.0 binary compiler produced no real machine-independent Lua bytecode");
    auto originalFormat=HostToRetail(nativeChunk).convert();
    ae3::RetailLua50Stats report;
    const auto host=ae3::retargetRetailLua50(std::string_view(
       reinterpret_cast<const char*>(originalFormat.data()),
       originalFormat.size()),&report);
    check(host==nativeChunk,"original float32 and native double64 Lua50 representation differed");
    check(report.prototypes>=2 && report.instructions>10,
          "Lua50 function/opcode structure was not retained during retarget");
    ae3::NativeStageWorld game;
    ae3::LuaStage lua(game);
    lua.executeSource(std::string_view(
        reinterpret_cast<const char*>(originalFormat.data()),
        originalFormat.size()),"@REAL_original_area.luc");
    lua.callSetupArea("spawn_default");
    check(game.entities().size()==2,
          "retargeted retail Lua50 did not actually execute on native Lua50");
    check(game.entities()[0].entityClass=="Player" &&
          game.entities()[1].entityClass=="Monkey",
          "original Lua native script types were lost");
    const auto* pos=game.entities()[0].fields.member("pos");
    check(pos && pos->array.size()==3 &&
          std::abs(pos->array[0].number-1.5)<1e-7,
          "native Lua50 numerical argument changed during original PS2 float conversion");
    auto broken=originalFormat;
    broken[13]=7;
    bool rejected=false;
    try{ae3::retargetRetailLua50(std::string_view(
        reinterpret_cast<const char*>(broken.data()),broken.size()));}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"unknown PS2 Lua header was allowed");
    broken=originalFormat;
    broken.resize(originalFormat.size()-2);
    rejected=false;
    try{ae3::retargetRetailLua50(std::string_view(
        reinterpret_cast<const char*>(broken.data()),broken.size()));}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"truncated original Lua game script accepted");
}
}
int main(){
    try{
        testRetailBytecodeExecution();
        std::cout<<"RETAIL_AE3_LUA50_NATIVE_EXECUTION_PASS "
                    "original_32bit_number=1 original_size_t32=1 "
                    "original_lua_opcode_identity=1 native_execution=1\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"RETAIL_AE3_LUA50_NATIVE_EXECUTION_FAILED: "
                 <<error.what()<<"\n";
        return 1;
    }
}
