#include "ae3/original_spatial_raw_bridge.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace ae3 {
namespace {
static_assert(std::endian::native==std::endian::little,
              "Original USA SCUS_975.01 records use little-endian float32");
static_assert(sizeof(OriginalVU0Vec4)==16);
static_assert(std::is_trivially_copyable_v<OriginalVU0Vec4>);
constexpr std::size_t headerPointerOffset=0x30;
constexpr std::size_t originalVectorOffset=0x40;
constexpr std::size_t originalVectorSize=0x10;
constexpr std::size_t originalRecordSize=0x70;
static_assert((originalRecordSize-originalVectorOffset)/originalVectorSize==3);
}
std::uint32_t originalSpatialHeaderPointer_21b468(
    const OriginalSpatialRecord70& record) noexcept {
    const auto& b=record.originalFields;
    // Original LW at record+0x30. Host pointer-sized reinterpret-cast is
    // intentionally avoided: these are source-data pointer *identities*.
    return std::uint32_t(std::to_integer<unsigned char>(b[headerPointerOffset]))|
           (std::uint32_t(std::to_integer<unsigned char>(b[headerPointerOffset+1]))<<8)|
           (std::uint32_t(std::to_integer<unsigned char>(b[headerPointerOffset+2]))<<16)|
           (std::uint32_t(std::to_integer<unsigned char>(b[headerPointerOffset+3]))<<24);
}
OriginalRawSpatialRecordInfo originalSpatialValidateHeader_21b468(
    const OriginalSpatialRecord70& record,
    const OriginalSpatialHeaderBinding& header) {
    const auto sourceVA=originalSpatialHeaderPointer_21b468(record);
    if(sourceVA==0||header.originalHeaderVA!=sourceVA)
        throw std::invalid_argument("Original record+0x30 source header is unresolved/mismatched");
    // 0x21B468/LQC2 read vertices directly at record+0x40 in 16-byte
    // strides. The fixed 0x70 record can hold exactly three of them;
    // other counts need their own original allocation layout recovered.
    if(header.sourcePointCountAt16==0||header.sourcePointCountAt16>3)
        throw std::out_of_range("Original polygon count outside recovered 0x70 record");
    return {sourceVA,header.sourcePointCountAt16};
}
std::array<OriginalVU0Vec4,3> originalSpatialVertices_21b468(
    const OriginalSpatialRecord70& record) {
    std::array<OriginalVU0Vec4,3> out{};
    for(std::size_t i=0;i<out.size();i++) {
        // Copies the actual original LQC2 128-bit lanes without type
        // punning/alignment UB or floating-point reinterpretation.
        std::memcpy(&out[i],record.originalFields.data()+originalVectorOffset+i*originalVectorSize,
                    sizeof(OriginalVU0Vec4));
    }
    return out;
}
std::vector<OriginalSpatialHeaderBinding> originalSpatialResolveHeaders_21b468(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourceAllocation> allocations) {
    std::vector<OriginalSpatialHeaderBinding> result;
    result.reserve(records.size());
    for(const auto& record:records) {
        const auto pointer=originalSpatialHeaderPointer_21b468(record);
        if(pointer==0)throw std::invalid_argument("Original polygon header pointer is zero");
        const OriginalSpatialSourceAllocation* matched=nullptr;
        for(const auto& original:allocations) {
            // Original +0x16: load unsigned halfword. Avoid unsigned
            // overflow and require the entire original 24-byte header.
            if(pointer<original.originalBaseVA)continue;
            const std::uint64_t offset=std::uint64_t(pointer)-original.originalBaseVA;
            if(offset>original.originalBytes.size() ||
               original.originalBytes.size()-static_cast<std::size_t>(offset)<0x18)continue;
            if(matched)throw std::invalid_argument("Overlapping original source allocation addresses");
            matched=&original;
        }
        if(!matched)throw std::out_of_range("Original polygon header missing from source allocations");
        const auto offset=static_cast<std::size_t>(std::uint64_t(pointer)-matched->originalBaseVA);
        const auto low=std::to_integer<std::uint16_t>(matched->originalBytes[offset+0x16]);
        const auto high=std::to_integer<std::uint16_t>(matched->originalBytes[offset+0x17]);
        const OriginalSpatialHeaderBinding bound{pointer,std::uint16_t(low|(high<<8))};
        originalSpatialValidateHeader_21b468(record,bound);
        result.push_back(bound);
    }
    return result;
}
OriginalResolvedSpatialGeometry::OriginalResolvedSpatialGeometry(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialHeaderBinding> sourceHeaders) {
    if(sourceRecords.size()!=sourceHeaders.size())
        throw std::invalid_argument("Original 0x70 spatial records and resolved headers must match");
    points_.reserve(sourceRecords.size());
    headers_.reserve(sourceRecords.size());
    polygons_.reserve(sourceRecords.size());
    for(std::size_t i=0;i<sourceRecords.size();i++) {
        auto info=originalSpatialValidateHeader_21b468(sourceRecords[i],sourceHeaders[i]);
        auto points=originalSpatialVertices_21b468(sourceRecords[i]);
        headers_.push_back(info);
        points_.push_back(points);
    }
    for(std::size_t i=0;i<sourceRecords.size();i++)
        polygons_.push_back({std::span<const OriginalVU0Vec4>(
            points_[i].data(),headers_[i].originalPointCount)});
}
const OriginalSpatialRecord70* originalSpatialNearestOriginalDataGrid_21d898(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourceAllocation> allocations,
    const OriginalSpatialGridFields& grid,float x,float y,
    const OriginalVU0Vec4& query) {
    const auto headers=originalSpatialResolveHeaders_21b468(records,allocations);
    return originalSpatialNearestRawGrid_21d898(records,headers,grid,x,y,query);
}
const OriginalSpatialRecord70* originalSpatialNearestOriginalDataAll_21da2c(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourceAllocation> allocations,
    const OriginalVU0Vec4& query) {
    const auto headers=originalSpatialResolveHeaders_21b468(records,allocations);
    return originalSpatialNearestRawAll_21da2c(records,headers,query);
}
const OriginalSpatialRecord70* originalSpatialNearestRawGeometry_21d898(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialHeaderBinding> headers,
    OriginalSpatialGridHits hits,const OriginalVU0Vec4& query) {
    OriginalResolvedSpatialGeometry binding(records,headers);
    return originalSpatialNearestGeometry_21d898(records,binding.polygons(),hits,query);
}
const OriginalSpatialRecord70* originalSpatialNearestRawGrid_21d898(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialHeaderBinding> headers,
    const OriginalSpatialGridFields& grid,float x,float y,
    const OriginalVU0Vec4& query) {
    return originalSpatialNearestRawGeometry_21d898(
        records,headers,originalSpatialGridLookup_4e3c14(grid,x,y),query);
}
const OriginalSpatialRecord70* originalSpatialNearestRawAll_21da2c(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialHeaderBinding> headers,
    const OriginalVU0Vec4& query) {
    OriginalResolvedSpatialGeometry binding(records,headers);
    return originalSpatialNearestAllWithVU0_21da2c(records,binding.polygons(),query);
}
} // namespace ae3
