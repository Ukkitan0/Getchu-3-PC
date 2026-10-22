#include "ae3/original_retail_pad.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
namespace {
void check(bool v,const char* msg){if(!v)throw std::runtime_error(msg);}
void close(float actual,float expected,const char* msg){
    check(std::abs(actual-expected)<0.00001f,msg);
}
class RetailChannels : public OriginalRetailPadChannelSource {
public:
    std::array<int,32> state{};
    std::vector<std::uint32_t> calls;
    int queryOriginalPadChannel_4059e8(std::uint32_t channel) override {
        calls.push_back(channel);
        return state.at(channel);
    }
};
void sourceAxes(){
    auto p=originalRetailAnalog_517788(128,128);
    close(p.x,0,"neutral x");close(p.y,0,"neutral y");
    p=originalRetailAnalog_517788(160,128);
    close(p.x,0,"radial dead zone x");close(p.y,0,"radial dead zone y");
    p=originalRetailAnalog_517788(248,128);
    close(p.x,1,"positive extreme normalization");
    p=originalRetailAnalog_517788(8,128);
    close(p.x,-1,"negative extreme normalization");
    p=originalRetailAnalog_517788(203,128);
    // 75/120=.625; factor (.625-.375)/(.625*.625)=0.64.
    close(p.x,.4f,"retail radial normalization");
    p=originalRetailAnalog_517788(255,255);
    check(p.x<=1 && p.y<=1 && p.x>0 && p.y>0,
          "retail componentwise saturation");
}
void hostObservationToActualRetailChannels(){
    OriginalPadObservation physical;
    physical.connected=true;
    physical.pressedMask=originalPadBit(OriginalPadButton::cross) |
                         originalPadBit(OriginalPadButton::start);
    physical.sticks={248,128,8,128}; // LX LY RX RY
    OriginalHostPadChannels bridge(physical);
    OriginalRetailPadReader reader;
    const auto value=reader.sample(bridge,true);
    // Original button query table maps index 1->source 3 (Start),
    // index 7->source 14 (Cross).
    check(value.originalButtonBits==((1u<<1)|(1u<<7)),
          "physical PS2 button identities not mapped to retail query order");
    check(value.originalAxisBytes==std::array<std::uint8_t,4>{8,128,248,128},
          "physical LX/LY/RX/RY not mapped to source RX/RY/LX/LY");
    close(value.left.x,1,"physical left stick source channel failed");
    close(value.right.x,-1,"physical right stick source channel failed");
    check(bridge.queryOriginalPadChannel_4059e8(0x14)==-1,
          "fabricated unsupported original pad channel");
}
void originalChannelSequenceAndEdges(){
    constexpr std::array<std::uint32_t,16> ids{
        0,3,4,6,7,5,12,14,15,13,10,8,1,11,9,2};
    check(OriginalRetailButtonChannels_6ed0c8==ids,"retail mapping changed");
    RetailChannels source;
    source.state[0]=1;source.state[15]=1;
    source.state[0x12]=248;source.state[0x13]=128;
    source.state[0x10]=128;source.state[0x11]=8;
    OriginalRetailPadReader reader;
    auto first=reader.sample(source,true);
    check(source.calls.size()==20,"not exactly 20 retail PAD queries");
    for(std::size_t i=0;i<16;++i)
        check(source.calls[i]==ids[i],"retail button query order wrong");
    for(std::size_t i=0;i<4;++i)
        check(source.calls[16+i]==OriginalRetailAxisChannels_5179b0[i],
              "retail axis query order wrong");
    check(first.originalButtonBits==((1u<<0)|(1u<<8)),
          "button bits not ordered by original index");
    check(first.originalNewPressBits==first.originalButtonBits &&
          first.hostReleasedBits==0,"first-frame press edge lost");
    check(first.originalAxisBytes==std::array<std::uint8_t,4>{248,128,128,8},
          "raw axis byte mapping changed");
    close(first.right.x,1,"retail right stick X not decoded");
    close(first.left.y,-1,"retail left stick Y not decoded");
    const auto gameInput=originalRetailGamePadState_26eb34(first);
    check(gameInput.originalButtonBits==first.originalButtonBits &&
          gameInput.originalNewPressBits==first.originalNewPressBits &&
          sizeof(gameInput)==0x18,"original game controller record layout wrong");
    close(gameInput.rightX,1,"retail game RX missing");
    close(gameInput.leftY,-1,"retail game LY missing");
    source.calls.clear();
    const auto next=reader.sample(source,true);
    check(next.originalNewPressBits==0 && next.hostReleasedBits==0 &&
          source.calls.size()==20,"held button repeated press or polls skipped");
    source.calls.clear();
    const auto lost=reader.sample(source,false);
    check(source.calls.empty(),"original pad queried while disconnected");
    check(lost.originalButtonBits==0 && lost.originalNewPressBits==0 &&
          lost.hostReleasedBits==first.originalButtonBits,
          "device loss did not zero original state");
    check(lost.originalAxisBytes==std::array<std::uint8_t,4>{128,128,128,128},
          "original disconnected stick neutrality lost");
    const auto reconnected=reader.sample(source,true);
    check(reconnected.originalNewPressBits==first.originalButtonBits,
          "original press edge absent after reconnect");
    reader.reset();
    const auto reset=reader.sample(source,true);
    check(reset.originalNewPressBits==first.originalButtonBits,
          "retail reader reset retained old bits");
}
}
int main(){
    try{sourceAxes();originalChannelSequenceAndEdges();hostObservationToActualRetailChannels();
        std::cout<<"ORIGINAL_RETAIL_PAD_517788_5179B0_51785C_PASSED "
                    "analog=1 channel_sequence=20 edge=1 disconnect=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"ORIGINAL_RETAIL_PAD_FAILED: "<<e.what()<<'\n';return 1;
    }
}
