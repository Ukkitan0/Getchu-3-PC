#include "ae3/original_render_packet_copies.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
namespace {
void require(bool v,const char* s){if(!v)throw std::runtime_error(s);}
void run(){
    std::vector<std::uint8_t> ownedSource(4*0x30);
    for(std::size_t i=0;i<ownedSource.size();++i)
        ownedSource[i]=std::uint8_t((i*137)&255);
    const OriginalPacketAttachment kind1{0x3af680,1,0x123456,3};
    auto segment=originalCopyRenderPacketRecords(kind1,ownedSource);
    require(segment.kind==1&&segment.sourceVa==0x3af680&&
            segment.originalRecordCount==3&&segment.originalAllocator16ByteUnits==6&&
            segment.bytes.size()==0x60,"retail 0x3AF680 uses 32-byte records/2 units each");
    require(std::equal(segment.bytes.begin(),segment.bytes.end(),ownedSource.begin()),
            "original 32-byte data was interpreted or changed");
    segment.bytes[0]^=0xff;
    require(segment.bytes[0]!=ownedSource[0],"native source packet bytes must be owned");
    const OriginalPacketAttachment kind4{0x3af600,4,0xabcdef,2};
    segment=originalCopyRenderPacketRecords(kind4,ownedSource);
    require(segment.kind==4&&segment.originalAllocator16ByteUnits==6&&segment.bytes.size()==0x60,
            "retail 0x3AF600 uses 48-byte records/3 units each");
    require(std::equal(segment.bytes.begin(),segment.bytes.end(),ownedSource.begin()),
            "original 48-byte data corrupted");
    bool rejected=false;
    try{originalCopyRenderPacketRecords({0x3af478,0,0,2},ownedSource);}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"unrecovered VU segment incorrectly treated as raw copy");
    rejected=false;
    try{originalCopyRenderPacketRecords({0x3af680,1,0,3},std::span(ownedSource).first(0x40));}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"truncated authenticated model payload accepted");
    rejected=false;
    try{originalCopyRenderPacketRecords({0x3af600,4,0,0xffffffff},ownedSource);}
    catch(const std::length_error&){rejected=true;}
    require(rejected,"retail count overflow/oversized packet accepted");
    segment=originalCopyRenderPacketRecords({0x3af680,1,0,0},{});
    require(segment.bytes.empty()&&segment.originalAllocator16ByteUnits==0,
            "zero source count unexpectedly fabricated data");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_PACKET_COPIES_PASS: retail_3AF680_stride32 retail_3AF600_stride48 owned_data budgets\n";return 0;}
    catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
