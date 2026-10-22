#include "ae3/original_render_model_streams.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
void check(bool c,const char* m){if(!c)throw std::runtime_error(m);}
void store(std::span<std::uint8_t> b,std::size_t p,std::uint32_t n){
    for(unsigned i=0;i<4;++i)b[p+i]=std::uint8_t(n>>(i*8));
}
void run(){
    std::array<std::uint8_t,0x80> root{};
    constexpr std::uint8_t start[5]={4,0x14,0x24,0x34,0x44};
    constexpr std::uint32_t step[5]={96,96,112,80,128};
    for(unsigned i=0;i<5;++i){
        store(root,start[i],0xA000+0x1000*i);
        store(root,start[i]+4,0xA000+0x1000*i+step[i]*(i+1));
    }
    auto decoded=originalDecodeModelStreams27c614(root);
    for(unsigned i=0;i<5;++i){
        check(decoded.streams[i].sourceBeginField==start[i]&&
              decoded.streams[i].sourceEndField==start[i]+4&&
              decoded.streams[i].recordStride==step[i]&&
              decoded.streams[i].recordCount==i+1,
              "retail five separate model data source strata decoded wrong");
    }
    store(root,0x54,0x18000);store(root,0x58,0x18000+3*128);
    const auto more=originalDecodeActorSupplement27c520(root);
    check(more.recordCount==3&&more.recordStride==128&&more.sourceBeginField==0x54,
          "original actor supplemental model stream stride wrong");
    bool threw=false;
    try{originalDecodeModelStreams27c614(std::span(root).first(0x48));}
    catch(const std::invalid_argument&){threw=true;}
    check(threw,"original model source shorter than fifth end pointer accepted");
    store(root,0x38,0xA000+0x3000-1);threw=false;
    try{originalDecodeModelStreams27c614(root);}
    catch(const std::invalid_argument&){threw=true;}
    check(threw,"partial original 80-byte model record accepted");
    store(root,0x38,0xA000+0x3000);
    store(root,0x58,0x17fff);threw=false;
    try{originalDecodeActorSupplement27c520(root);}
    catch(const std::invalid_argument&){threw=true;}
    check(threw,"original actor resource end before begin accepted");
}
}
int main(){try{run();std::cout<<"ORIGINAL_RENDER_MODEL_STREAMS_PASS\n";return 0;}
catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
