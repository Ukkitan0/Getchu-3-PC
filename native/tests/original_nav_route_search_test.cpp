#include "ae3/original_nav_route_search.hpp"
#include "ae3/original_stage_spatial_loader.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool v,const char* message){if(!v)throw std::runtime_error(message);}
static void w16(std::vector<std::byte>& b,std::size_t p,std::uint16_t n){
    b.at(p)=std::byte(n&255);b.at(p+1)=std::byte(n>>8);
}
static void w32(std::vector<std::byte>& b,std::size_t p,std::uint32_t n){
    for(unsigned i=0;i<4;i++)b.at(p+i)=std::byte((n>>(8*i))&255);
}
struct Spline final:OriginalNavRouteSplineCalls {
    unsigned called=0;
    float call_original_21b4e0_21b194(
        const OriginalSpatialRecord70&,std::uint32_t slot) override {
        check(slot==0,"Original spline slot mismatch");
        called++;
        return 0.4f;
    }
};
int main(int argc,char** argv){
    std::vector<std::byte> original(0x250);
    w32(original,0x24,4);w32(original,0x3c,0x80);
    w32(original,0x40,0x130);
    for(std::size_t i=0;i<4;i++){
        const auto at=0x80+0x1e*i;
        w16(original,at,0xffff);
        w16(original,at+0x16,1);
        w16(original,at+0x18,static_cast<std::uint16_t>(i));
        const float x=static_cast<float>(i==2?1:i==3?2:i);
        w32(original,0x130+16*i,std::bit_cast<std::uint32_t>(x));
    }
    const auto at0=0x80u,at1=at0+0x1eu,at2=at1+0x1eu;
    w16(original,at0+2,2);w16(original,at0+4,1);w16(original,at0+6,2);
    w16(original,at1+2,1);w16(original,at1+4,3);
    w16(original,at1+0xa,2);
    w16(original,at2+2,1);w16(original,at2+4,3);
    const auto stage=originalStageBuildSourceSpatial_21d46c(
        {0x510000,original,0x620000});
    auto work=originalNavRouteBuildSourceWorkspace_220c00(
        stage.records,original);
    check(work.nodes.size()==4,"Original route record count");
    check(work.nodes[1].linkTypes18[0]==2,"Original type-2 link lost");
    check(work.nodes[0].destinationIndices0c[1]==2,"Original linked destination");
    check(originalNavRouteFindRecord_220d14(work,&stage.records[2])==2,
          "Original pointer identity lookup");
    OriginalSpatialRecord70 unrelated{};
    check(originalNavRouteFindRecord_220d14(work,&unrelated)==-1,
          "Missing original node did not return -1");
    check(originalNavRouteNearest_220f60(work,{0,0,0,999})==0,
          "Original nearest source centroid");
    check(originalNavRouteFarthest_221030(work,{0,0,0,-999})==3,
          "Original farthest source centroid");
    originalNavRouteRelax_220d5c(work);
    check(work.nodes[0].cost34==0.f,"Original initial route cost");
    check(work.nodes[1].cost34==1.f&&work.nodes[2].cost34==1.f,
          "Parallel original navigation routes");
    check(work.nodes[3].cost34==2.f,"Original accumulated route cost");
    check(work.nodes[3].predecessor30==1,"Original strict predecessor tie");
    const auto path=originalNavRouteExtract_220994(work,3);
    check(path.size()==3&&path[0]==&stage.records[0]&&
          path[1]==&stage.records[1]&&path[2]==&stage.records[3],
          "Original route backtrace and forward ordering");
    check(originalNavRouteExtract_220994(work,-1).empty(),
          "Original null route reconstruction");
    check(originalNavRouteExtract_220994(work,0).size()==1,
          "Original root-only route");
    auto kind1=originalNavRouteBuildSourceWorkspace_220c00(
        stage.records,original);
    kind1.nodes[0].linkTypes18[0]=1;
    bool missing=false;
    try{originalNavRouteRelax_220d5c(kind1);}
    catch(const std::runtime_error&){missing=true;}
    check(missing,"Original unresolved spline was incorrectly fabricated");
    kind1=originalNavRouteBuildSourceWorkspace_220c00(
        stage.records,original);
    kind1.nodes[0].linkTypes18[0]=1;
    Spline spline{};
    originalNavRouteRelax_220d5c(kind1,&spline);
    check(spline.called==1,"Missing original type-1 source call");
    check(std::abs(kind1.nodes[3].cost34-1.4f)<0.0001f,
          "Original spline source-cost relaxation");
    auto invalid=original;
    w16(invalid,at0+4,999);
    bool bad=false;
    try{(void)originalNavRouteBuildSourceWorkspace_220c00(
          stage.records,invalid);}
    catch(const std::out_of_range&){bad=true;}
    check(bad,"Invalid retail adjacency target accepted");
    invalid=original;
    w16(invalid,at0+0xa,99);
    auto unknown=originalNavRouteBuildSourceWorkspace_220c00(
        stage.records,invalid);
    bad=false;
    try{originalNavRouteRelax_220d5c(unknown);}
    catch(const std::runtime_error&){bad=true;}
    check(bad,"Unsupported original link type was guessed");
    if(argc>1){
        std::ifstream in(argv[1],std::ios::binary);
        check(bool(in),"Cannot open owner-provided original .nav file");
        const std::vector<char> raw{std::istreambuf_iterator<char>(in),
                                    std::istreambuf_iterator<char>()};
        std::vector<std::byte> bytes(raw.size());
        for(std::size_t i=0;i<raw.size();i++)
            bytes[i]=std::byte(static_cast<unsigned char>(raw[i]));
        const auto loaded=originalStageBuildSourceSpatial_21d46c(
            {0x510000,bytes,0x620000});
        auto retail=originalNavRouteBuildSourceWorkspace_220c00(
            loaded.records,bytes);
        for(const auto& node:retail.nodes)
            for(std::size_t i=0;i<node.linkCount08;i++)
                check(node.linkTypes18[i]==0||node.linkTypes18[i]==2,
                      "Original retail file has an unported link type");
        originalNavRouteRelax_220d5c(retail);
        check(retail.nodes.size()==loaded.records.size(),
              "Original loaded record count differs");
    }
}
