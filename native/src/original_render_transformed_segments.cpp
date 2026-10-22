#include "ae3/original_render_transformed_segments.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace ae3 {
namespace {
constexpr std::size_t maxPacketBytes=64u*1024u*1024u;
float readFloat(std::span<const std::uint8_t> data,std::size_t offset){
    std::uint32_t bits=0;
    for(unsigned i=0;i<4;++i)bits|=std::uint32_t(data[offset+i])<<(i*8);
    float result;
    static_assert(sizeof(result)==sizeof(bits));
    std::memcpy(&result,&bits,sizeof(result));
    return result;
}
void writeFloat(std::span<std::uint8_t> data,std::size_t offset,float v){
    std::uint32_t bits;
    std::memcpy(&bits,&v,sizeof(bits));
    for(unsigned i=0;i<4;++i)data[offset+i]=std::uint8_t(bits>>(i*8));
}
void originalGroupTwoScalars(std::span<const std::uint8_t> source,
                             std::span<std::uint8_t> dest,
                             const OriginalTransformDependencies& f){
    if(!f.retailFunction41d3d8||!f.retailFunction41d490)
        throw std::runtime_error("original 0x41D3D8/0x41D490 functions unbound");
    // Source at 0x7206D8/DC/E0/E4: 1.569796323..., 0.949999988,
    // 1.0, 0.00100000005. Verified directly against retail .rodata.
    constexpr float upper=1.5697963237762451f;
    constexpr float scale=0.9499999880790710f;
    constexpr float one=1.0f;
    constexpr float lower=0.0010000000474974513f;
    // 0x3AF7AC load source+0x34 first; original COP1 c.olt.s
    // and branch-likely clamp each source value to [lower,upper].
    const float sourceAngle=readFloat(source,0x34);
    const float sourceLimit=readFloat(source,0x30);
    if(!std::isfinite(sourceAngle)||!std::isfinite(sourceLimit))
        throw std::invalid_argument("nonfinite retail group-two angles");
    const float a=std::max(lower,std::min(upper,sourceAngle));
    float b=std::max(lower,std::min(upper,sourceLimit));
    // 0x3AF7F8 mul.s; 0x3AF7FC/0x3AF800 bc1tl caps b.
    const volatile float scaled=a*scale;
    if(scaled<b)b=scaled;
    // 0x3AF808 and 0x3AF814 are TWO separate calls to 0x41D3D8.
    // Do not replace either with host sin until source equivalence proven.
    const float sineB=f.retailFunction41d3d8(b);
    const float sineA=f.retailFunction41d3d8(a);
    const volatile float difference=sineB-sineA; // 0x3AF81C
    if(!std::isfinite(sineB)||!std::isfinite(sineA)||!std::isfinite(difference)||difference==0)
        throw std::runtime_error("retail group-two normalization denominator invalid");
    writeFloat(dest,0x30,sineA); // 0x3AF820
    const volatile float square=sineA*sineA; // 0x3AF834
    writeFloat(dest,0x38,square); // 0x3AF838
    const volatile float invDiff=one/difference; // 0x3AF830
    // Original delay slot at 0x3AF840 writes f20 to dest+0x34.
    writeFloat(dest,0x34,invDiff);
    const float cosineA=f.retailFunction41d490(a); // 0x3AF83C
    if(!std::isfinite(cosineA)||cosineA==0)
        throw std::runtime_error("retail group-two 0x41D490 denominator invalid");
    const volatile float invCos=one/cosineA; // 0x3AF84C
    writeFloat(dest,0x3C,invCos); // 0x3AF858 branch delay slot
}
} // namespace
OriginalTransformGroupLayout originalTransformGroupLayout(std::uint8_t kind){
    switch(kind){
      case 0:return {0,0x3AF478,0x10,0x14,2,0x20};
      case 2:return {2,0x3AF6F8,0x20,0x24,4,0x40};
      case 3:return {3,0x3AF530,0x28,0x2C,3,0x30};
      default:throw std::invalid_argument("original packet group has no pending VU transform");
    }
}
OriginalTransformedPacketSegment originalTransformPacketSegment(
    const OriginalPacketAttachment& call,std::span<const std::uint8_t> data,
    const OriginalTransformDependencies& funcs){
    const auto layout=originalTransformGroupLayout(call.kind);
    if(call.sourceVa!=layout.sourceVa)
        throw std::invalid_argument("source VA does not match retail packet group");
    if(!funcs.originalVuTransform)
        throw std::runtime_error("genuine original VU render-record conversion unavailable");
    if(call.kind==2&&(!funcs.retailFunction41d3d8||!funcs.retailFunction41d490))
        throw std::runtime_error("original group-two libm routines unavailable");
    const auto stride=std::size_t(layout.bytesPerRecord);
    if(call.count>maxPacketBytes/stride)
        throw std::length_error("original packet transform exceeds native safe allocation budget");
    const auto n=std::size_t(call.count)*stride;
    if(n>data.size())throw std::invalid_argument("authentic packet transform source truncated");
    OriginalTransformedPacketSegment out;
    out.layout=layout;
    out.originalRecordCount=call.count;
    out.bytes.resize(n);
    for(std::uint32_t i=0;i<call.count;++i){
        auto from=data.subspan(std::size_t(i)*stride,stride);
        auto into=std::span<std::uint8_t>(out.bytes).subspan(std::size_t(i)*stride,stride);
        funcs.originalVuTransform(call.kind,i,from,into);
        if(call.kind==2)originalGroupTwoScalars(from,into,funcs);
    }
    return out;
}
} // namespace ae3
