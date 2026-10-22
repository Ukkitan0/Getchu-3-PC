#include "ae3/original_actor_ray_geometry.hpp"
#include <array>
#include <bit>
#include <cmath>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>
using namespace ae3;
int main(int argc,char**argv) {
    const OriginalVU0Vec4 a{1,2,3,55},b{4,5,6,-12};
    assert(originalVU0DotXYZ_4748ac(a,b)==32.0f);
    const auto packedOrigin=originalActorPackedSubsequentRayStart_57c88c(a);
    assert(std::bit_cast<std::uint32_t>(packedOrigin.x)==0 &&
           std::bit_cast<std::uint32_t>(packedOrigin.y)==0 &&
           std::bit_cast<std::uint32_t>(packedOrigin.z)==
           std::bit_cast<std::uint32_t>(a.z) &&
           std::bit_cast<std::uint32_t>(packedOrigin.w)==
           std::bit_cast<std::uint32_t>(a.w));
    // 0x57C814 initial ray must still retain original a1 X/Y.
    assert(a.x==1.0f && a.y==2.0f);
    const auto ray=originalActorProjectionRay_4c1b3c(a,b);
    assert(ray.originalDisplacement.x==3 &&
           ray.originalDisplacement.y==3 &&
           ray.originalDisplacement.z==3 &&
           ray.originalDisplacement.w==-67);
    assert(std::fabs(ray.originalLengthF12-std::sqrt(27.0f))<0.00001f);
    assert(std::fabs(originalVU0DotXYZ_4748ac(
        ray.originalNormalizedRay,ray.originalNormalizedRay)-1.0f)<0.00001f);
    assert(argc==1||argc==2);
    unsigned checked=0;
    if(argc==2) {
        std::ifstream f(argv[1],std::ios::binary);assert(f);
        const std::vector<char> data((std::istreambuf_iterator<char>(f)),{});
        assert(data.size()==3328);
        const auto u16=[&](std::size_t p)->unsigned{
            return (unsigned char)data.at(p)|
                ((unsigned char)data.at(p+1)<<8);
        };
        const auto u32=[&](std::size_t p)->unsigned {
            return u16(p)|(u16(p+2)<<16);
        };
        for(unsigned file=0;file<2;++file) {
            const unsigned base=file?2224:96;
            const unsigned count=u32(base+0x24),header=u32(base+0x3c);
            const unsigned vertices=u32(base+0x40);
            assert(count==(file?20u:39u));
            for(unsigned i=0;i<count;++i) {
                const unsigned at=base+header+30*i,number=u16(at+0x16);
                assert(number>=1&&number<=3);
                if(number<2)continue;
                OriginalVU0Vec4 source{},dest{};
                const std::size_t first=base+vertices+16*u16(at+0x18);
                const std::size_t next=base+vertices+16*u16(at+0x1a);
                assert(first+16<=base+(file?1104:2128));
                assert(next+16<=base+(file?1104:2128));
                std::memcpy(&source,data.data()+first,16);
                std::memcpy(&dest,data.data()+next,16);
                const auto packed=originalActorPackedSubsequentRayStart_57c88c(source);
                assert(packed.x==0.0f&&packed.y==0.0f &&
                       std::bit_cast<std::uint32_t>(packed.z)==
                       std::bit_cast<std::uint32_t>(source.z) &&
                       std::bit_cast<std::uint32_t>(packed.w)==
                       std::bit_cast<std::uint32_t>(source.w));
                const auto real=originalActorProjectionRay_4c1b3c(source,dest);
                assert(std::isfinite(real.originalLengthF12));
                assert(real.originalLengthF12>=0);
                if(real.originalLengthF12>0) {
                    const auto squared=originalVU0DotXYZ_4748ac(
                        real.originalNormalizedRay,real.originalNormalizedRay);
                    assert(std::fabs(squared-1.0f)<0.002f);
                }
                ++checked;
            }
        }
        assert(checked==59);
    }
    std::cout<<"Original VU0 XYZ dot and actor projection ray, real polygons: "<<checked<<"\n";
}
