#include "ae3/pck_archive.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool condition,const char* message){
    if(!condition)throw std::runtime_error(message);
}
void write32(std::vector<std::uint8_t>& data,std::size_t at,std::uint32_t v){
    if(at+4>data.size())throw std::runtime_error("fixture bounds");
    for(unsigned i=0;i<4;++i)data[at+i]=std::uint8_t(v>>(i*8));
}
std::vector<std::uint8_t> sample(){
    std::vector<std::uint8_t> bytes(512);
    const std::string pool="testmesh";
    bytes[0]='P';bytes[1]='C';bytes[2]='K';bytes[3]=0;
    bytes[16]='t';bytes[17]='e';bytes[18]='s';bytes[19]='t';
    bytes[20]='m';bytes[21]='e';bytes[22]='s';bytes[23]='h';
    bytes[24]=0;
    bytes[25]='a';bytes[26]='n';bytes[27]='i';bytes[28]='m';
    bytes[29]=0;
    bytes[30]='t';bytes[31]='e';bytes[32]='x';bytes[33]=0;
    bytes[34]='c';bytes[35]='o';bytes[36]='l';bytes[37]=0;
    write32(bytes,4,64);write32(bytes,8,4);
    // Entry name indexes may land in the middle or AT trailing NUL.
    write32(bytes,64,2);write32(bytes,68,980);
    write32(bytes,72,128);write32(bytes,76,16);
    write32(bytes,80,13);write32(bytes,84,428);
    write32(bytes,88,144);write32(bytes,92,16);
    write32(bytes,96,17);write32(bytes,100,1674);
    write32(bytes,104,160);write32(bytes,108,16);
    write32(bytes,112,18);write32(bytes,116,1508);
    write32(bytes,120,176);write32(bytes,124,16);
    for(unsigned i=0;i<4;++i){
        std::size_t at=128+i*16;
        if(i==0) {
            const char magic[]="I3D_BIN";
            for(unsigned j=0;j<7;++j)bytes[at+j]=std::uint8_t(magic[j]);
        }else if(i==1){
            const char magic[]="I3D_I3M";
            for(unsigned j=0;j<7;++j)bytes[at+j]=std::uint8_t(magic[j]);
        }else if(i==2){
            bytes[at]='T';bytes[at+1]='I';bytes[at+2]='M';bytes[at+3]='2';
        }else{
            const char magic[]="I3D_I3C";
            for(unsigned j=0;j<8;++j)bytes[at+j]=std::uint8_t(magic[j]);
        }
    }
    return bytes;
}
}
int main(){
    try{
        auto b=sample();
        ae3::PckArchive pck(b);
        check(pck.entries().size()==4,"PCK member count wrong");
        check(pck.entries()[0].name=="testmesh","PCK middle-of-name pointer incorrectly read");
        check(pck.entries()[1].name=="anim","PCK terminator-offset name incorrectly read");
        check(pck.entries()[2].name=="tex","PCK texture entry missing");
        check(pck.entries()[3].name=="col","PCK original collision member name missing");
        check(pck.entries()[0].kind==ae3::PckAssetKind::mesh,"real I3D_BIN magic not detected");
        check(pck.entries()[1].kind==ae3::PckAssetKind::animation,"I3D_I3M animation not detected");
        check(pck.entries()[2].kind==ae3::PckAssetKind::texture,"TIM2 texture not detected");
        check(pck.entries()[3].kind==ae3::PckAssetKind::collision,
              "real I3D_I3C PCK magic not detected");
        auto blob=pck.read(pck.entries()[0]);
        check(blob.size()==16 && blob[0]=='I',"native PCK extraction bytes mismatch");
        auto invalid=b;
        write32(invalid,72,900);
        bool rejected=false;
        try{ae3::PckArchive bad(invalid);}
        catch(const std::runtime_error&){rejected=true;}
        check(rejected,"out-of-range PCK payload accepted");
        invalid=b;write32(invalid,4,5000);
        rejected=false;
        try{ae3::PckArchive bad(invalid);}
        catch(const std::runtime_error&){rejected=true;}
        check(rejected,"broken PCK index table accepted");
        const auto root=std::filesystem::temp_directory_path()/"getchu3_native_pck_fixture";
        std::filesystem::remove_all(root);
        pck.extractTo(root);
        check(std::filesystem::exists(root/"0000_testmesh.i3d"),"I3D member was not exported");
        check(std::filesystem::exists(root/"0001_anim.i3m"),"I3M member was not exported");
        check(std::filesystem::exists(root/"0002_tex.tm2"),"TIM2 member was not exported");
        check(std::filesystem::exists(root/"0003_col.i3c"),"real original collision was not exported as I3C");
        rejected=false;
        try{pck.extractTo(root);}
        catch(const std::runtime_error&){rejected=true;}
        check(rejected,"native extraction silently overwrote original files");
        std::filesystem::remove_all(root);
        std::cout<<"NATIVE_PCK_PARSER_TEST_PASSED name_recovery=1 asset_types=4 output_security=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_PCK_PARSER_TEST_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
