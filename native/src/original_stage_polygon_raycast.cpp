#include "ae3/original_stage_polygon_raycast.hpp"
#include <cstdint>
#include <stdexcept>
namespace ae3 {
namespace {
std::uint16_t readOriginalHeaderU16(std::span<const std::byte> bytes,
    std::uint64_t offset) {
    if(offset>bytes.size()||bytes.size()-static_cast<std::size_t>(offset)<2)
        throw std::out_of_range("Original polygon header outside owner stage data");
    const auto p=static_cast<std::size_t>(offset);
    return std::uint16_t(std::to_integer<unsigned>(bytes[p])|
        (std::to_integer<unsigned>(bytes[p+1])<<8));
}
}
std::vector<OriginalPolygonLinks_21bd3c> originalStagePolygonLinks_21bd3c(
    const OriginalStageSpatialSource& originalSource,
    const OriginalStageSpatialLoaded& originalStage) {
    const auto count=static_cast<std::size_t>(originalStage.sourceHeader.countAt24);
    if(count!=originalStage.records.size()||
       count!=originalStage.polygonHeaders.size())
        throw std::invalid_argument("Original stage polygon headers/records count mismatch");
    const auto base=std::uint64_t(originalStage.sourceHeader.relativeOffsets[1]);
    if(base>originalSource.loadedBytes.size()||
       count>(originalSource.loadedBytes.size()-static_cast<std::size_t>(base))/0x1e)
        throw std::out_of_range("Source original 0x1E polygon header table truncated");
    std::vector<OriginalPolygonLinks_21bd3c> graph(count);
    for(std::size_t i=0;i<count;++i){
        const auto descriptor=base+i*0x1e;
        auto& item=graph[i];
        item.edgeCount=readOriginalHeaderU16(originalSource.loadedBytes,descriptor+2);
        if(item.edgeCount>3)
            throw std::out_of_range("Original stage polygon has more than three directed links");
        for(unsigned edge=0;edge<item.edgeCount;++edge){
            const auto neighbor=readOriginalHeaderU16(
                originalSource.loadedBytes,descriptor+4+edge*2);
            if(neighbor>=count)
                throw std::out_of_range("Original stage neighbor outside loaded polygon registry");
            item.edges[edge]={
                neighbor,
                readOriginalHeaderU16(originalSource.loadedBytes,descriptor+0xa+edge*2)};
        }
    }
    return graph;
}
OriginalPolygonRaycastResult_4c1b3c originalStagePolygonRaycast_4c1b3c(
    const OriginalStageSpatialSource& originalSource,
    const OriginalStageSpatialLoaded& originalStage,
    std::uint32_t originalStagePolygonIndex,
    const OriginalVU0Vec4& originalCallerRayStart,
    const OriginalVU0Vec4& originalCallerRayEnd,
    std::size_t nativeMaxRecursions) {
    const auto sourceHeaders=originalStagePolygonLinks_21bd3c(originalSource,originalStage);
    return originalPolygonRaycast_4c1b3c(
        originalStage.records,sourceHeaders,originalStagePolygonIndex,
        originalCallerRayStart,originalCallerRayEnd,nativeMaxRecursions);
}
} // namespace ae3
