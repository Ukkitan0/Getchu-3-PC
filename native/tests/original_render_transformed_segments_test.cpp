#include "ae3/original_render_transformed_segments.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
namespace {
void expect(bool c,const char* msg){if(!c)throw std::runtime_error(msg);}
void floatStore(std::vector<std::uint8_t>& b,std::size_t offset,float v){
    auto u=std::bit_cast<std::uint32_t>(v);
    for(unsigned j=0;j<4;++j)b[offset+j]=std::uint8_t(u>>(8*j));
}
float floatLoad(const std::vector<std::uint8_t>& b,std::size_t offset){
    std::uint32_t u=0;
    for(unsigned j=0;j<4;++j)u|=std::uint32_t(b[offset+j])<<(8*j);
    return std::bit_cast<float>(u);
}
void run(){
    auto a=originalTransformGroupLayout(0);
    auto b=originalTransformGroupLayout(2);
    auto c=originalTransformGroupLayout(3);
    expect(a.destinationCountOffset==0x10&&a.destinationDataOffset==0x14&&
           a.bytesPerRecord==32&&a.allocatorUnitsPerRecord==2,"group0 original packet record layout wrong");
    expect(b.destinationCountOffset==0x20&&b.destinationDataOffset==0x24&&
           b.bytesPerRecord==64&&b.allocatorUnitsPerRecord==4,"group2 original packet record layout wrong");
    expect(c.destinationCountOffset==0x28&&c.destinationDataOffset==0x2c&&
           c.bytesPerRecord==48&&c.allocatorUnitsPerRecord==3,"group3 original packet record layout wrong");
    OriginalTransformDependencies f;
    std::string calls;
    f.originalVuTransform=[&](std::uint8_t kind,std::uint32_t index,
                              std::span<const std::uint8_t> src,std::span<std::uint8_t> out){
        calls+=std::to_string(int(kind))+":"+std::to_string(index)+";";
        expect(src.size()==out.size(),"original VU quadword transfer source size wrong");
        for(std::size_t i=0;i<src.size();++i)out[i]=src[i];
    };
    std::vector<std::uint8_t> input(128);
    for(std::size_t i=0;i<input.size();++i)input[i]=std::uint8_t(i*71);
    auto zero=originalTransformPacketSegment({0x3AF478,0,0,2},input,f);
    expect(zero.bytes.size()==64&&zero.originalRecordCount==2&&
           calls=="0:0;0:1;","group0 source 32-byte stride/VU callback sequence wrong");
    expect(zero.bytes[0]==input[0]&&zero.bytes[63]==input[63],"unmodified VU callback staging failed");
    calls.clear();
    auto three=originalTransformPacketSegment({0x3AF530,3,0,2},input,f);
    expect(three.bytes.size()==96&&calls=="3:0;3:1;",
           "group3 source 48-byte stride/VU callback sequence wrong");
    calls.clear();
    // Unit-test math functions are controlled stubs, never game substitutions.
    floatStore(input,0x30,1.0f);
    floatStore(input,0x34,2.0f);
    std::vector<float> sineArgs,cosArgs;
    f.retailFunction41d3d8=[&](float x){sineArgs.push_back(x);return x*0.5f;};
    f.retailFunction41d490=[&](float x){cosArgs.push_back(x);return 2.0f+x;};
    auto two=originalTransformPacketSegment({0x3AF6F8,2,0,1},input,f);
    const float clamp=1.5697963237762451f;
    expect(two.bytes.size()==64&&sineArgs.size()==2&&cosArgs.size()==1,
           "group2 original math library call cardinality wrong");
    expect(sineArgs[0]==1.0f&&sineArgs[1]==clamp&&cosArgs[0]==clamp,
           "group2 original angle source offsets / clamping wrong");
    const float first=clamp*0.5f;
    expect(floatLoad(two.bytes,0x30)==first,"group2 original sine write at +0x30 wrong");
    expect(floatLoad(two.bytes,0x38)==first*first,"group2 square at +0x38 wrong");
    expect(floatLoad(two.bytes,0x34)==1.f/(0.5f-first),"group2 inverse difference wrong");
    expect(floatLoad(two.bytes,0x3c)==1.f/(2.0f+clamp),"group2 inverse cosine wrong");
    sineArgs.clear();cosArgs.clear();
    floatStore(input,0x34,-100.f);floatStore(input,0x30,100.f);
    two=originalTransformPacketSegment({0x3AF6F8,2,0,1},input,f);
    expect(sineArgs.size()==2&&sineArgs[1]==0.0010000000474974513f&&
           sineArgs[0]==0.0010000000474974513f*0.9499999880790710f,
           "group2 original lower bound / 95 percent cap wrong");
    bool rejected=false;
    try{originalTransformPacketSegment({0x3AF478,0,0,1},input,{});}
    catch(const std::runtime_error&){rejected=true;}
    expect(rejected,"missing original VU transformer accepted");
    rejected=false;
    try{originalTransformPacketSegment({0x3AF680,1,0,1},input,f);}
    catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"raw packet-copy group incorrectly treated as transformed group");
    rejected=false;
    try{originalTransformPacketSegment({0x3AF6F8,2,0,3},input,f);}
    catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"truncated original record count accepted");
    rejected=false;
    try{originalTransformPacketSegment({0x3AF530,3,0,0xffffffffu},input,f);}
    catch(const std::length_error&){rejected=true;}
    expect(rejected,"native transform overflow not rejected");
}
}
int main(){try{run();std::cout<<"ORIGINAL_RENDER_TRANSFORMED_SEGMENTS_PASS\n";return 0;}
catch(const std::exception&e){std::cerr<<"ORIGINAL_RENDER_TRANSFORMED_SEGMENTS_FAIL: "<<e.what()<<"\n";return 1;}}
