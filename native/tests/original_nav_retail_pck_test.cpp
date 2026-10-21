#include "ae3/original_nav_pck_bridge.hpp"
#include <array>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok){if(!ok)throw std::runtime_error("Original retail woods_c nav regression");}
static std::uint32_t r32(const std::vector<std::uint8_t>& d,std::size_t p){
    ck(p+4<=d.size());
    return std::uint32_t(d[p])|(std::uint32_t(d[p+1])<<8)|
        (std::uint32_t(d[p+2])<<16)|(std::uint32_t(d[p+3])<<24);
}
static std::uint16_t r16(const std::vector<std::uint8_t>& d,std::size_t p){
    ck(p+2<=d.size());return std::uint16_t(d[p])|(std::uint16_t(d[p+1])<<8);
}
int main(int argc,char**argv){
    ck(argc==2);
    std::ifstream file(argv[1],std::ios::binary);
    ck(static_cast<bool>(file));
    const std::vector<char> content((std::istreambuf_iterator<char>(file)),{});
    std::vector<std::byte> bytes;
    for(unsigned char c:content)bytes.push_back(std::byte{c});
    auto sources=originalNavReadNativePck_4f0a5c(bytes);
    ck(sources.size()==2);
    ck(sources[0].originalFilename=="nav_monkey.nav"&&sources[0].originalSlot==0);
    ck(sources[1].originalFilename=="nav_enemy.nav"&&sources[1].originalSlot==1);
    unsigned total=0;
    for(unsigned i=0;i<2;++i){
        const auto& d=sources[i].originalNavBytes;
        ck(d.size()==(i?1104u:2128u));
        const auto n=r32(d,0x24),descs=r32(d,0x3c),vertices=r32(d,0x40);
        ck(n==(i?20u:39u));
        ck(descs==(i?144u:208u)&&vertices==(i?752u:1392u));
        for(unsigned j=0;j<n;++j){
            const std::size_t pos=descs+30u*j;
            ck(r16(d,pos+22)==3);
            for(unsigned k=0;k<3;++k)
                ck(vertices+16ull*r16(d,pos+24+2*k)+16<=d.size());
        }
        total+=n;
    }
    ck(total==59);
    bool rejected=false;
    try{(void)originalNavBuildNativePck_4f0a5c(sources,{});}
    catch(const std::invalid_argument&){rejected=true;}
    ck(rejected);
    const std::array<OriginalNavRuntimeSourceBinding_4f0a5c,2> absent{{
        {"nav_monkey.nav",0,0},{"nav_enemy.nav",0,0}}};
    rejected=false;
    try{(void)originalNavBuildNativePck_4f0a5c(sources,absent);}
    catch(const std::invalid_argument&){rejected=true;}
    ck(rejected);
    bytes.clear();bytes.shrink_to_fit();
    ck(sources[0].originalNavBytes.size()==2128);
}
