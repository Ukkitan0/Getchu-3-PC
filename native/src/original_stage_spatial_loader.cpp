#include "ae3/original_stage_spatial_loader.hpp"
#include <bit>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace ae3 {
namespace {
std::size_t checkedOffset(const OriginalStageSpatialSource& input,
                          std::uint64_t off,std::size_t size) {
    if(off>input.loadedBytes.size() ||
       size>input.loadedBytes.size()-static_cast<std::size_t>(off))
        throw std::out_of_range("Truncated original stage spatial source data");
    return static_cast<std::size_t>(off);
}
std::uint16_t read16(const OriginalStageSpatialSource& input,std::uint64_t off) {
    const auto i=checkedOffset(input,off,2);
    return std::uint16_t(std::to_integer<unsigned char>(input.loadedBytes[i]))|
      (std::uint16_t(std::to_integer<unsigned char>(input.loadedBytes[i+1]))<<8);
}
std::uint32_t read32(const OriginalStageSpatialSource& input,std::uint64_t off) {
    const auto i=checkedOffset(input,off,4);
    return std::uint32_t(std::to_integer<unsigned char>(input.loadedBytes[i]))|
      (std::uint32_t(std::to_integer<unsigned char>(input.loadedBytes[i+1]))<<8)|
      (std::uint32_t(std::to_integer<unsigned char>(input.loadedBytes[i+2]))<<16)|
      (std::uint32_t(std::to_integer<unsigned char>(input.loadedBytes[i+3]))<<24);
}
void store32(std::array<std::byte,0x70>& target,std::size_t offset,std::uint32_t word) {
    for(unsigned j=0;j<4;j++)target[offset+j]=std::byte((word>>(8*j))&0xff);
}
}
OriginalStageSpatialOffsets originalStageReadSpatialOffsets_21d46c(
    const OriginalStageSpatialSource& stage) {
    if(!stage.runtimeBaseVA)
        throw std::invalid_argument("Original stage allocation's runtime base VA must be supplied");
    OriginalStageSpatialOffsets desc{};
    desc.countAt24=read32(stage,0x24);
    // 0x21D4B4..0x21D51C resolves source+offset to original pointer.
    for(std::size_t i=0;i<6;i++) {
        const auto src=read32(stage,0x38+4*i);
        checkedOffset(stage,src,0);
        const auto absolute=std::uint64_t(stage.runtimeBaseVA)+src;
        if(absolute>std::numeric_limits<std::uint32_t>::max())
            throw std::out_of_range("Retail 32-bit stage VA exceeds original address space");
        desc.relativeOffsets[i]=src;
        desc.resolvedVAs[i]=static_cast<std::uint32_t>(absolute);
    }
    return desc;
}
OriginalStageSpatialLoaded originalStageBuildSourceSpatial_21d46c(
    const OriginalStageSpatialSource& stage) {
    OriginalStageSpatialLoaded built{};
    built.sourceHeader=originalStageReadSpatialOffsets_21d46c(stage);
    const auto& fields=built.sourceHeader.relativeOffsets;
    const auto recordCount=static_cast<std::size_t>(built.sourceHeader.countAt24);
    // The source uses a 0x1E-byte polygon header per 0x70-byte record.
    // Enforce bounds before allocating host memory, including huge counts.
    if(recordCount && recordCount > stage.loadedBytes.size()/0x1e)
        throw std::out_of_range("Original stage spatial-record count cannot fit loaded source");
    checkedOffset(stage,fields[1],recordCount*0x1e);
    built.records.resize(recordCount);
    built.polygonHeaders.reserve(recordCount);
    for(std::size_t index=0;index<recordCount;index++) {
        auto& rec=built.records[index].originalFields;
        // Original 0x21B254 constructor installs the registry owner at
        // record+0 and owner+0x18 at record+0x24, and clears child slots.
        if(stage.originalRegistryOwnerVA) {
            const auto childVA=std::uint64_t(stage.originalRegistryOwnerVA)+0x18;
            if(childVA>std::numeric_limits<std::uint32_t>::max())
                throw std::out_of_range("Original registry owner+0x18 VA overflow");
            store32(rec,0,stage.originalRegistryOwnerVA);
            store32(rec,0x24,static_cast<std::uint32_t>(childVA));
        }
        const auto originalHeaderOff=std::uint64_t(fields[1])+index*0x1e;
        const auto originalHeaderVA=std::uint64_t(stage.runtimeBaseVA)+originalHeaderOff;
        if(originalHeaderVA>std::numeric_limits<std::uint32_t>::max())
            throw std::out_of_range("Original descriptor VA overflow");
        // Source 0x21D5A4: SW pointer into record+0x30.
        store32(rec,0x30,static_cast<std::uint32_t>(originalHeaderVA));
        const auto vertices=read16(stage,originalHeaderOff+0x16);
        if(vertices==0 || vertices>3)
            throw std::out_of_range("Original polygon exceeds three decoded 0x70 record vertices");
        built.polygonHeaders.push_back({static_cast<std::uint32_t>(originalHeaderVA),vertices});
        // Source 0x21D5D0..0x21D60C: each original polygon local
        // 16-bit vertex index is resolved through global index table
        // at 0x21D4FC, vertex table entries are 16 bytes each.
        for(std::uint16_t j=0;j<vertices;j++) {
            const auto vertexId=read16(stage,originalHeaderOff+0x18+std::uint64_t(j)*2);
            const auto sourceOff=std::uint64_t(fields[2])+std::uint64_t(vertexId)*16;
            checkedOffset(stage,sourceOff,12);
            const auto destOff=0x40u+16u*j;
            std::memcpy(rec.data()+destOff,stage.loadedBytes.data()+sourceOff,12);
            // The original inserts the IEEE754 1.0f fourth lane.
            store32(rec,destOff+12,0x3f800000u);
        }
        // 0x21D610..0x21D640: optional +0x1C pointer constructed
        // from an original header selector and 0x20-byte entries.
        const auto sentinel=read16(stage,originalHeaderOff);
        if(sentinel!=0xffff) {
            const auto pointer=std::uint64_t(stage.runtimeBaseVA)+fields[4]+
                               std::uint64_t(sentinel)*32;
            if(pointer>std::numeric_limits<std::uint32_t>::max())
                throw std::out_of_range("Original source record extra pointer overflow");
            // 0x21D610..0x21D640 selects a complete 0x20-byte
            // original entry; accepting only one trailing byte leaves
            // an unresolved/truncated original object reference.
            checkedOffset(stage,std::uint64_t(fields[4])+std::uint64_t(sentinel)*32,32);
            store32(rec,0x1c,static_cast<std::uint32_t>(pointer));
        }
        // Dynamic class/action entries are game-owned. Preserve and
        // report them rather than constructing fake actors/controllers.
        const auto actionCount=read16(stage,originalHeaderOff+2);
        // The original 0x1E-byte descriptor lays out action types at
        // +0x0A..+0x0F and arguments at +0x10..+0x15. +0x16 is the
        // separately recovered polygon vertex count; exceeding three
        // action pairs would reinterpret other descriptor fields.
        if(actionCount>3)
            throw std::out_of_range("Original nav descriptor action count exceeds fixed three-slot layout");
        for(std::uint16_t a=0;a<actionCount;a++) {
            const auto type=read16(stage,originalHeaderOff+0x0a+std::uint64_t(a)*2);
            const auto argument=read16(stage,originalHeaderOff+0x10+std::uint64_t(a)*2);
            if(type==1||type==2)
                built.pendingOriginalActions.push_back({static_cast<std::uint32_t>(index),a,type,argument});
        }
    }
    return built;
}
const OriginalSpatialRecord70* originalStageNearestGrid_21d898(
    const OriginalStageSpatialLoaded& stage,
    const OriginalSpatialGridFields& grid,float x,float y,
    const OriginalVU0Vec4& query) {
    return originalSpatialNearestRawGrid_21d898(
        stage.records,stage.polygonHeaders,grid,x,y,query);
}
const OriginalSpatialRecord70* originalStageNearestAll_21da2c(
    const OriginalStageSpatialLoaded& stage,const OriginalVU0Vec4& query) {
    return originalSpatialNearestRawAll_21da2c(
        stage.records,stage.polygonHeaders,query);
}
} // namespace ae3
