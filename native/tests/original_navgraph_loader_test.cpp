#include "ae3/original_navgraph_loader.hpp"
#include <zlib.h>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ae3;
static void check(bool ok,const char* text) {
    if(!ok)throw std::runtime_error(text);
}
static void put16(std::vector<std::uint8_t>& b,std::size_t o,std::uint16_t v) {
    b.at(o)=std::uint8_t(v&255);b.at(o+1)=std::uint8_t(v>>8);
}
static void put32(std::vector<std::uint8_t>& b,std::size_t o,std::uint32_t v) {
    for(unsigned i=0;i<4;i++)b.at(o+i)=std::uint8_t((v>>(i*8))&255);
}
static void xyz(std::vector<std::uint8_t>& b,std::size_t o,float x,float y,float z) {
    put32(b,o,std::bit_cast<std::uint32_t>(x));
    put32(b,o+4,std::bit_cast<std::uint32_t>(y));
    put32(b,o+8,std::bit_cast<std::uint32_t>(z));
}
static std::vector<std::uint8_t> navFixture() {
    std::vector<std::uint8_t> src(0x160);
    put32(src,0x24,1); // original NavGraph source+0x24 record count
    for(unsigned i=0;i<6;i++)put32(src,0x38+i*4,0x60+i*0x20);
    put32(src,0x3c,0x80); // 0x1e descriptor
    put32(src,0x40,0xb0); // indexed 0x10 original vertex records
    put32(src,0x48,0x120);
    put32(src,0x4c,0x160); // source end, permitted
    put16(src,0x80,0xffff); // no extra pointer
    put16(src,0x82,0); // no pending actions
    put16(src,0x80+0x16,3); // three vertices
    put16(src,0x80+0x18,0);put16(src,0x80+0x1a,1);
    put16(src,0x80+0x1c,2);
    xyz(src,0xb0,1,0,0);xyz(src,0xc0,3,1,0);xyz(src,0xd0,3,0,1);
    return src;
}
static std::vector<std::uint8_t> pckFixture(std::span<const std::uint8_t> nav) {
    std::vector<std::uint8_t> pck(0x60+nav.size());
    put32(pck,0,0x004b4350);put32(pck,4,0x40);put32(pck,8,1);
    const std::string attr="nav",name="nav_monkey.nav";
    std::memcpy(pck.data()+0x10,attr.c_str(),attr.size());
    std::memcpy(pck.data()+0x14,name.c_str(),name.size());
    put32(pck,0x40,0x14); // name absolute offset, not relative to pool
    put32(pck,0x44,0x10); // attr absolute offset
    put32(pck,0x48,0x60);put32(pck,0x4c,static_cast<std::uint32_t>(nav.size()));
    std::memcpy(pck.data()+0x60,nav.data(),nav.size());
    return pck;
}
static std::vector<std::uint8_t> originalSZ(std::span<const std::uint8_t> pck) {
    check(pck.size()<=UINT32_MAX,"Fixture exceeds original SZ 32-bit length");
    z_stream stream{};
    check(deflateInit2(&stream,Z_BEST_COMPRESSION,Z_DEFLATED,-MAX_WBITS,8,Z_DEFAULT_STRATEGY)==Z_OK,
          "Native test DEFLATE initialization failed");
    const auto maxSize=deflateBound(&stream,static_cast<uLong>(pck.size()));
    std::vector<std::uint8_t> payload(4+maxSize+4);
    put32(payload,0,static_cast<std::uint32_t>(pck.size()));
    stream.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(pck.data()));
    stream.avail_in=static_cast<uInt>(pck.size());
    stream.next_out=payload.data()+4;stream.avail_out=static_cast<uInt>(maxSize);
    check(deflate(&stream,Z_FINISH)==Z_STREAM_END,"Native fixture deflate failed");
    const auto length=stream.total_out;deflateEnd(&stream);
    payload.resize(4+length+4); // the retail SZ blobs may have 4 trailing bytes
    return payload;
}
int main() {
    check(originalNavGraphSlot_4f0fbc("nav_monkey.nav")==0,"Retail nav-monkey slot");
    check(originalNavGraphSlot_4f0fbc("nav_enemy.nav")==1,"Retail nav-enemy slot");
    check(originalNavGraphSlot_4f0fbc("nav_extra0.nav")==2,"Retail nav-extra slot");
    check(originalNavGraphSlot_4f0fbc("unrecognized.nav")==2,"Retail fallthrough slot");
    auto nav=navFixture();
    auto pck=pckFixture(nav);
    auto src=originalSZ(pck);
    check(originalNavInflateSZ(src)==pck,"Retail raw-DEFLATE did not round-trip");
    const auto sources=originalNavReadPackage(src);
    check(sources.size()==1,"Original PCK member count differs");
    check(sources[0].originalFilename=="nav_monkey.nav","PCK absolute name offset misread");
    check(sources[0].originalSlot==0,"Original nav slot changed");
    check(sources[0].originalNavBytes==nav,"PCK source navigation bytes modified");
    const auto spatial=originalNavLoadGraph_4f0a5c(sources[0],0x510000,0x620000);
    check(spatial.records.size()==1,"Original nav record constructor did not run");
    check(spatial.polygonHeaders.size()==1 && spatial.polygonHeaders[0].sourcePointCountAt16==3,
          "Original navigation header vertex count lost");
    const auto points=originalSpatialVertices_21b468(spatial.records[0]);
    check(points[0].x==1 && points[1].x==3 && points[2].z==1,
          "Source .nav vertex data no longer feeds native VU0");
    bool failed=false;
    auto corrupted=src;corrupted[0]=0xff;corrupted[1]=0xff;
    corrupted[2]=0xff;corrupted[3]=0xff;
    try{(void)originalNavInflateSZ(corrupted);}catch(const std::out_of_range&){failed=true;}
    check(failed,"Original SZ oversized output not rejected");
    failed=false;
    auto truncated=src;truncated.resize(10);
    try{(void)originalNavInflateSZ(truncated);}catch(const std::runtime_error&){failed=true;}
    check(failed,"Truncated original SZ stream accepted");
    failed=false;
    auto wrongPck=pckFixture(nav);put32(wrongPck,0x40,0xffff);
    try{(void)originalNavReadPackage(originalSZ(wrongPck));}
    catch(const std::out_of_range&){failed=true;}
    check(failed,"Invalid retail PCK name accepted");
}
