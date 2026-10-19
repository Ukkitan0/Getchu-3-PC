#include "ae3/archive.hpp"
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace fs=std::filesystem;
namespace {
void verify_check(bool value,const char* expression){
    if(!value)throw std::runtime_error(std::string("native assertion failed: ")+expression);
}
#define assert(expr) verify_check(bool(expr), #expr)
void w16(std::vector<std::uint8_t>& b,std::size_t at,std::uint16_t v){
    b.at(at)=std::uint8_t(v);b.at(at+1)=std::uint8_t(v>>8);
}
void w32(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t v){
    w16(b,at,std::uint16_t(v));w16(b,at+2,std::uint16_t(v>>16));
}
void word(std::vector<std::uint8_t>& b,std::size_t pos,const char* s){
    while(*s)b.at(pos++)=std::uint8_t(*s++);
}
void record(std::vector<std::uint8_t>& b,std::size_t at,
            const std::string& name,std::uint32_t sector,std::uint32_t length,
            bool folder=false){
    std::size_t size=33+name.size()+(name.size()%2==0);
    b.at(at)=std::uint8_t(size);
    w32(b,at+2,sector);w32(b,at+10,length);
    b[at+25]=folder?2:0;b[at+28]=1;b[at+30]=1;b[at+32]=std::uint8_t(name.size());
    for(std::size_t i=0;i<name.size();++i)b.at(at+33+i)=std::uint8_t(name[i]);
}
void save(const fs::path& path,const std::vector<std::uint8_t>& b){
    std::ofstream out(path,std::ios::binary|std::ios::trunc);
    out.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));
    if(!out)throw std::runtime_error("fixture write failed");
}
std::vector<std::uint8_t> vfi(){
    std::vector<std::uint8_t> b(4096,0);
    word(b,0,"VFI");w32(b,4,1);w32(b,8,1);
    w16(b,16,1);w16(b,18,0);
    w32(b,20,32);w32(b,24,56);w32(b,28,56);
    w16(b,32,24);w16(b,34,0);
    w32(b,36,1);w32(b,40,4);
    word(b,44,"test.tm2");
    b[2048]=0x54;b[2049]=0x49;b[2050]=0x4d;b[2051]=0x32;
    return b;
}
void verify(ae3::Archive& archive){
    assert(archive.entries().size()==1);
    assert(archive.entries()[0].path=="test.tm2");
    assert(archive.entries()[0].size==4);
    assert(archive.find("TEST.TM2").has_value());
    assert(!archive.find("not-present").has_value());
    assert(ae3::assetType("level.i3d")=="I3D");
    auto value=archive.read(archive.entries()[0]);
    assert((value==std::vector<std::uint8_t>{'T','I','M','2'}));
}
void reject(const fs::path& path) {
    bool failed=false;
    try{ae3::Archive x(path);}catch(const std::runtime_error&){failed=true;}
    if(!failed)throw std::runtime_error("invalid owned-game image passed validation");
}
}
int main(){
    fs::path base=fs::temp_directory_path() / "getchu3_native_iso_vfi_test";
    fs::create_directories(base);
    try{
        auto source=vfi();
        fs::path file=base/"DATA.BIN";
        save(file,source);
        ae3::Archive loose(file);
        verify(loose);
        assert(loose.sourceKind()=="native VFI DATA.BIN");

        auto iso=std::vector<std::uint8_t>(20*2048,0);
        auto vdb=16*2048;
        iso[vdb]=1;word(iso,vdb+1,"CD001");iso[vdb+6]=1;
        record(iso,vdb+156,std::string(1,'\0'),17,2048,true);
        auto dir=17*2048;
        record(iso,dir,"DATA.BIN;1",18,4096);
        std::copy(source.begin(),source.end(),iso.begin()+18*2048);
        auto isopath=base/"Ape Escape 3 USA.iso";
        save(isopath,iso);
        ae3::Archive mounted(isopath);
        verify(mounted);
        assert(mounted.sourceKind()=="ISO9660 direct archive mounting");

        auto unsafe=source;unsafe[44]='.';unsafe[45]='.';
        save(base/"invalid.bin",unsafe);reject(base/"invalid.bin");
        auto oob=source;w32(oob,40,9000);
        save(base/"outofbounds.bin",oob);reject(base/"outofbounds.bin");
        auto badMagic=source;badMagic[0]='X';
        save(base/"not-vfi.bin",badMagic);reject(base/"not-vfi.bin");
        fs::remove_all(base);
        std::cout<<"NATIVE_WINDOWS_ISO_VFI_TEST_PASSED"
                 <<" host_file_io=1 real_vfi_parser=1 no_ee_iop_gs=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_ARCHIVE_TEST_FAILED: "<<e.what()<<"\n";
        fs::remove_all(base);
        return 1;
    }
}
