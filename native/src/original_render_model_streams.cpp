#include "ae3/original_render_model_streams.hpp"
#include <stdexcept>
namespace ae3 {
namespace {
std::uint32_t word(std::span<const std::uint8_t> src,std::size_t off){
    if(off+4>src.size())throw std::invalid_argument("original model render root truncated");
    return std::uint32_t(src[off]) | std::uint32_t(src[off+1])<<8 |
           std::uint32_t(src[off+2])<<16 | std::uint32_t(src[off+3])<<24;
}
OriginalRenderModelStream parse(std::span<const std::uint8_t> input,
                                std::uint8_t low,std::uint8_t high,std::uint32_t stride){
    OriginalRenderModelStream out;
    out.originalBegin=word(input,low);
    out.originalEnd=word(input,high);
    out.sourceBeginField=low;
    out.sourceEndField=high;
    out.recordStride=stride;
    if(out.originalEnd<out.originalBegin)
        throw std::invalid_argument("retail source render stream end precedes begin");
    const auto bytes=out.originalEnd-out.originalBegin;
    if(bytes%stride)
        throw std::invalid_argument("retail source render stream length not whole records");
    // Safety budget; not a claim that retail source rejects huge counts.
    if(bytes/stride>100000)
        throw std::length_error("original model stream exceeds native safety budget");
    out.recordCount=bytes/stride;
    return out;
}
} // namespace
OriginalRenderModelStreamTable originalDecodeModelStreams27c614(
    std::span<const std::uint8_t> root){
    // Original 0x27C614: ptr pairs 0x04/08,0x14/18,0x24/28,
    // 0x34/38,0x44/48. Their MIPS multiply/shift idioms divide
    // byte differences by 96,96,112,80,128 respectively.
    if(root.size()<0x4c)
        throw std::invalid_argument("retail 0x27C614 root missing source pointer pairs");
    OriginalRenderModelStreamTable result;
    result.streams[0]=parse(root,0x04,0x08,96);
    result.streams[1]=parse(root,0x14,0x18,96);
    result.streams[2]=parse(root,0x24,0x28,112);
    result.streams[3]=parse(root,0x34,0x38,80);
    result.streams[4]=parse(root,0x44,0x48,128);
    return result;
}
OriginalRenderModelStream originalDecodeActorSupplement27c520(
    std::span<const std::uint8_t> root){
    // Original 0x27C5CC–0x27C5DC, pointer distance >> 7.
    return parse(root,0x54,0x58,128);
}
} // namespace ae3
