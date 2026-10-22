#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void require(bool condition,const char* message){
    if(!condition)throw std::runtime_error(message);
}
static unsigned read16(std::span<const std::byte> data,unsigned at){
    require(at+2<=data.size(),"Original source u16 out of bounds");
    return std::to_integer<unsigned>(data[at])|
        (std::to_integer<unsigned>(data[at+1])<<8);
}
static unsigned read32(std::span<const std::byte> data,unsigned at){
    return read16(data,at)|(read16(data,at+2)<<16);
}
int main(int argc,char**argv){
    const std::array<OriginalVU0Vec4,3> sample{{{0,0,0,0},{0,0,2,0},{2,0,0,0}}};
    require(originalVU0PolygonEligible_21b788(sample,{.5f,0,.5f,0}),
        "Original VU0 source XZ triangle interior rejected");
    require(!originalVU0PolygonEligible_21b788(sample,{-1,0,.5f,0}),
        "Original VU0 source exterior incorrectly accepted");
    require(originalVU0PolygonEligible_21b788(sample,{-.05f,0,0,0},.2f),
        "Original F12 plane tolerance lost");
    require(argc==1||argc==2,"Optional owner-private woods_c/navigation.pck");
    unsigned polygons=0,outside=0;
    if(argc==2){
        std::ifstream input(argv[1],std::ios::binary);
        require(bool(input),"Original woods_c navigation.pck unavailable");
        const std::vector<char> file((std::istreambuf_iterator<char>(input)),{});
        require(file.size()==3328,"Original woods_c PCK byte count changed");
        std::vector<std::byte> bytes;
        for(unsigned char c:file)bytes.push_back(std::byte(c));
        for(unsigned graph=0;graph<2;++graph){
            const auto source=std::span<const std::byte>(bytes)
                .subspan(graph?2224:96,graph?1104:2128);
            const auto count=read32(source,0x24),header=read32(source,0x3c),
                       vertexBase=read32(source,0x40);
            require(count==(graph?20u:39u),"Retail woods_c graph count differs");
            for(unsigned p=0;p<count;++p){
                const auto at=header+30*p,numVerts=read16(source,at+0x16);
                require(numVerts==3,"Expected three authentic polygon vertices");
                std::array<OriginalVU0Vec4,3> verts{};
                for(unsigned i=0;i<3;++i){
                    const auto from=vertexBase+16*read16(source,at+0x18+2*i);
                    require(from+16<=source.size(),"Original vertex outside asset");
                    std::memcpy(&verts[i],source.data()+from,16);
                }
                OriginalVU0Vec4 centroid{};
                for(const auto& v:verts){
                    centroid.x+=v.x;centroid.y+=v.y;
                    centroid.z+=v.z;centroid.w+=v.w;
                }
                centroid.x/=3;centroid.y/=3;
                centroid.z/=3;centroid.w/=3;
                require(originalVU0PolygonEligible_21b788(verts,centroid),
                    "Authentic original polygon centroid rejected");
                auto elevated=centroid;elevated.y+=1000;
                require(originalVU0PolygonEligible_21b788(verts,elevated),
                    "Original XZ plane incorrectly depends on height Y");
                for(unsigned edge=0;edge<3;++edge){
                    const auto& prev=verts[(edge+2)%3];
                    const auto& curr=verts[edge];
                    const float dx=curr.x-prev.x,dz=curr.z-prev.z;
                    const float length=std::sqrt(dx*dx+dz*dz);
                    require(length>0,"Original source edge has zero length");
                    const OriginalVU0Vec4 probe{
                        (prev.x+curr.x)*.5f-(dz/length)*2,
                        (prev.y+curr.y)*.5f,
                        (prev.z+curr.z)*.5f+(dx/length)*2,0};
                    require(!originalVU0PolygonEligible_21b788(verts,probe),
                        "Authentic source polygon accepted exterior edge probe");
                    ++outside;
                }
                ++polygons;
            }
        }
        require(polygons==59&&outside==177,
                "Expected all 59 original polygons and all 177 directed edges");
    }
}
