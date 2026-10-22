#include "ae3/original_actor_navigation_pipeline.hpp"
#include "ae3/original_actor_random_triangle.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;

class NoReplacementMath final : public OriginalActorUntranslatedMath_57c7cc {
public:
    unsigned called=0;
    const OriginalSpatialRecord70* call_original_4c1b3c_projection(
        const OriginalSpatialRecord70&,const OriginalVU0Vec4&) override {
        ++called;throw std::runtime_error("Unrecovered polygon projection called");
    }
    std::uint64_t call_original_57ca68_fallback(
        const OriginalSpatialRecord70&) override {
        ++called;throw std::runtime_error("Unrecovered fallback vector called");
    }
    std::uint64_t call_original_57c98c_random_vertex_transform(
        const OriginalSpatialRecord70&,const std::array<std::array<std::byte,16>,3>&) override {
        ++called;throw std::runtime_error("Replaced game RNG triangle instead of native source");
    }
};
static std::uint64_t low64(const OriginalVU0Vec4& v) {
    return std::uint64_t(std::bit_cast<std::uint32_t>(v.x)) |
        (std::uint64_t(std::bit_cast<std::uint32_t>(v.y))<<32);
}
int main(int argc,char**argv) {
    assert(argc==1||argc==2);
    for(std::uint32_t seed: {0u,1u,5489u,0xffffffffu}) {
        OriginalGameRandom_36f3c0 random(seed),check(seed);
        const auto weights=originalActorTriangleWeights_57c98c(random);
        const auto first=check.originalRandomFloat_36f4d4();
        const auto second=check.originalRandomFloatRange_36f548(0.0f,1.0f-first);
        assert(weights.firstDraw==first&&weights.secondDraw==second);
        assert(random.originalCursor()==1&&check.originalCursor()==1);
        std::array<OriginalVU0Vec4,3> source{
            OriginalVU0Vec4{1,2,3,1},OriginalVU0Vec4{4,5,6,1},OriginalVU0Vec4{7,8,9,1}};
        OriginalGameRandom_36f3c0 a(seed),b(seed);
        const auto sample=originalActorRandomTriangle_57c98c(source,a);
        std::array<std::array<std::byte,16>,3> bytes{};
        for(unsigned i=0;i<3;++i)std::memcpy(bytes[i].data(),&source[i],16);
        assert(originalActorRandomTriangleLow64_57c98c(bytes,b)==low64(sample));
        assert(a.originalCursor()==1&&b.originalCursor()==1);
        assert(std::isfinite(sample.x)&&std::isfinite(sample.y)&&std::isfinite(sample.z));
        const auto sum=weights.originalWeightedCoefficients[0]+
            weights.originalWeightedCoefficients[1]+weights.originalWeightedCoefficients[2];
        assert(std::fabs(sample.w-sum)<0.000001f);
    }
    unsigned verified=0;
    if(argc==2) {
        std::ifstream file(argv[1],std::ios::binary);assert(file);
        const std::vector<char> retail((std::istreambuf_iterator<char>(file)),{});
        assert(retail.size()==3328);
        auto u16=[&](std::size_t p)->unsigned {
            return static_cast<unsigned char>(retail.at(p))|
                (unsigned(static_cast<unsigned char>(retail.at(p+1)))<<8);
        };
        auto u32=[&](std::size_t p)->unsigned {return u16(p)|(u16(p+2)<<16);};
        for(unsigned graph=0;graph<2;++graph) {
            const unsigned off=graph?2224:96;
            const unsigned count=u32(off+0x24),desc=u32(off+0x3c),vec=u32(off+0x40);
            assert(count==(graph?20u:39u));
            std::vector<OriginalSpatialRecord70> records(count);
            std::vector<std::array<OriginalVU0Vec4,3>> vertexRecords(count);
            std::vector<OriginalSpatialSourcePolygon> polygons(count);
            for(unsigned i=0;i<count;++i) {
                const auto at=off+desc+30*i,number=u16(at+0x16);
                assert(number>=1&&number<=3);
                for(unsigned j=0;j<number;++j) {
                    const std::size_t p=off+vec+16*u16(at+0x18+j*2);
                    assert(p+16<=off+(graph?1104:2128));
                    std::memcpy(&vertexRecords[i][j],retail.data()+p,16);
                    std::memcpy(records[i].originalFields.data()+0x40+j*16,retail.data()+p,16);
                }
                polygons[i]={std::span<const OriginalVU0Vec4>(vertexRecords[i].data(),number)};
            }
            for(unsigned i=0;i<count;++i) {
                // Two source route entries select the second pointer; no
                // unrelated projection or fallback VU0 calculation required.
                const std::array<std::uint32_t,2> route{0,i};
                OriginalActorRouteFollowerState_57c7cc actor{
                    records,route,&records[0],nullptr,true};
                std::array<std::byte,16> controller20{};
                NoReplacementMath dependencies;
                OriginalGameRandom_36f3c0 actual(0x1571+i),independent(0x1571+i);
                OriginalActorNavigationPipeline_57c7cc pipeline(
                    records,polygons,controller20,false,dependencies,&actual);
                const auto progress=pipeline.advance(actor);
                assert(progress.sourceTargetsApplied==1);
                assert(actor.activePolygonAtController50==&records[i]);
                assert(dependencies.called==0);
                const auto expected=originalActorRandomTriangle_57c98c(vertexRecords[i],independent);
                assert(pipeline.sourceTargetResults().size()==1);
                assert(pipeline.sourceTargetResults()[0].originalVectorLow64==low64(expected));
                assert(actual.originalCursor()==independent.originalCursor());
                ++verified;
            }
        }
        assert(verified==59);
    }
    std::cout<<"Original game actor 0x57C98C and pipeline, retail polygons: "<<verified<<"\n";
}
