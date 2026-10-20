#include "ae3/retail_lua50.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace ae3 {
namespace {
// Verified against 168 original SCUS-97501 area.luc scripts. The
// original Lua 5.0.3 lundump.c sequence is:
// Header, Function { source, lineDefined, nups, numparams, vararg,
// maxstacksize, lines, locals, upvalues, constants, nested functions,
// instructions }. DO NOT treat bytes as Sony EE opcodes.
class RetailReader {
public:
    explicit RetailReader(std::string_view s):data_(s){}
    std::vector<std::uint8_t> convert(RetailLua50Stats* report) {
        const auto header=take(14);
        constexpr std::array<unsigned char,14> expected{
            0x1b,0x4c,0x75,0x61,0x50,1,4,4,4,6,8,9,9,4};
        if(!std::equal(header.begin(),header.end(),expected.begin(),
            [](char a,unsigned char b){return static_cast<unsigned char>(a)==b;}))
            fail("not original Ape Escape 3 Lua 5.0 float32 bytecode");
        const float check=number();
        if(!std::isfinite(check)||static_cast<std::int64_t>(check)!=31415926)
            fail("Lua 5.0 test number does not match real retail format");
        append(header.substr(0,7));
        appendByte(8);  // Windows x64 host sizeof(size_t)
        append(header.substr(8,5));
        appendByte(8);  // standard Lua 5.0.3 host sizeof(lua_Number) = double
        appendDouble(static_cast<double>(check));
        parseFunction(0);
        if(pos_!=data_.size())fail("trailing bytes after original Lua prototype");
        if(report)*report=stats_;
        return std::move(output_);
    }
private:
    std::string_view data_;
    std::size_t pos_=0;
    std::vector<std::uint8_t> output_;
    RetailLua50Stats stats_{};
    [[noreturn]] void fail(const char* detail) const {
        throw std::runtime_error(
            "original Lua 5.0 bytecode at offset "+std::to_string(pos_)+": "+detail);
    }
    std::string_view take(std::size_t size){
        if(size>data_.size()-pos_)fail("truncated script or unreasonable field size");
        auto view=data_.substr(pos_,size);
        pos_+=size;
        return view;
    }
    std::uint8_t appendByte(std::uint8_t b){
        output_.push_back(b);
        return b;
    }
    void append(std::string_view b){
        output_.insert(output_.end(),b.begin(),b.end());
    }
    void appendU64(std::uint64_t value){
        for(int shift=0;shift<64;shift+=8)
            output_.push_back(static_cast<std::uint8_t>(value>>shift));
    }
    void appendDouble(double value){
        static_assert(sizeof(double)==8);
        std::uint64_t bits=0;
        std::memcpy(&bits,&value,8);
        appendU64(bits);
    }
    std::uint8_t readByte(){
        auto bytes=take(1);
        return appendByte(static_cast<std::uint8_t>(bytes[0]));
    }
    std::uint32_t readInt(){
        auto bytes=take(4);
        append(bytes);
        const auto v=static_cast<std::uint32_t>(
             static_cast<unsigned char>(bytes[0]) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1]))<<8) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[2]))<<16) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[3]))<<24));
        if(v>2'000'000)fail("impossible Lua function count/line index");
        return v;
    }
    float number(){
        auto bytes=take(4);
        std::uint32_t bits=static_cast<std::uint8_t>(bytes[0])|
            (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes[1]))<<8)|
            (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes[2]))<<16)|
            (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes[3]))<<24);
        float value=0;
        std::memcpy(&value,&bits,4);
        return value;
    }
    void luaString(){
        // Original Lua 5.0 serialized size_t as little-endian uint32.
        auto v=take(4);
        const std::uint64_t n=static_cast<std::uint8_t>(v[0]) |
            (static_cast<std::uint64_t>(static_cast<std::uint8_t>(v[1]))<<8) |
            (static_cast<std::uint64_t>(static_cast<std::uint8_t>(v[2]))<<16) |
            (static_cast<std::uint64_t>(static_cast<std::uint8_t>(v[3]))<<24);
        if(n>16*1024*1024)fail("original Lua string exceeds memory budget");
        appendU64(n);
        auto bytes=take(static_cast<std::size_t>(n));
        if(n && bytes.back()!=0)fail("string missing source NUL terminator");
        append(bytes);
    }
    void parseFunction(unsigned depth){
        if(depth>90||++stats_.prototypes>10000)
            fail("original Lua function nesting/count exceeds safe limit");
        luaString();             // preserved original filename
        readInt();               // lineDefined
        for(int i=0;i<4;++i)readByte(); // nups, numparams, vararg, maxstack
        auto lines=readInt();
        append(take(static_cast<std::size_t>(lines)*4));
        auto locals=readInt();
        for(std::uint32_t i=0;i<locals;++i){
            luaString();
            append(take(8)); // startpc/endpc
        }
        auto upvalues=readInt();
        for(std::uint32_t i=0;i<upvalues;++i)luaString();
        const auto count=readInt();
        if(stats_.constants+count>2'000'000)
            fail("original Lua total constant budget exceeded");
        stats_.constants+=count;
        for(std::uint32_t i=0;i<count;++i){
            const auto type=readByte();
            switch(type){
              case 0: break;       // LUA_TNIL
              case 3: {             // LUA_TNUMBER (PS2 float32)
                  const float n=number();
                  if(!std::isfinite(n))fail("nonfinite original Lua numeric constant");
                  appendDouble(static_cast<double>(n));
                  break;
              }
              case 4: luaString();break; // LUA_TSTRING
              default: fail("unexpected original Lua constant tag");
            }
        }
        const auto functions=readInt();
        for(std::uint32_t i=0;i<functions;++i)parseFunction(depth+1);
        const auto instructionCount=readInt();
        stats_.instructions+=instructionCount;
        if(stats_.instructions>2'000'000)
            fail("Lua opcode budget exceeded");
        append(take(static_cast<std::size_t>(instructionCount)*4));
        if(output_.size()>64*1024*1024)
            fail("converted Lua bytecode output exceeds max size");
    }
};
} // namespace
std::vector<std::uint8_t> retargetRetailLua50(
    std::string_view retailChunk,RetailLua50Stats* stats) {
    if(retailChunk.size()<32||retailChunk.size()>16*1024*1024)
        throw std::runtime_error("original Lua 5.0 source size invalid");
    return RetailReader(retailChunk).convert(stats);
}
} // namespace ae3
