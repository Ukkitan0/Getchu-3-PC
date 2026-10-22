#include "ae3/original_render_draw_packet.hpp"
#include <stdexcept>
#include <string>
#include <iostream>
using namespace ae3;
static void require(bool b,const char* s){if(!b)throw std::runtime_error(s);}
int main(){
    try{
        OriginalRenderContextSlots dest{};
        OriginalRenderResourceOwner owner{};
        std::string calls;
        OriginalDrawPacketDependencies deps;
        deps.initialize51c9b0=[&](OriginalDrawScratch& scratch){
            calls+='I';scratch.data[0x330]=0xAB;
            require(scratch.data.size()==0x390,"retail scratch size mismatch");
        };
        deps.convertModel27c614=[&](std::uintptr_t root,OriginalDrawScratch& scratch){
            calls+='S';require(root==0x12345,"static model root wrong");scratch.data[0x331]=0xCD;
        };
        deps.convertModel27c520=[&](std::uintptr_t root,OriginalDrawScratch& scratch){
            calls+='A';require(root==0x45678,"actor model root wrong");
            require(scratch.data[0]==0x78&&scratch.data[1]==0x56&&
                    scratch.data[2]==0x34&&scratch.data[3]==0x12,
                    "retail actor source +0x118 not copied before converter");
            scratch.data[0x332]=0xEF;
        };
        deps.createPacket3af8f0=[&](std::uintptr_t gfx,std::span<const std::uint8_t> desc){
            calls+='P';require(gfx==0x9876,"retail global graphics pointer missing");
            require(desc.size()==0x60&&desc[0]==0xAB,
                    "retail +0x330 packet descriptor absent");
            return static_cast<std::uintptr_t>(desc[1]?0xAA55:0x55AA);
        };
        auto handle=originalBuildStaticDrawPacket280364(dest,0x12345,0x9876,deps);
        require(calls=="ISP"&&handle==0xAA55&&dest.source82c==handle,
                "original 0x280364 failed source call chain/context store");
        calls.clear();
        handle=originalBuildActorDrawPacket2803b4(dest,owner,0x45678,0x12345678,0x9876,deps);
        require(calls=="IAP"&&handle==0x55AA&&dest.source82c==handle,
                "normal actor branch must bypass original special VU block");
        auto& res=originalEnsureRenderActorResource(owner);
        originalSetResourceFlag00(res,1);
        deps.fillSpecialModelDescriptor=[&](OriginalRenderResourceState& resource,OriginalDrawScratch& scratch){
            calls+='V';require(resource.flag00==1,"wrong original effect resource");
            scratch.data[0x331]=0x42;
        };
        calls.clear();
        handle=originalBuildActorDrawPacket2803b4(dest,owner,0x45678,0x12345678,0x9876,deps);
        require(calls=="IAVP"&&handle==0xAA55&&dest.source82c==handle,
                "original actor special/VU path not handed to authentic callback");
        calls.clear();deps.fillSpecialModelDescriptor={};
        bool threw=false;
        try{originalBuildActorDrawPacket2803b4(dest,owner,0x45678,0,0x9876,deps);}
        catch(const std::runtime_error&){threw=true;}
        require(threw&&calls=="IA","missing original actor VU instructions were replaced");
        deps.createPacket3af8f0={};threw=false;
        try{originalBuildStaticDrawPacket280364(dest,0x12345,0x9876,deps);}
        catch(const std::runtime_error&){threw=true;}
        require(threw,"unrecovered packet renderer was silently skipped");
        std::cout<<"ORIGINAL_RETAIL_DRAW_PACKET_PASS: genuine_280364_280328_chain actor_flag_branch scratch_330 context_82c dependency_guards\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
