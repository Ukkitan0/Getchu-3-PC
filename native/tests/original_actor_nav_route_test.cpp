#include "ae3/original_actor_nav_route.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static void w16(std::vector<std::byte>& d,std::size_t p,std::uint16_t w){
    d.at(p)=std::byte(w&255);d.at(p+1)=std::byte(w>>8);
}
static void w32(std::vector<std::byte>& d,std::size_t p,std::uint32_t w){
    for(unsigned i=0;i<4;++i)d.at(p+i)=std::byte((w>>(i*8))&255);
}
static std::uint16_t u16(std::span<const std::byte> d,std::size_t p){
    ck(p+2<=d.size(),"source graph truncated");
    return std::uint16_t(std::to_integer<unsigned>(d[p])|
        (std::to_integer<unsigned>(d[p+1])<<8));
}
static std::uint32_t u32(std::span<const std::byte> d,std::size_t p){
    return std::uint32_t(u16(d,p))|(std::uint32_t(u16(d,p+2))<<16);
}
struct Lifetime:OriginalActorRouteLifetimeCalls_575ec0{
    std::vector<std::string> observed;
    void* expected=nullptr;
    void call_original_220a78(void* p) override {ck(p==expected,"inner route identity");observed.push_back("inner");}
    void call_original_41a3b0(void* p) override {ck(p==expected,"outer route identity");observed.push_back("outer");}
};
int main(int argc,char**argv){
    ck(argc==1||argc==2,"optional authentic woods_c/navigation.pck");
    std::vector<std::byte> data(0x80+5*0x1e);
    w32(data,0x24,5);w32(data,0x3c,0x80);
    w16(data,0x80+2,3);
    for(unsigned slot=0;slot<3;++slot){
        w16(data,0x80+4+slot*2,slot+1);
        w16(data,0x80+0x0a+slot*2,slot);
    }
    for(unsigned dest=1;dest<=3;++dest){
        const std::array<std::uint32_t,2> route{0,dest};
        auto value=originalActorRouteEdgeDecision_57bd88(route,0,data);
        ck(value.sourceEdgeSlot==dest-1,"exact source edge slot");
        ck(value.originalActionKind==dest-1,"original edge-kind slot");
        ck(value.originalKindTwoBranch==(dest==3),"actor kind-two branch");
    }
    const std::array<std::uint32_t,2> missing{0,4};
    ck(originalActorRouteEdgeSlot_221104(missing,0,data)==0,"original no-match result is zero");
    ck(originalActorRouteEdgeDecision_57bd88(missing,0,data).originalActionKind==0,
       "source proceeds with edge kind zero on no match");
    bool invalid=false;try{(void)originalActorRouteEdgeSlot_221104(missing,1,data);}
    catch(const std::out_of_range&){invalid=true;}ck(invalid,"host segment bounds");
    invalid=false;try{(void)originalActorRouteEdgeKind_21b444(0,3,data);}
    catch(const std::out_of_range&){invalid=true;}ck(invalid,"original max three edge kinds");
    Lifetime lifetime;int a=1,b=2;void* stored=nullptr;
    originalActorReplaceRoute_575ec0(stored,&a,lifetime);
    ck(stored==&a&&lifetime.observed.empty(),"initial acquisition should not release");
    originalActorReplaceRoute_575ec0(stored,&a,lifetime);
    ck(lifetime.observed.empty(),"same pointer must be retained");
    lifetime.expected=&a;originalActorReplaceRoute_575ec0(stored,&b,lifetime);
    ck(stored==&b&&lifetime.observed==std::vector<std::string>({"inner","outer"}),
       "old route release order or replacement invalid");
    lifetime.expected=&b;originalActorReplaceRoute_575ec0(stored,nullptr,lifetime);
    ck(stored==nullptr&&lifetime.observed==std::vector<std::string>({"inner","outer","inner","outer"}),
       "route null reset must release old allocation");
    originalActorReplaceRoute_575ec0(stored,nullptr,lifetime);
    ck(lifetime.observed.size()==4,"null to null should not call allocator");
    if(argc==2){
        std::ifstream f(argv[1],std::ios::binary);ck(bool(f),"authentic private nav PCK absent");
        const std::vector<char> in((std::istreambuf_iterator<char>(f)),{});
        ck(in.size()==3328,"authentic woods_c PCK size changed");
        std::vector<std::byte> pck;for(unsigned char c:in)pck.push_back(std::byte(c));
        unsigned checked=0, kind2=0, checkedGraphs=0;
        for(unsigned t=0;t<2;++t){
            const auto raw=std::span<const std::byte>(pck).subspan(t?2224:96,t?1104:2128);
            const auto count=u32(raw,0x24),base=u32(raw,0x3c);
            ck(count==(t?20u:39u),"genuine woods_c polygon count");
            ck(base+count*0x1e<=raw.size(),"original descriptor table truncated");
            for(unsigned i=0;i<count;++i){
                const auto at=base+i*0x1e;
                const auto degree=u16(raw,at+2);ck(degree<=3,"retail adjacency limit");
                for(unsigned slot=0;slot<degree;++slot){
                    const auto neighbor=u16(raw,at+4+2*slot);
                    ck(neighbor<count,"real edge target range");
                    const std::array<std::uint32_t,2> route{i,neighbor};
                    const auto decided=originalActorRouteEdgeDecision_57bd88(route,0,raw);
                    // Duplicate destination slots retain first-match behavior.
                    unsigned first=0;
                    while(first<slot&&u16(raw,at+4+2*first)!=neighbor)++first;
                    ck(decided.sourceEdgeSlot==first,"original route-to-edge action slot mismatch");
                    ck(decided.originalActionKind==u16(raw,at+0x0a+2*first),
                       "original route edge action kind mismatch");
                    kind2+=(decided.originalKindTwoBranch?1u:0u);
                    ++checked;
                }
            }
            ++checkedGraphs;
        }
        ck(checkedGraphs==2&&checked>59,"genuine source graphs and edges not tested");
        ck(kind2>0,"real kind-two actor branches not exercised");
    }
}
