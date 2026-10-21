#include "ae3/original_spatial_raw_bridge.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void check(bool b) {if(!b)throw std::runtime_error("original record-source bridge failure");}
static void writeRecord(OriginalSpatialRecord70& r,std::uint32_t ptr,
                        std::span<const OriginalVU0Vec4> points) {
    for(std::size_t n=0;n<4;n++)
        r.originalFields[0x30+n]=std::byte((ptr>>(8*n))&255u);
    for(std::size_t i=0;i<points.size();i++)
        std::memcpy(r.originalFields.data()+0x40+16*i,&points[i],16);
}
int main() {
    const std::array<OriginalVU0Vec4,3> a{{{0,0,0,71},{0,2,0,72},{0,1,2,73}}};
    const std::array<OriginalVU0Vec4,3> b{{{2,0,0,11},{2,2,0,12},{2,1,2,13}}};
    const std::array<OriginalVU0Vec4,3> c{{{5,0,0,21},{5,2,0,22},{5,1,2,23}}};
    std::array<OriginalSpatialRecord70,3> source{};
    writeRecord(source[0],0x51a000u,a);
    writeRecord(source[1],0x51a020u,b);
    writeRecord(source[2],0x51a040u,c);
    const std::array<OriginalSpatialHeaderBinding,3> headers{{
        {0x51a000u,1},{0x51a020u,1},{0x51a040u,1}}};
    check(originalSpatialHeaderPointer_21b468(source[1])==0x51a020u);
    check(originalSpatialValidateHeader_21b468(source[2],headers[2]).originalPointCount==1);
    auto decoded=originalSpatialVertices_21b468(source[0]);
    for(std::size_t i=0;i<3;i++) {
        check(std::bit_cast<std::array<std::byte,16>>(decoded[i])==
              std::bit_cast<std::array<std::byte,16>>(a[i]));
    }
    OriginalResolvedSpatialGeometry binding(source,headers);
    check(binding.polygons().size()==3);
    check(binding.polygons()[1].sourcePoints.size()==1 && binding.polygons()[1].sourcePoints[0].w==11);
    // Test actual source vertex bytes; a one-point source has zero edge normal.
    const OriginalVU0Vec4 query{2,1,0.5f,999.f};
    const std::array<std::uint16_t,3> hits{{2,1,0}};
    const auto expected=originalSpatialNearestGeometry_21d898(
        source,binding.polygons(),OriginalSpatialGridHits{hits},query);
    check(expected==&source[1]);
    check(originalSpatialNearestRawGeometry_21d898(
        source,headers,OriginalSpatialGridHits{hits},query)==expected);
    check(originalSpatialNearestRawAll_21da2c(source,headers,query)==
          originalSpatialNearestAllWithVU0_21da2c(source,binding.polygons(),query));
    OriginalSpatialGridFields grid{};
    grid.originX00=0;grid.originY04=0;grid.stepX10=1;grid.stepY14=1;
    grid.dimensionX18=1;grid.dimensionY1c=1;
    const std::array<std::uint16_t,1> offsets{{0}};
    const std::array<std::uint16_t,4> pool{{3,2,1,0}};
    grid.cellOffsets50=offsets;grid.hitBuffer08=pool;
    check(originalSpatialNearestRawGrid_21d898(source,headers,grid,0,0,query)==expected);
    // Source-header count is retrieved from actual little-endian bytes
    // at source pointer+0x16; no manually populated count list is needed.
    std::array<std::byte,0x60> actualSourceBytes{};
    actualSourceBytes[0x16]=std::byte{1};
    actualSourceBytes[0x36]=std::byte{1};
    actualSourceBytes[0x56]=std::byte{1};
    const std::array<OriginalSpatialSourceAllocation,1> sourceAllocations{{
        {0x51a000u,actualSourceBytes}}};
    const auto resolved=originalSpatialResolveHeaders_21b468(source,sourceAllocations);
    check(resolved.size()==3 && resolved[1].originalHeaderVA==0x51a020u &&
          resolved[1].sourcePointCountAt16==1);
    check(originalSpatialNearestOriginalDataGrid_21d898(
        source,sourceAllocations,grid,0,0,query)==expected);
    check(originalSpatialNearestOriginalDataAll_21da2c(
        source,sourceAllocations,query)==&source[1]);
    const std::array<OriginalSpatialSourceAllocation,1> wrongBase{{
        {0x51b000u,actualSourceBytes}}};
    bool missing=false;
    try{(void)originalSpatialResolveHeaders_21b468(source,wrongBase);}
    catch(const std::out_of_range&){missing=true;}check(missing);
    const std::array<OriginalSpatialSourceAllocation,2> ambiguous{{
        {0x51a000u,actualSourceBytes},{0x51a000u,actualSourceBytes}}};
    bool overlap=false;
    try{(void)originalSpatialResolveHeaders_21b468(source,ambiguous);}
    catch(const std::invalid_argument&){overlap=true;}check(overlap);
    auto corrupt=headers;
    corrupt[1].originalHeaderVA=0x123456u;
    bool failed=false;
    try{OriginalResolvedSpatialGeometry bad(source,corrupt);(void)bad;}catch(const std::invalid_argument&){failed=true;}
    check(failed);
    corrupt=headers;corrupt[1].sourcePointCountAt16=4;
    failed=false;
    try{OriginalResolvedSpatialGeometry bad(source,corrupt);(void)bad;}catch(const std::out_of_range&){failed=true;}
    check(failed);
    corrupt=headers;corrupt[1].sourcePointCountAt16=0;
    failed=false;
    try{OriginalResolvedSpatialGeometry bad(source,corrupt);(void)bad;}catch(const std::out_of_range&){failed=true;}
    check(failed);
    auto broken=source;
    for(std::size_t n=0;n<4;n++)broken[0].originalFields[0x30+n]=std::byte{0};
    failed=false;
    try{OriginalResolvedSpatialGeometry bad(broken,headers);(void)bad;}catch(const std::invalid_argument&){failed=true;}
    check(failed);
    failed=false;
    try{OriginalResolvedSpatialGeometry bad(source,std::span<const OriginalSpatialHeaderBinding>(headers).first(2));(void)bad;}
    catch(const std::invalid_argument&){failed=true;}
    check(failed);
}
